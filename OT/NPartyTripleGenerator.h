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

class MascotGenerator
{
protected:
    pthread_mutex_t mutex;
    pthread_cond_t ready;

public:
    int nTriples;

    map<string,Timer> timers;

    bool multi_threaded;

    MascotGenerator() : nTriples(0), multi_threaded(true) {}
    virtual ~MascotGenerator() {};
    virtual void generate() = 0;

    void lock();
    void unlock();
    void signal();
    void wait();
};

template<class T>
class OTTripleGenerator : public MascotGenerator
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

    void start_progress();
    void print_progress(int k);

    void signal_multipliers(MultJob job);
    void wait_for_multipliers();

    typename T::Multiplier* new_multiplier(int i);

public:
    // TwoPartyPlayer's for OTs, n-party Player for sacrificing
    vector<TwoPartyPlayer*> players;
    vector<OTMultiplierMac<sacri_type, open_type>*> ot_multipliers;
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

    OTTripleGenerator(OTTripleSetup& setup, const Names& names,
            int thread_num, int nTriples, int nloops, MascotParams& machine,
            Player* parentPlayer = 0);
    ~OTTripleGenerator();

    void generate() { throw not_implemented(); }

    void generatePlainTriples();
    void plainTripleRound(int k = 0);

    size_t data_sent();
};

template<class T>
class NPartyTripleGenerator : public OTTripleGenerator<T>
{
    typedef typename T::open_type open_type;
    typedef typename T::mac_key_type mac_key_type;
    typedef typename T::sacri_type sacri_type;

    template <int K, int S>
    void generateTriplesZ2k();

    void generateTriples();
    void generateBits();
    template<class U, class V, class W, int N>
    void generateBitsFromTriples(vector<ShareTriple_<U, V, N> >& triples,
            W& MC, ofstream& outputFile);

    void sacrifice(vector<ShareTriple_<open_type, mac_key_type, 2> >& uncheckedTriples,
            typename T::MAC_Check& MC, PRNG& G);
    template<class U>
    void sacrificeZ2k(vector<ShareTriple_<sacri_type, mac_key_type, 2> >& uncheckedTriples,
            U& MC, PRNG& G);

public:
    vector< ShareTriple_<sacri_type, mac_key_type, 2> > uncheckedTriples;
    vector<T> bits;
    vector<InputTuple<Share<sacri_type>>> inputs;

    NPartyTripleGenerator(OTTripleSetup& setup, const Names& names,
            int thread_num, int nTriples, int nloops, MascotParams& machine,
            Player* parentPlayer = 0);

    void generate();
    void generateInputs(int player);
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

#endif
