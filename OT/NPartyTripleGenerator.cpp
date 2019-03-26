// (C) 2018 University of Bristol. See License.txt

#include "NPartyTripleGenerator.h"

#include "OT/OTExtensionWithMatrix.h"
#include "OT/OTMultiplier.h"
#include "Math/gfp.h"
#include "Math/Share.h"
#include "Math/operators.h"
#include "Auth/Subroutines.h"
#include "Auth/MAC_Check.h"

#include <sstream>
#include <fstream>
#include <math.h>

template <class T, int N>
class Triple
{
public:
    T a[N];
    T b;
    T c[N];

    int repeat(int l)
    {
        switch (l)
        {
        case 0:
        case 2:
            return N;
        case 1:
            return 1;
        default:
            throw bad_value();
        }
    }

    T& byIndex(int l, int j)
    {
        switch (l)
        {
        case 0:
            return a[j];
        case 1:
            return b;
        case 2:
            return c[j];
        default:
            throw bad_value();
        }
    }

    template <int M>
    void amplify(const Triple<T,M>& uncheckedTriple, PRNG& G)
    {
        b = uncheckedTriple.b;
        for (int i = 0; i < N; i++)
            for (int j = 0; j < M; j++)
            {
                typename T::value_type r;
                r.randomize(G);
                a[i] += r * uncheckedTriple.a[j];
                c[i] += r * uncheckedTriple.c[j];
            }
    }

    template <int M>
    void amplify(BitVector& a, T& b, Rectangle<Z2<M>, T>& c, PRNG& G)
    {
        assert(a.size() == M);
        this->b = b;
        for (int i = 0; i < N; i++)
        {
            this->a[i] = 0;
            this->c[i] = 0;
            for (int j = 0; j < M; j++)
            {
                T r;
                r.randomize(G);
                this->a[i] += r * a.get_bit(j);
                this->c[i] += r * c.rows[j];
            }
        }
    }

    void output(ostream& outputStream, int n = N, bool human = false)
    {
        for (int i = 0; i < n; i++)
        {
            a[i].output(outputStream, human);
            b.output(outputStream, human);
            c[i].output(outputStream, human);
        }
    }
};

template <class T, int N>
class PlainTriple : public Triple<T,N>
{
public:
    // this assumes that valueBits[1] is still set to the bits of b
    void to(vector<BitVector>& valueBits, int i)
    {
        for (int j = 0; j < N; j++)
        {
            valueBits[0].set_portion(i * N + j, this->a[j]);
            valueBits[2].set_portion(i * N + j, this->c[j]);
        }
    }
};

// T is Z2<K + 2S>, U is Z2<K + S>
template <class T, class U, int N>
class PlainTriple_ : public PlainTriple<T,N>
{
public:
    
    template <int M>
    void amplify(BitVector& a, U& b, Rectangle<Z2<M>, U>& c, PRNG& G)
    {
        assert(a.size() == M);
        this->b = b;
        for (int i = 0; i < N; i++)
        {
            U aa = 0, cc = 0;
            for (int j = 0; j < M; j++)
            {
                U r;
                r.randomize(G);
                if (a.get_bit(j))
                    aa += r;
                cc += U::Mul(r, c.rows[j]);
            }
            this->a[i] = aa;
            this->c[i] = cc;
        }
    }

    void to(vector<BitVector>& valueBits, int i)
    {
        for (int j = 0; j < N; j++)
        {
            valueBits[0].set_portion(i * N + j, this->a[j]);
            valueBits[2].set_portion(i * N + j, this->c[j]);
        }
    }
};

template <class T, class U, int N>
class ShareTriple_ : public Triple<Share<T>, N>
{
public:
    void from(PlainTriple<T,N>& triple, vector<OTMultiplierBase*>& ot_multipliers,
            int iTriple, const NPartyTripleGenerator& generator)
    {
        for (int l = 0; l < 3; l++)
        {
            int repeat = this->repeat(l);
            for (int j = 0; j < repeat; j++)
            {
                T value = triple.byIndex(l,j);
                T mac;
                mac.mul(value, generator.machine.get_mac_key<U>());
                for (int i = 0; i < generator.nparties-1; i++)
                    mac += ((OTMultiplierMac<T>*)ot_multipliers[i])->macs.at(l).at(iTriple * repeat + j);
                Share<T>& share = this->byIndex(l,j);
                share.set_share(value);
                share.set_mac(mac);
            }
        }
    }

