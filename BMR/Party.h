/*
 * Party.h
 *
 */

#ifndef PROTOCOL_PARTY_H_
#define PROTOCOL_PARTY_H_

#include <mutex>
#include <boost/atomic.hpp>
#include <boost/thread.hpp>

#include "Register.h"
#include "GarbledGate.h"
#include "network/Node.h"
#include "CommonParty.h"
#include "SpdzWire.h"
#include "AndJob.h"

#include "GC/Machine.h"
#include "GC/Program.h"
#include "GC/Processor.h"
#include "GC/Secret.h"
#include "Tools/Worker.h"

class BooleanCircuit;

#define SERVER_ID (0)
#define INPUT_KEYS_MSG_TYPE_SIZE (16) // so memory will by alligned

#ifndef N_EVAL_THREADS
// Default Intel desktop processor has 8 half cores.
// This is beneficial if only one AES available per full core.
#define N_EVAL_THREADS (thread::hardware_concurrency())
#endif


typedef struct {
	unsigned long min=0;
	unsigned long long acc=0;
} exec_props_t;

class BaseParty : virtual public CommonParty {
public:
    BaseParty(party_id_t id);
    virtual ~BaseParty();

	/* From NodeUpdatable class */
	void NodeReady();
	void NewMessage(int from, ReceivedMsg& msg);
	void NodeAborted(struct sockaddr_in* from) { (void)from; }

	void Start();

	party_id_t get_id() { return _id; }
	Key get_delta() { return delta; }

protected:
	party_id_t _id;

//	int _num_evaluation_threads;
	struct timeval _start_online_net, _end_online_net;

	Timer online_timer;

	Key delta;

	virtual void _compute_prfs_outputs(Key* keys) = 0;
	void _send_prfs();

	virtual void _process_external_received(char* externals,
			party_id_t from) = 0;
	virtual void _process_all_external_received(char* externals) = 0;
	virtual void _process_input_keys(Key* keys, party_id_t from) = 0;
	virtual void _process_all_input_keys(char* keys) = 0;

	virtual void store_garbled_circuit(ReceivedMsg& msg) = 0;
	virtual void _check_evaluate() = 0;

	virtual void mask_output(ReceivedMsg& msg) = 0;
	virtual void mask_input(ReceivedMsg& msg) = 0;

	void done();

	virtual void start_online_round() = 0;

	virtual void receive_spdz_wires(ReceivedMsg& msg) = 0;
};

class Party : public BaseParty, public CommonCircuitParty {
    friend class BooleanCircuit;

public:
	Party(const char* netmap_file, const char* circuit_file, party_id_t id, const std::string input, int numthreads=5, int numtries=2);
	virtual ~Party();

	/* TEST methods */

private:
    wire_id_t _IO;

	std::string _all_input;

	int _NUMTHREADS;
	int _NUMTRIES;

	vector<GarbledGate> _garbled_tbl;

	vector<char> _input;

	vector<char> input_masks;
	vector<char>::iterator input_mask;

	SendBuffer _external_values_msg;
	int _num_externals_msg_received;
	std::mutex _process_externals_mx;

	SendBuffer _input_wire_keys_msg;
	int _num_inputkeys_msg_received;
	std::mutex _process_keys_mx;

	std::mutex _sync_mx;

#ifdef __PURE_SHE__
    Key* _sqr_keys;
    inline Key* _sqr_key(party_id_t i, wire_id_t w,int b) {return _sqr_keys+ w*2*_num_parties + b*_num_parties + i-1 ; }
#endif

    inline Key& _key(party_id_t i, wire_id_t w,int b) {return registers[w][b][i-1] ; }
    inline KeyVector& _garbled_entry(gate_id_t g, int entry) {return _garbled_tbl[g-1][entry];}
    vector<GarbledGate>::iterator get_garbled_tbl_end() { return _garbled_tbl.begin() + garbled_tbl_size; }
    void resize_garbled_tbl() { _garbled_tbl.resize(_G, _N); garbled_tbl_size = _G; }

	void _initialize_input();
	void _generate_prf_inputs();
	void _compute_prfs_outputs(Key* keys);
	void _print_keys();

	void _generate_external_values_msg();
	void _process_external_received(char* externals,
			party_id_t from);
	void _process_all_external_received(char* externals);
	void _print_input_keys_checksum();
	void _process_input_keys(Key* keys, party_id_t from);
	void _process_all_input_keys(char* keys);

	void _print_input_keys_msg();
	void _print_keys_of_party(Key *keys, int id);
	void _printf_garbled_table();

	void store_garbled_circuit(ReceivedMsg& msg);
	void load_garbled_circuit() {}

	void _check_evaluate();

	void receive_keys(Key* keys);

	void receive_spdz_wires(ReceivedMsg& msg) { (void)msg; }

	void start_online_round();

	void mask_output(ReceivedMsg& msg);
	void mask_input(ReceivedMsg& msg);

	int get_n_inputs();
};

class ProgramParty : public BaseParty
{
	friend class PRFRegister;
	friend class EvalRegister;
	friend class Register;

	char* prf_output;
	Key* keys_for_prf;

	deque<octetStream> spdz_wires[SPDZ_OP_N];
	size_t spdz_storage;
	size_t garbled_storage;
	vector<size_t> spdz_counters;

	Worker<AndJob>* eval_threads;
	vector<AndJob> and_jobs;

	ReceivedMsgStore output_masks_store;
	ReceivedMsgStore input_masks_store;

	GC::Memory< GC::Secret<EvalRegister>::DynamicType > dynamic_memory;
	GC::Machine< GC::Secret<EvalRegister> > machine;
	GC::Processor<GC::Secret<EvalRegister> > processor;
	GC::Program<GC::Secret<EvalRegister> > program;

	GC::Machine< GC::Secret<PRFRegister> > prf_machine;
	GC::Processor<GC::Secret<PRFRegister> > prf_processor;

	void _compute_prfs_outputs(Key* keys);

	void _process_external_received(char* externals,
			party_id_t from) { (void)externals; (void)from; }
	void _process_all_external_received(char* externals) { (void)externals; }
	void _process_input_keys(Key* keys, party_id_t from)
	{ (void)keys; (void)from; }
	void _process_all_input_keys(char* keys) { (void)keys; }

	void store_garbled_circuit(ReceivedMsg& msg);
	void load_garbled_circuit();

	void _check_evaluate();

	void receive_keys(Register& reg);
	void receive_all_keys(Register& reg, bool external);

	void receive_spdz_wires(ReceivedMsg& msg);

	void start_online_round();

	void mask_output(ReceivedMsg& msg) { output_masks_store.push(msg); }
	void mask_input(ReceivedMsg& msg) { input_masks_store.push(msg); }

public:
	static ProgramParty* singleton;

	ReceivedMsg garbled_circuit;
	ReceivedMsgStore garbled_circuits;

	ReceivedMsg output_masks;
	ReceivedMsg input_masks;

	MAC_Check<gf2n>* MC;
	Player* P;
	Names N;

	int threshold;

	static ProgramParty& s();

	ProgramParty(int argc, char** argv);
	~ProgramParty();

	void reset();

	void get_spdz_wire(SpdzOp op, SpdzWire& spdz_wire);

	void store_wire(const Register& reg);
	void load_wire(Register& reg);
};

inline ProgramParty& ProgramParty::s()
{
	if (singleton)
		return *singleton;
	else
		throw runtime_error("no singleton");
}

#endif /* PROTOCOL_PARTY_H_ */
