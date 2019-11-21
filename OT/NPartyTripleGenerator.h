#ifndef OT_NPARTYTRIPLEGENERATOR_H_
#define OT_NPARTYTRIPLEGENERATOR_H_

#include "Networking/Player.h"
#include "OT/BaseOT.h"
#include "Tools/random.h"
#include "Tools/time-func.h"
#include "Processor/InputTuple.h"

#include "OT/OTTripleSetup.h"
#include "OT/TripleMachine.h"
#include "OT/OTMultiplier.h"

#include <map>
#include <vector>

#define N_AMPLIFY 3

template <class T, class U, int N>
class ShareTriple_;
template <class T, int N>
class PlainTriple;

template <class T, int N>
using ShareTriple = ShareTriple_<T, T, N>;

class GeneratorThread
{
protected:
    pthread_mutex_t mutex;
    pthread_cond_t ready;

public:
    int nTriples;

    map<string,Timer> timers;

    bool multi_threaded;

    GeneratorThread() : nTriples(0), multi_threaded(true) {}
    virtual ~GeneratorThread() {};
    virtual void generate() = 0;

    void lock();
    void unlock();
    void signal();
    void wait();
};

template<class T>
class OTTripleGenerator : public GeneratorThread
{
    typedef typename T::open_type open_type;
    typedef typename T::mac_key_type mac_key_type;
    typedef typename T::sacri_type sacri_type;

protected:
    //OTTripleSetup* setup;
    Player& globalPlayer;
    Player* parentPlayer;

    int thread_num;
    int nbase;

    struct timeval last_lap;

    ofstream outputFile;

    SeededPRNG share_prg;

    mac_key_type mac_key;

    void start_progress();
    void print_progress(int k);

    void signal_multipliers(MultJob job);
    void wait_for_multipliers();

    typename T::Multiplier* new_multiplier(int i);

public:
    // TwoPartyPlayer's for OTs, n-party Player for sacrificing
    vector<TwoPartyPlayer*> players;
    vector<typename T::Multiplier*> ot_multipliers;
    //vector<OTMachine*> machines;
    BitVector baseReceiverInput; // same for every set of OTs
    vector< vector< vector<BitVector> > > baseSenderInputs;
    vector< vector<BitVector> > baseReceiverOutputs;
    vector<BitVector> valueBits;
    BitVector b_padded_bits;

    int my_num;
    int nTriplesPerLoop;
    int nloops;
    int field_size;
    int nAmplify;
    int nPreampTriplesPerLoop;
    int repeat[3];
    int nparties;

    MascotParams& machine;

    vector<PlainTriple<open_type, N_AMPLIFY>> preampTriples;
    vector<array<open_type, 3>> plainTriples;

    typename T::MAC_Check* MC;

    OTTripleGenerator(const OTTripleSetup& setup, const Names& names,
            int thread_num, int nTriples, int nloops, MascotParams& machine,
            mac_key_type mac_key,
            Player* parentPlayer = 0);
    ~OTTripleGenerator();

    void generate() { throw not_implemented(); }

    void generatePlainTriples();
    void plainTripleRound(int k = 0);

    void run_multipliers(MultJob job);

    mac_key_type get_mac_key() const { return mac_key; }

    size_t data_sent();
    NamedCommStats comm_stats();
};

template<class T>
class NPartyTripleGenerator : public OTTripleGenerator<T>
{
    typedef typename T::open_type open_type;
    typedef typename T::mac_key_type mac_key_type;
    typedef typename T::sacri_type sacri_type;

    virtual void generateTriples() = 0;
    virtual void generateBits() = 0;

public:
    vector< ShareTriple_<sacri_type, mac_key_type, 2> > uncheckedTriples;
    vector<InputTuple<Share<sacri_type>>> inputs;

    NPartyTripleGenerator(const OTTripleSetup& setup, const Names& names,
            int thread_num, int nTriples, int nloops, MascotParams& machine,
            mac_key_type mac_key,
            Player* parentPlayer = 0);
    virtual ~NPartyTripleGenerator() {}

    void generate();
    void generateInputs(int player);
};

template<class T>
class MascotTripleGenerator : public NPartyTripleGenerator<T>
{
    typedef typename T::open_type open_type;
    typedef typename T::mac_key_type mac_key_type;
    typedef typename T::sacri_type sacri_type;

    void generateTriples();
    void generateBits();
    void generateBitsGf2n();
    template<class U, class V, class W, int N>
    void generateBitsFromTriples(vector<ShareTriple_<U, V, N> >& triples,
            W& MC, ofstream& outputFile);

    void sacrifice(vector<ShareTriple_<open_type, mac_key_type, 2> >& uncheckedTriples,
            typename T::MAC_Check& MC, PRNG& G);

public:
    vector<T> bits;

    MascotTripleGenerator(const OTTripleSetup& setup, const Names& names,
            int thread_num, int nTriples, int nloops, MascotParams& machine,
            mac_key_type mac_key,
            Player* parentPlayer = 0);
};

template<class T>
class Spdz2kTripleGenerator : public NPartyTripleGenerator<T>
{
    typedef typename T::open_type open_type;
    typedef typename T::mac_key_type mac_key_type;
    typedef typename T::sacri_type sacri_type;

    void generateBits() { throw not_implemented(); }

    template<class U>
    void sacrificeZ2k(
            vector<
                    ShareTriple_<typename T::sacri_type,
                            typename T::mac_key_type, 2> >& uncheckedTriples,
            U& MC, PRNG& G);

public:
    Spdz2kTripleGenerator(const OTTripleSetup& setup, const Names& names,
            int thread_num, int nTriples, int nloops, MascotParams& machine,
            mac_key_type mac_key,
            Player* parentPlayer = 0);

    void generateTriples();
};

template<class T>
size_t OTTripleGenerator<T>::data_sent()
{
    size_t res = 0;
    if (parentPlayer != &globalPlayer)
        res = globalPlayer.sent;
    for (auto& player : players)
        res += player->sent;
    return res;
}

template<class T>
NamedCommStats OTTripleGenerator<T>::comm_stats()
{
    NamedCommStats res;
    if (parentPlayer != &globalPlayer)
        res = globalPlayer.comm_stats;
    for (auto& player : players)
        res += player->comm_stats;
    return res;
}

#endif