    Share<T> get_check_value(PRNG& G)
    {
        Share<T> res;
        res += G.get<T>() * this->b;
        for (int i = 0; i < N; i++)
        {
            res += G.get<T>() * this->a[i];
            res += G.get<T>() * this->c[i];
        }
        return res;
    }

    template<class V>
    Triple<Share<V>, 1> reduce() {
        Triple<Share<V>, 1> triple;
        
        Share<V> _a;
        _a.set_share(V(this->a[0].get_share()));
        _a.set_mac(V(this->a[0].get_mac()));
        triple.a[0] = _a;
        
        Share<V> _b;
        _b.set_share(V(this->b.get_share()));
        _b.set_mac(V(this->b.get_mac()));
        triple.b = _b;

        Share<V> _c;
        _c.set_share(V(this->c[0].get_share()));
        _c.set_mac(V(this->c[0].get_mac()));
        triple.c[0] = _c;

        return triple;
    }

};

template <class T>
class TripleToSacrifice : public Triple<Share<T>, 1>
{
public:
    template <class U>
    void prepare_sacrifice(const ShareTriple_<T, U, 2>& uncheckedTriple, PRNG& G)
    {
        this->b = uncheckedTriple.b;
        U t;
        t.randomize(G);
        this->a[0] = uncheckedTriple.a[0] * t - uncheckedTriple.a[1];
        this->c[0] = uncheckedTriple.c[0] * t - uncheckedTriple.c[1];
    }

    Share<T> computeCheckShare(const T& maskedA)
    {
        return this->c[0] - maskedA * this->b;
    }
};

/*
 * Copies the relevant base OTs from setup
 * N.B. setup must not be stored as it will be used by other threads
 */
NPartyTripleGenerator::NPartyTripleGenerator(OTTripleSetup& setup,
        const Names& names, int thread_num, int _nTriples, int nloops,
        TripleMachine& machine) :
        globalPlayer(names, - thread_num * machine.nplayers * machine.nplayers),
        thread_num(thread_num),
        my_num(setup.get_my_num()),
        nloops(nloops),
        nparties(setup.get_nparties()),
        machine(machine)
{
    nTriplesPerLoop = DIV_CEIL(_nTriples, nloops);
    nTriples = nTriplesPerLoop * nloops;
    field_size = 128;
    nAmplify = machine.amplify ? N_AMPLIFY : 1;
    nPreampTriplesPerLoop = nTriplesPerLoop * nAmplify;

    int n = nparties;
    //baseReceiverInput = machines[0]->baseReceiverInput;
    //baseSenderInputs.resize(n-1);
    //baseReceiverOutputs.resize(n-1);
    nbase = setup.get_nbase();
    baseReceiverInput.resize(nbase);
    baseReceiverOutputs.resize(n - 1);
    baseSenderInputs.resize(n - 1);
    players.resize(n-1);

    gf2n_long::init_field(128);

    for (int i = 0; i < n-1; i++)
    {
        // i for indexing, other_player is actual number
        int other_player, id;
        if (i >= my_num)
            other_player = i + 1;
        else
            other_player = i;

        // copy base OT inputs + outputs
        for (int j = 0; j < 128; j++)
        {
            baseReceiverInput.set_bit(j, (unsigned int)setup.get_base_receiver_input(j));
        }
        baseReceiverOutputs[i] = setup.baseOTs[i]->receiver_outputs;
        baseSenderInputs[i] = setup.baseOTs[i]->sender_inputs;

        // new TwoPartyPlayer with unique id for each thread + pair of players
        if (my_num < other_player)
            id = (thread_num+1)*n*n + my_num*n + other_player;
        else
            id = (thread_num+1)*n*n + other_player*n + my_num;
        players[i] = new TwoPartyPlayer(names, other_player, id);
        cout << "Set up with player " << other_player << " in thread " << thread_num << " with id " << id << endl;
    }

    pthread_mutex_init(&mutex, 0);
    pthread_cond_init(&ready, 0);

    share_prg.ReSeed();
}

