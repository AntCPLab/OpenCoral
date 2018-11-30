/*
 * CommonParty.h
 *
 */

#ifndef BMR_COMMONPARTY_H_
#define BMR_COMMONPARTY_H_

#include <vector>
using namespace std;

#include "config.h"
#include "GarbledGate.h"
#include "Register.h"
#include "proto_utils.h"
#include "network/Node.h"
#include "Tools/random.h"
#include "Auth/MAC_Check.h"
#include "Tools/time-func.h"
#include "GC/Program.h"
#include "Tools/FlexBuffer.h"

#if (defined(DEBUG) || defined(DEBUG_COMM)) && !defined(DEBUG_STEPS)
#define DEBUG_STEPS
#endif

enum SpdzOp
{
	SPDZ_LOAD,
	SPDZ_STORE,
	SPDZ_MAC,
	SPDZ_OP_N,
};

template <class T>
class PersistentFront
{
	deque<T>& container;
	int i;
public:
	PersistentFront(deque<T>& container) : container(container), i(0) {}
	T& operator*() { return container.front(); }
	void operator++(int) { i++; }
	void reset() {}
	int get_i() { return i; }
};

namespace GC
{
template<class T> class Machine;
}

class CommonParty : public NodeUpdatable
{
protected:
    friend class Register;

#ifdef N_PARTIES
    const party_id_t _N = N_PARTIES;
#else
    party_id_t _N;
#endif
    Node* _node;

    int gate_counter, gate_counter2;
    int garbled_tbl_size;

    Timer cpu_timer;
    Timer timers[2];
    Timer timer;

    gf2n mac_key;

    mutex global_lock;

    LocalBuffer wires;
    ReceivedMsgStore wire_storage;

    template<class T, class U>
    GC::BreakType first_phase(GC::Program<U>& program, GC::Processor<T>& processor,
            GC::Machine<T>& machine);
    template<class T>
    GC::BreakType second_phase(GC::Program<T>& program, GC::Processor<T>& processor,
            GC::Machine<T>& machine);

public:
    static CommonParty* singleton;
    static CommonParty& s();

    vector<SendBuffer> buffers;

    PRNG prng;

    CommonParty();
    virtual ~CommonParty();

#ifdef N_PARTIES
    static int get_n_parties() { return N_PARTIES; }
#else
    static int get_n_parties() { return s()._N; }
#endif

    void init(const char* netmap_file, int id, int n_parties);
    int init(const char* netmap_file, int id);
    virtual void reset();

    virtual party_id_t get_id() { return -1; }

    gate_id_t new_gate();
    void next_gate(GarbledGate& gate);
    gate_id_t next_gate(int skip) { return gate_counter2 += skip; }
    size_t get_garbled_tbl_size() { return garbled_tbl_size; }

    SendBuffer& get_buffer(MSG_TYPE type);

    gf2n get_mac_key() { return mac_key; }
};

class BooleanCircuit;

class CommonCircuitParty : virtual public CommonParty
{
protected:
    BooleanCircuit* _circuit;
    gate_id_t _G;
    wire_id_t _W;
    wire_id_t _OW;

    CheckVector<Register> registers;

    deque< CheckVector< CheckVector<int> > > input_regs_queue;
    PersistentFront< CheckVector< CheckVector<int> > > input_regs;
    vector<int> output_regs;

    CommonCircuitParty() : input_regs(input_regs_queue) {}

    void prepare_input_regs(party_id_t id);
    void prepare_output_regs();

    void resize_registers() { registers.resize(_W, {(int)_N}); }

    Register& get_reg(int reg);

    void print_masks(const vector<int>& indices);
    void print_outputs(const vector<int>& indices);
    void print_round_regs();
};


inline Register& CommonCircuitParty::get_reg(int reg)
{
#ifdef DEBUG_REGS
	cout << "get reg " << reg << endl << registers.at(reg).keys[0] << endl
			<< registers.at(reg).keys[1] << endl;
#endif
	return registers.at(reg);
}

inline CommonParty& CommonParty::s()
{
    if (singleton)
        return *singleton;
    else
        throw runtime_error("no singleton");
}

#endif /* BMR_COMMONPARTY_H_ */
