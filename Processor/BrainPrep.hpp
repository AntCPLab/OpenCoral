/*
 * BrainPrep.cpp
 *
 */

#include "BrainPrep.h"
#include "Processor.h"
#include "Auth/MaliciousRepMC.h"

template<class T> class ZProtocol;

template<int L>
class Zint : public SignedZ2<L + 2>
{
    typedef SignedZ2<L + 2> super;

public:
    static string type_string()
    {
        return "Zint" + to_string(L);
    }

    Zint()
    {
    }

    template<class T>
    Zint(const T& other) : super(other)
    {
    }

    void randomize(PRNG& G)
    {
        *this = G.get<Z2<L>>();
    }
};

template<class U>
class ZShare : public Rep3Share<Zint<U::Z_BITS>>
{
public:
    typedef ZProtocol<U> Protocol;
    typedef ReplicatedMC<ZShare> MAC_Check;

    ZShare()
    {
    }

    template<class T>
    ZShare(const FixedVec<T, 2>& other)
    {
        FixedVec<T, 2>::operator=(other);
    }
};

template<class U>
class ZProtocol : public Replicated<Rep3Share<Zint<U::Z_BITS>>>
{
    typedef Rep3Share<Zint<U::Z_BITS>> T;
    vector<T> random;
    SeededPRNG G;

public:
    ZProtocol(Player& P) : Replicated<T>(P)
    {
    }

    T get_random()
    {
        if (random.empty())
        {
            int buffer_size = 10000;
            ReplicatedInput<Rep3Share<Zint<U::N_MASK_BITS>>> input(0, this->P);
            input.reset_all(this->P);
            for (int i = 0; i < buffer_size; i++)
            {
                typename U::clear tmp;
                tmp.randomize(G);
                input.add_mine(tmp);
            }
            input.exchange();
            for (int i = 0; i < buffer_size; i++)
            {
                random.push_back({});
                for (int j = 0; j < 3; j++)
                    random.back() += input.finalize(j);
            }
        }

        auto res = random.back();
        random.pop_back();
        return res;
    }
};

template<class T>
void BrainPrep<T>::buffer_triples()
{
    if(gfp2::get_ZpD().pr_bit_length
            <= ZProtocol<T>::share_type::clear::N_BITS)
        throw runtime_error(
                to_string(gfp2::get_ZpD().pr_bit_length)
                        + "-bit prime too short for "
                        + to_string(ZProtocol<T>::share_type::clear::N_BITS)
                        + "-bit integer computation");
    typedef Rep3Share<gfp2> pShare;
    auto buffer_size = this->buffer_size;
    Player& P = this->protocol->P;
    vector<array<ZShare<T>, 3>> triples;
    vector<array<Rep3Share<gfp2>, 3>> check_triples;
    DataPositions usage;
    HashMaliciousRepMC<pShare> MC;
    vector<Rep3Share<gfp2>> masked, checks;
    vector<gfp2> opened;
    ZProtocol<T> Z_protocol(P);
    Replicated<pShare> p_protocol(P);
    generate_triples(triples, buffer_size, &Z_protocol);
    generate_triples(check_triples, buffer_size, &p_protocol);
    auto t = Create_Random<gfp2>(P);
    vector<Rep3Share<gfp2>> converted_bs;
    for (int i = 0; i < buffer_size; i++)
    {
        pShare a = triples[i][0];
        converted_bs.push_back(triples[i][1]);
        auto& b = converted_bs[i];
        auto& f = check_triples[i][0];
        auto& g = check_triples[i][1];
        masked.push_back(a * t - f);
        masked.push_back(b - g);
    }
    MC.POpen(opened, masked, P);
    for (int i = 0; i < buffer_size; i++)
    {
        auto& b = converted_bs[i];
        pShare c = triples[i][2];
        auto& f = check_triples[i][0];
        auto& h = check_triples[i][2];
        auto& rho = opened[2 * i];
        auto& sigma = opened[2 * i + 1];
        checks.push_back(t * c - h - rho * b - sigma * f);
    }
    MC.POpen(opened, checks, P);
    for (auto& check : opened)
        if (check != 0)
            throw Offline_Check_Error("triple");
    MC.Check(P);
    for (auto& x : triples)
        this->triples.push_back({{x[0], x[1], x[2]}});
}