NPartyTripleGenerator::~NPartyTripleGenerator()
{
    for (size_t i = 0; i < players.size(); i++)
        delete players[i];
    //delete nplayer;
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&ready);
}

void* run_ot_thread(void* ptr)
{
    ((OTMultiplierBase*)ptr)->multiply();
    return NULL;
}

template<class T>
OTMultiplierBase* NPartyTripleGenerator::new_multiplier(int i)
{
    return new MascotMultiplier<T>(*this, i);
}

template<>
OTMultiplierBase* NPartyTripleGenerator::new_multiplier<Z2<160> >(int i)
{
    return new Spdz2kMultiplier<64, 96>(*this, i);
}

template<>
OTMultiplierBase* NPartyTripleGenerator::new_multiplier<Z2<128> >(int i)
{
    return new Spdz2kMultiplier<64, 64>(*this, i);
}

template<>
OTMultiplierBase* NPartyTripleGenerator::new_multiplier<Z2<64> >(int i)
{
    return new Spdz2kMultiplier<32, 32>(*this, i);
}

template<class T>
void NPartyTripleGenerator::generate()
{
    ot_multipliers.resize(nparties-1);

    timers["Generator thread"].start();

    for (int i = 0; i < nparties-1; i++)
    {
        ot_multipliers[i] = new_multiplier<T>(i);
        pthread_mutex_lock(&ot_multipliers[i]->mutex);
        pthread_create(&(ot_multipliers[i]->thread), 0, run_ot_thread, ot_multipliers[i]);
    }

    // add up the shares from each thread and write to file
    stringstream ss;
    ss << machine.prep_data_dir;
    if (machine.generateBits)
        ss << "Bits-";
    else
        ss << "Triples-";
    ss << T::type_char() << "-P" << my_num;
    if (thread_num != 0)
        ss << "-" << thread_num;
    if (machine.output)
        outputFile.open(ss.str().c_str());

    if (machine.generateBits)
    	generateBits<T>(ot_multipliers, outputFile);
    else
    	generateTriples<T>(ot_multipliers, outputFile);

    timers["Generator thread"].stop();
    if (machine.output)
        cout << "Written " << nTriples << " outputs to " << ss.str() << endl;
    else
        cout << "Generated " << nTriples << " outputs" << endl;

    // wait for threads to finish
    for (int i = 0; i < nparties-1; i++)
    {
        pthread_mutex_unlock(&ot_multipliers[i]->mutex);
        pthread_join(ot_multipliers[i]->thread, NULL);
        cout << "OT thread " << i << " finished\n" << flush;
    }
    cout << "OT threads finished\n";

    for (size_t i = 0; i < ot_multipliers.size(); i++)
        delete ot_multipliers[i];
}

template<>
void NPartyTripleGenerator::generateBits<gf2n>(vector< OTMultiplierBase* >& ot_multipliers,
		ofstream& outputFile)
{
    PRNG share_prg;
    share_prg.ReSeed();

    int nBitsToCheck = nTriplesPerLoop + field_size;
    valueBits.resize(1);
    valueBits[0].resize(ceil(1.0 * nBitsToCheck / field_size) * field_size);
    MAC_Check<gf2n> MC(machine.get_mac_key<gf2n>());
    vector< Share<gf2n> > bits(nBitsToCheck);
    vector< Share<gf2n> > to_open(1);
    vector<gf2n> opened(1);

    start_progress();

    for (int k = 0; k < nloops; k++)
    {
        print_progress(k);

    	valueBits[0].randomize_blocks<gf2n>(share_prg);

        signal_multipliers();
        timers["Authentication OTs"].start();
        wait_for_multipliers();
        timers["Authentication OTs"].stop();

        octet seed[SEED_SIZE];
        Create_Random_Seed(seed, globalPlayer, SEED_SIZE);
        PRNG G;
        G.SetSeed(seed);

        Share<gf2n> check_sum;
        gf2n r;
        for (int j = 0; j < nBitsToCheck; j++)
        {
            gf2n mac_sum = bool(valueBits[0].get_bit(j)) * machine.get_mac_key<gf2n>();
            for (int i = 0; i < nparties-1; i++)
                mac_sum += ((MascotMultiplier<gf2n>*)ot_multipliers[i])->macs[0][j];
            bits[j].set_share(valueBits[0].get_bit(j));
            bits[j].set_mac(mac_sum);
            r.randomize(G);
            check_sum += r * bits[j];
        }

        to_open[0] = check_sum;
        MC.POpen_Begin(opened, to_open, globalPlayer);
        MC.POpen_End(opened, to_open, globalPlayer);
        MC.Check(globalPlayer);

        if (machine.output)
            for (int j = 0; j < nTriplesPerLoop; j++)
                bits[j].output(outputFile, false);

        signal_multipliers();
   }
}

