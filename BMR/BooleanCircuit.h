// (C) 2018 University of Bristol, Bar-Ilan University. See License.txt


#ifndef __BOOLEAN_CIRCUIT__
#define __BOOLEAN_CIRCUIT__

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>
#include <queue>
#include <set>
#include <atomic>

#include <boost/thread/mutex.hpp>
#include <boost/thread.hpp>

#include <mutex>
#include <condition_variable>

#include "proto_utils.h"
#include "common.h"
#include "Gate.h"
#include "Wire.h"
#include "Key.h"
#include "Register.h"
#include "GarbledGate.h"

#include "Party.h"


#define INIT_PARTY(W,N) {.wires=W, .n_wires=N }
typedef struct party_t{
	unsigned int wires; //the index of the first circuit input wire associated with this party
	unsigned int n_wires; //number of circuit input wires associated with this party
	void init(int w, int n){wires=w; n_wires=n;}
} party_t;


#define PRFS_PER_PARTY(G,n) (8*G*n)
#define RESERVE_FOR_MSG_TYPE (16) //Apparently must be a divided by 16 in order the aes encryption to work
#define GARBLED_GATE_SIZE(N) (4*N)
#define MSG_KEYS_HEADER_SZ (16)

class BooleanCircuit
{
	friend class Party;
	friend class TrustedParty;
	friend class CommonCircuitParty;
public:
	BooleanCircuit(const char* desc_file);
//	void RawInputs(std::string raw_inputs);
	void Inputs(const char* inputs_file);
	void Evaluate(int num_threads, party_id_t my_id);
	void EvaluateByLayer(int num_threads, party_id_t my_id);
	void EvaluateByLayerLinearly(party_id_t my_id);
	/*virtual*/ std::string Output();

	/* Usefull getters/setters */
	inline wire_id_t NumWires() { return _wires.size(); }
	inline wire_id_t NumOutWires() { return _num_output_wires; }
	inline wire_id_t OutWiresStart() { return _output_start; }
	inline gate_id_t NumGates() {return _num_gates; }
	inline wire_id_t NumParties() { return _num_parties; }
	inline party_t get_party(party_id_t id) {return _parties[id];}


private:
	gate_id_t	_num_gates;
	party_id_t	_num_parties;
	wire_id_t	_num_input_wires;
	wire_id_t	_num_output_wires;
	wire_id_t	_output_start; // output wires starts from that index

	std::vector<Gate> _gates; //begins with 1
	std::vector<Wire> _wires; //begins with 0
	std::vector<party_t> _parties; //begins with 1
	std::vector<gate_id_t> _input_gates; // ease the launch of the evaluation
	

	std::vector<std::vector<gate_id_t>> _layers;
	size_t _max_layer_sz;
	void _make_layers();
	int __make_layers(gate_id_t g);
	void _add_to_layer(int layer, gate_id_t g);
	void _print_layers();
	void _validate_layers();

	std::mutex _layers_mx;
	std::condition_variable _layers_cv;
	std::atomic_int _num_threads_finished;
	std::atomic_int _current_layer;
	std::atomic_int _evaluator_initiated;
	void _eval_by_layer(int i, int num_threads, party_id_t my_id);
	std::mutex _coordinator_mx;
	std::condition_variable _coordinator_cv;

	std::set<gate_id_t> _ready_gates_set;
	std::atomic_int _num_evaluated_out_wires;
	boost::mutex _ready_mx, _inc_mx;

	void _parse_circuit(const char* desc_file);
	void _eval_thread(party_id_t my_id);
#ifdef __PURE_SHE__
	void _eval_gate(gate_id_t g, party_id_t my_id, char* prf_output, mpz_t& tmp_mpz);
#else
	void _eval_gate(gate_id_t g, party_id_t my_id, char* prf_output);
#endif

	inline bool is_wire_ready(wire_id_t w) {
		return party->registers[w].get_external() != NO_SIGNAL;
	}
	inline bool is_gate_ready(gate_id_t g) {
		bool ready = is_wire_ready(_gates[g]._left)
						&& is_wire_ready(_gates[g]._right);
		return ready;
	}

	Party* party;
};



#endif