template<>
void NPartyTripleGenerator::generateBits<gfp>(vector< OTMultiplierBase* >& ot_multipliers,
		ofstream& outputFile)
{
	generateTriples<gfp>(ot_multipliers, outputFile);
}

template <class T>
void NPartyTripleGenerator::generateBits(vector< OTMultiplierBase* >& ot_multipliers,
        ofstream& outputFile)
{
    (void)ot_multipliers;
    (void)outputFile;
    throw not_implemented();
}

template<int K, int S>
void NPartyTripleGenerator::generateTriplesZ2k(vector< OTMultiplierBase* >& ot_multipliers,
        ofstream& outputFile)
{
	(void) outputFile;
	const int TAU = Spdz2kMultiplier<K, S>::TAU;
	valueBits.resize(3);
	for (int i = 0; i < 2; i++)
		valueBits[2*i].resize(TAU * nTriplesPerLoop);
	valueBits[1].resize((K + S) * (nTriplesPerLoop + 1));
	b_padded_bits.resize((K + 2 * S) * (nTriplesPerLoop + 1));
	vector< PlainTriple_<Z2<K + 2 * S>, Z2<K + S>, 2> > amplifiedTriples(nTriplesPerLoop);
	vector< ShareTriple_<Z2<K + 2 * S>, Z2<S>, 2> > uncheckedTriples(nTriplesPerLoop);
	MAC_Check_Z2k<Z2<K + 2 * S>, Z2<S>, Z2<K + S> > MC(machine.get_mac_key<Z2<S> >());

	start_progress();

	for (int k = 0; k < nloops; k++)
	{
		print_progress(k);

		for (int j = 0; j < 2; j++)
			valueBits[j].randomize_blocks<gf2n>(share_prg);

		for (int j = 0; j < nTriplesPerLoop + 1; j++)
		{
			Z2<K + S> b(valueBits[1].get_ptr_to_bit(j, K + S));
			b_padded_bits.set_portion(j, Z2<K + 2 * S>(b));
		}

		timers["OTs"].start();
		wait_for_multipliers();
		timers["OTs"].stop();

		octet seed[SEED_SIZE];
		Create_Random_Seed(seed, globalPlayer, SEED_SIZE);
		PRNG G;
		G.SetSeed(seed);

		for (int j = 0; j < nTriplesPerLoop; j++)
		{
			BitVector a(valueBits[0].get_ptr_to_bit(j, TAU), TAU);
			Z2<K + S> b(valueBits[1].get_ptr_to_bit(j, K + S));
			Z2kRectangle<TAU, K + S> c;
			c.mul(a, b);
			timers["Triple computation"].start();
			for (int i = 0; i < nparties-1; i++)
			{
				c += ((Spdz2kMultiplier<K, S>*)ot_multipliers[i])->c_output[j];
			}
			timers["Triple computation"].stop();
			amplifiedTriples[j].amplify(a, b, c, G);
			amplifiedTriples[j].to(valueBits, j);
		}

		signal_multipliers();
		wait_for_multipliers();

		for (int j = 0; j < nTriplesPerLoop; j++)
		{
			uncheckedTriples[j].from(amplifiedTriples[j], ot_multipliers, j, *this);
		}

		// we can skip the consistency check since we're doing a mac-check next
		// get piggy-backed random value
		Z2<K + 2 * S> r_share = b_padded_bits.get_ptr_to_bit(nTriplesPerLoop, K + 2 * S);
		Z2<K + 2 * S> r_mac;
		r_mac.mul(r_share, this->machine.template get_mac_key<Z2<S>>());
		for (int i = 0; i < this->nparties-1; i++)
			r_mac += ((OTMultiplierMac<Z2<K + 2 * S>>*)ot_multipliers[i])->macs.at(1).at(nTriplesPerLoop);
		Share<Z2<K + 2 * S>> r;
		r.set_share(r_share);
		r.set_mac(r_mac);

		MC.set_random_element(r);
		sacrifice<Z2<K + 2 * S>, Z2<S>, Z2<K + S>>(uncheckedTriples, MC, G);

		signal_multipliers();
   }
}

template<>
void NPartyTripleGenerator::generateTriples<Z2<64> >(vector< OTMultiplierBase* >& ot_multipliers,
        ofstream& outputFile)
{
    this->template generateTriplesZ2k<32, 32>(ot_multipliers, outputFile);
}

template<>
void NPartyTripleGenerator::generateTriples<Z2<128> >(vector< OTMultiplierBase* >& ot_multipliers,
        ofstream& outputFile)
{
    this->template generateTriplesZ2k<64, 64>(ot_multipliers, outputFile);
}


template<>
void NPartyTripleGenerator::generateTriples<Z2<160> >(vector< OTMultiplierBase* >& ot_multipliers,
        ofstream& outputFile)
{
    this->template generateTriplesZ2k<64, 96>(ot_multipliers, outputFile);
}

template<class T>
void NPartyTripleGenerator::generateTriples(vector< OTMultiplierBase* >& ot_multipliers,
    ofstream& outputFile)
{
    valueBits.resize(3);
    for (int i = 0; i < 2; i++)
        valueBits[2*i].resize(field_size * nPreampTriplesPerLoop);
    valueBits[1].resize(field_size * nTriplesPerLoop);
    vector< PlainTriple<T,N_AMPLIFY> > preampTriples;
    vector< PlainTriple<T,2> > amplifiedTriples;
    vector< ShareTriple<T,2> > uncheckedTriples;
    MAC_Check<T> MC(machine.get_mac_key<T>());

    if (machine.amplify)
        preampTriples.resize(nTriplesPerLoop);
    if (machine.generateMACs)
      {
	amplifiedTriples.resize(nTriplesPerLoop);
	uncheckedTriples.resize(nTriplesPerLoop);
      }

    start_progress();

    for (int k = 0; k < nloops; k++)
    {
        print_progress(k);

        for (int j = 0; j < 2; j++)
            valueBits[j].randomize_blocks<T>(share_prg);

        timers["OTs"].start();
        wait_for_multipliers();
        timers["OTs"].stop();

        for (int j = 0; j < nPreampTriplesPerLoop; j++)
        {
            T a((char*)valueBits[0].get_ptr() + j * T::size());
            T b((char*)valueBits[1].get_ptr() + j / nAmplify * T::size());
            T c = a * b;
            timers["Triple computation"].start();
            for (int i = 0; i < nparties-1; i++)
            {
                c += ((MascotMultiplier<T>*)ot_multipliers[i])->c_output[j];
            }
            timers["Triple computation"].stop();
            if (machine.amplify)
            {
                preampTriples[j/nAmplify].a[j%nAmplify] = a;
                preampTriples[j/nAmplify].b = b;
                preampTriples[j/nAmplify].c[j%nAmplify] = c;
            }
            else if (machine.output)
            {
                timers["Writing"].start();
                a.output(outputFile, false);
                b.output(outputFile, false);
                c.output(outputFile, false);
                timers["Writing"].stop();
            }
        }

        if (machine.amplify)
        {
            octet seed[SEED_SIZE];
            Create_Random_Seed(seed, globalPlayer, SEED_SIZE);
            PRNG G;
            G.SetSeed(seed);
            for (int iTriple = 0; iTriple < nTriplesPerLoop; iTriple++)
            {
                PlainTriple<T,2> triple;
                triple.amplify(preampTriples[iTriple], G);

                if (machine.generateMACs)
                    amplifiedTriples[iTriple] = triple;
                else if (machine.output)
                {
                    timers["Writing"].start();
                    triple.output(outputFile);
                    timers["Writing"].stop();
                }
            }

            if (machine.generateMACs)
            {
                for (int iTriple = 0; iTriple < nTriplesPerLoop; iTriple++)
                    amplifiedTriples[iTriple].to(valueBits, iTriple);

                signal_multipliers();
                timers["Authentication OTs"].start();
                wait_for_multipliers();
                timers["Authentication OTs"].stop();

                for (int iTriple = 0; iTriple < nTriplesPerLoop; iTriple++)
                {
                    uncheckedTriples[iTriple].from(amplifiedTriples[iTriple], ot_multipliers, iTriple, *this);

                    if (!machine.check and machine.output)
                    {
                        timers["Writing"].start();
                        amplifiedTriples[iTriple].output(outputFile);
                        timers["Writing"].stop();
                    }
                }

                if (machine.check)
                {
                    sacrifice(uncheckedTriples, MC, G);
                }
            }
        }

        signal_multipliers();
    }
}

template<class T, class U>
void NPartyTripleGenerator::sacrifice(
		vector<ShareTriple_<T, U, 2> > uncheckedTriples, MAC_Check<T>& MC, PRNG& G)
{
    vector< Share<T> > maskedAs(nTriplesPerLoop);
    vector<TripleToSacrifice<T> > maskedTriples(nTriplesPerLoop);
    for (int j = 0; j < nTriplesPerLoop; j++)
    {
        maskedTriples[j].prepare_sacrifice(uncheckedTriples[j], G);
        maskedAs[j] = maskedTriples[j].a[0];
    }

    vector<T> openedAs(nTriplesPerLoop);
    MC.POpen_Begin(openedAs, maskedAs, globalPlayer);
    MC.POpen_End(openedAs, maskedAs, globalPlayer);

    for (int j = 0; j < nTriplesPerLoop; j++) {
        MC.AddToCheck(maskedTriples[j].computeCheckShare(openedAs[j]), 0,
                globalPlayer);
    }

    MC.Check(globalPlayer);

    if (machine.generateBits)
        generateBitsFromTriples(uncheckedTriples, MC, outputFile);
    else
        if (machine.output)
            for (int j = 0; j < nTriplesPerLoop; j++)
                uncheckedTriples[j].output(outputFile, 1);
}

template<class T, class U, class V>
void NPartyTripleGenerator::sacrifice(
        vector<ShareTriple_<T, U, 2> > uncheckedTriples, MAC_Check_Z2k<T, U, V>& MC, PRNG& G)
{
    vector< Share<T> > maskedAs(nTriplesPerLoop);
    vector<TripleToSacrifice<T> > maskedTriples(nTriplesPerLoop);
    for (int j = 0; j < nTriplesPerLoop; j++)
    {
        // compute [p] = t * [a] - [ahat]
        // and first part of [sigma], i.e., t * [c] - [chat] 
        maskedTriples[j].prepare_sacrifice(uncheckedTriples[j], G);
        maskedAs[j] = maskedTriples[j].a[0];
    }

    vector<T> openedAs(nTriplesPerLoop);
    MC.POpen_Begin(openedAs, maskedAs, globalPlayer);
    MC.POpen_End(openedAs, maskedAs, globalPlayer);

    vector<Share<T>> sigmas;
    for (int j = 0; j < nTriplesPerLoop; j++) {
        // compute t * [c] - [chat] - [b] * p
        sigmas.push_back(maskedTriples[j].computeCheckShare(V(openedAs[j])));
    }
    vector<T> open_sigmas;
    
    MC.POpen_Begin(open_sigmas, sigmas, globalPlayer);
    MC.POpen_End(open_sigmas, sigmas, globalPlayer);
    MC.Check(globalPlayer);

    for (int j = 0; j < nTriplesPerLoop; j++) {
        if (V(open_sigmas[j]) != 0)
            throw mac_fail();
    }
    
    if (machine.generateBits)
        generateBitsFromTriples(uncheckedTriples, MC, outputFile);
    else
        if (machine.output)
            for (int j = 0; j < nTriplesPerLoop; j++)
                uncheckedTriples[j].template reduce<V>().output(outputFile, 1);
}

template<>
void NPartyTripleGenerator::generateBitsFromTriples(
        vector< ShareTriple<gfp,2> >& triples, MAC_Check<gfp>& MC, ofstream& outputFile)
{
    vector< Share<gfp> > a_plus_b(nTriplesPerLoop), a_squared(nTriplesPerLoop);
    for (int i = 0; i < nTriplesPerLoop; i++)
        a_plus_b[i] = triples[i].a[0] + triples[i].b;
    vector<gfp> opened(nTriplesPerLoop);
    MC.POpen_Begin(opened, a_plus_b, globalPlayer);
    MC.POpen_End(opened, a_plus_b, globalPlayer);
    for (int i = 0; i < nTriplesPerLoop; i++)
        a_squared[i] = triples[i].a[0] * opened[i] - triples[i].c[0];
    MC.POpen_Begin(opened, a_squared, globalPlayer);
    MC.POpen_End(opened, a_squared, globalPlayer);
    Share<gfp> one(gfp(1), globalPlayer.my_num(), MC.get_alphai());
    for (int i = 0; i < nTriplesPerLoop; i++)
    {
        gfp root = opened[i].sqrRoot();
        if (root.is_zero())
            continue;
        Share<gfp> bit = (triples[i].a[0] / root + one) / gfp(2);
        if (machine.output)
            bit.output(outputFile, false);
    }
}

template<class T, class U>
void NPartyTripleGenerator::generateBitsFromTriples(
        vector< ShareTriple_<T, U, 2> >& triples, MAC_Check<T>& MC, ofstream& outputFile)
{
    throw how_would_that_work();
    // warning gymnastics
    triples[0];
    MC.number();
    outputFile << "";
}

void NPartyTripleGenerator::start_progress()
{
    wait_for_multipliers();
    lock();
    signal();
    wait();
    gettimeofday(&last_lap, 0);
    signal_multipliers();
}

void NPartyTripleGenerator::print_progress(int k)
{
    if (thread_num == 0 && my_num == 0)
    {
        struct timeval stop;
        gettimeofday(&stop, 0);
        if (timeval_diff_in_seconds(&last_lap, &stop) > 1)
        {
            double diff = timeval_diff_in_seconds(&machine.start, &stop);
            double throughput = k * nTriplesPerLoop * machine.nthreads / diff;
            double remaining = diff * (nloops - k) / k;
            cout << k << '/' << nloops << ", throughput: " << throughput
                    << ", time left: " << remaining << ", elapsed: " << diff
                    << ", estimated total: " << (diff + remaining) << endl;
            last_lap = stop;
        }
    }
}

void NPartyTripleGenerator::lock()
{
    pthread_mutex_lock(&mutex);
}

void NPartyTripleGenerator::unlock()
{
    pthread_mutex_unlock(&mutex);
}

void NPartyTripleGenerator::signal()
{
    pthread_cond_signal(&ready);
}

void NPartyTripleGenerator::wait()
{
    pthread_cond_wait(&ready, &mutex);
}

void NPartyTripleGenerator::signal_multipliers()
{
    for (int i = 0; i < nparties-1; i++)
        pthread_cond_signal(&ot_multipliers[i]->ready);
}

void NPartyTripleGenerator::wait_for_multipliers()
{
    for (int i = 0; i < nparties-1; i++)
        pthread_cond_wait(&ot_multipliers[i]->ready, &ot_multipliers[i]->mutex);
}


template void NPartyTripleGenerator::generate<gf2n>();
template void NPartyTripleGenerator::generate<gfp>();

template void NPartyTripleGenerator::generate<Z2<64> >();
template void NPartyTripleGenerator::generate<Z2<128> >();
template void NPartyTripleGenerator::generate<Z2<160> >();
