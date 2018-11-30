/*
 * Party.cpp
 *
 */

#include "Party.h"


#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>

#include "Tools/callgrind.h"

#include "proto_utils.h"
#include "msg_types.h"
#include "prf.h"
#include "BooleanCircuit.h"
#include "Math/Setup.h"

#ifdef __PURE_SHE__
#include "mpirxx.h"
#endif

ProgramParty* ProgramParty::singleton = 0;


BaseParty::BaseParty(party_id_t id) : _id(id)
{
#ifdef DEBUG_PRNG_PARTY
	octet seed[SEED_SIZE];
	memset(seed, 0, sizeof(seed));
	seed[0] = id;
	prng.SetSeed(seed);
#endif
}

BaseParty::~BaseParty()
{
}

Party::Party(const char* netmap_file, // required to init Node
		   const char* circuit_file, // required to init BooleanCircuit
		   party_id_t id,
		   const std::string input,
		   int numthreads,
		   int numtries
		   ) :BaseParty(id),
		   	  _all_input(input),
			  _NUMTHREADS(numthreads),
			  _NUMTRIES(numtries)
{
	_circuit = new BooleanCircuit( circuit_file );
	_circuit->party = this;
	_G = _circuit->NumGates();
#ifndef N_PARTIES
    _N = _circuit->NumParties();
#endif
	_W = _circuit->NumWires();
	_OW = _circuit->NumOutWires();
	_IO = _circuit->_num_input_wires;
	resize_registers();
#ifdef __PURE_SHE__
	init_modulos();
#endif
	reset();
	_generate_prf_inputs();
	init(netmap_file, id, _N);
	_num_externals_msg_received = {0};//ATOMIC_VAR_INIT(0);
	_num_inputkeys_msg_received = {0};//ATOMIC_VAR_INIT(0);
}

void Party::_initialize_input()
{
	party_t me = _circuit->_parties[_id];
	std::string my_input = _all_input.substr(me.wires, me.n_wires);
	_input.resize(me.n_wires);
	const char* input = my_input.c_str();
	memcpy(_input.data(), input, me.n_wires);

	for(size_t i=0; i<me.n_wires; i++) {
		_input[i]-=0x30;
	}
#ifdef DEBUG
	printf("inputs:\n");
	phex(_input.data(), me.n_wires);
#endif
}

Party::~Party() {
}

void BaseParty::NodeReady()
{
	printf("Node is ready\n");
}

void BaseParty::NewMessage(int from, ReceivedMsg& msg)
{
	char* message = msg.data();
	size_t len = msg.size();
//	printf("got message of len %u from %d\n", len, from);
	MSG_TYPE message_type;
	msg.unserialize(message_type);
#ifdef DEBUG_STEPS
	cout << "processing " << message_type_names[message_type] << " from " << dec
			<< from << " of length " << msg.size() << endl;
#endif
	unique_lock<mutex> locker(global_lock);
	switch(message_type) {
	case TYPE_KEYS:
		{
#ifdef DEBUG_STEPS
		printf("TYPE_KEYS\n");
#endif
#ifdef DEBUG2
		cout << "received keys" << endl;
		phex(message, len);
#endif
		get_buffer(TYPE_PRF_OUTPUTS);
		_compute_prfs_outputs((Key*)(message + MSG_KEYS_HEADER_SZ));
		_send_prfs();
//		printf("sent prfs\n");
		break;
		}
	case TYPE_MASK_INPUTS:
		{
#ifdef DEBUG_STEPS
		printf("TYPE_MASK_INPUTS\n");
#endif
#ifdef DEBUG_INPUT
		cout << "received " << msg.left() << " input masks" << endl;
#endif
		mask_input(msg);
		break;
		}
	case TYPE_MASK_OUTPUT:
		{
#ifdef DEBUG_STEPS
		printf("TYPE_MASK_OUTPUT\n");
#endif
#ifdef DEBUG_OUTPUT_MASKS
		cout << "receiving " << msg.left() << " output masks" << endl;
#endif
		mask_output(msg);
		break;
		}
	case TYPE_GARBLED_CIRCUIT:
	{
#ifdef DEBUG_STEPS
		printf("TYPE_GARBLED_CIRCUIT\n");
#endif
#ifdef DEBUG2
		phex(message, len);
#endif
#ifdef DEBUG_COMM
		cout << "got " << len << " bytes for " << get_garbled_tbl_size() << " gates" << endl;
#endif
		if ((len - 4) != 4 * _N * sizeof(Key) * get_garbled_tbl_size())
			throw runtime_error("wrong size of garbled table");
		store_garbled_circuit(msg);

//				printf("\nGarbled Table\n\n");
//				_printf_garbled_table();
//		char garbled_circuit_cs = cs((char*)_garbled_tbl , garbled_tbl_sz);
//		printf ("\ngarbled_circuit_cs = %d\n", garbled_circuit_cs);

		_node->Send(SERVER_ID, get_buffer(TYPE_RECEIVED_GC));

		break;
	}
	case TYPE_EXTERNAL_VALUES:
	{
		//this is done by party 1 only
#ifdef DEBUG_STEPS
		printf("TYPE_EXTERNAL_VALUES from %d\n",from);
#endif
		_process_external_received(message+sizeof(MSG_TYPE), from);
		break;
	}
	case TYPE_ALL_EXTERNAL_VALUES:
	{
#ifdef DEBUG_STEPS
		printf("TYPE_ALL_EXTERNAL_VALUES\n");
#endif
		_process_all_external_received(message+sizeof(MSG_TYPE));
		break;
	}
	case TYPE_KEY_PER_IN_WIRE:
	{
#ifdef DEBUG_STEPS
		printf("TYPE_KEY_PER_IN_WIRE from %d\n", from);
#endif
		_process_input_keys((Key*)(message+INPUT_KEYS_MSG_TYPE_SIZE), from);
		break;
	}
	case TYPE_ALL_KEYS_PER_IN_WIRE:
	{
#ifdef DEBUG_STEPS
					printf("TYPE_ALL_KEYS_PER_IN_WIRE\n");
#endif
		_process_all_input_keys(message+INPUT_KEYS_MSG_TYPE_SIZE);
		break;
	}
	case TYPE_LAUNCH_ONLINE:
	{
#ifdef DEBUG_STEPS
		printf("TYPE_LAUNCH_ONLINE\n");
#endif
		cout << "Launching online phase at " << timer.elapsed() << endl;
		_node->print_waiting();
		// store a token item in case it's needed just before ending the program
		ReceivedMsg msg;
		// to avoid copying from address 0
		msg.resize(1);
		// for valgrind
		msg.data()[0] = 0;
		// token input round
		store_garbled_circuit(msg);
		online_timer.start();
		_start_online_net = GET_TIME();
		start_online_round();
		break;
	}
	case TYPE_CHECKSUM:
		{
		printf("TYPE_CHECKSUM\n");
		printf("got checksum = %d\n", message[sizeof(MSG_TYPE)]);
		break;
		}
	case TYPE_SPDZ_WIRES:
		receive_spdz_wires(msg);
		break;
	case TYPE_DELTA:
		msg.unserialize(delta);
		break;
	default:
		{
		printf("UNDEFINED\n");
		printf("got undefined message\n");
		phex(message, len);
		break;
		}
	}

#ifdef DEBUG_STEPS
	cout << "done with " << message_type_names[message_type] << " from " << from << endl;
#endif
}

void Party::start_online_round()
{
		_generate_external_values_msg();
//		_node->Broadcast2(_external_values_msg, _external_values_msg_sz);
		if(_id!=1) {
//					printf("sending my externals:\n");
//					phex(_external_values_msg+sizeof(MSG_TYPE), _circuit->Party(_id).n_wires);
			_node->Send(1,_external_values_msg);
		}
}

void Party::store_garbled_circuit(ReceivedMsg& msg)
{
	auto end = get_garbled_tbl_end();
	for (auto it = _garbled_tbl.begin(); it != end; it++)
	{
		it->unserialize(msg, _N);
#ifdef DEBUG
		it->print();
#endif
	}
}

void Party::_check_evaluate()
{
	load_garbled_circuit();
	_end_online_net = GET_TIME();
	printf("Network time: ");
	PRINT_DIFF(&_start_online_net, &_end_online_net);
#ifdef __PRIME_FIELD__
	printf("this implementation uses PRIME FIELD\n");
#endif

	printf("\n\npress for EVALUATING\n\n");
	getchar();

	vector<exec_props_t> execs(_NUMTHREADS+1);
	unsigned long diff;
	for (int ntry=1; ntry<=_NUMTRIES; ntry++) {
		for(int nthreads=1; nthreads<=_NUMTHREADS; nthreads+=1) {
//			_garbled_tbl = _garbled_tbl_copy;
//				printf("num threads = %d\n", nthreads);

//					CALLGRIND_START_INSTRUMENTATION;
			struct timeval b;
			if(nthreads == 1) {
				b = GET_TIME();
//					printf("Linear evaluation\n");
				_circuit->EvaluateByLayerLinearly(_id);
			} else {
				b = GET_TIME();
//					printf("Non linear evaluation\n");
				_circuit->EvaluateByLayer(nthreads, _id);
			}
//					CALLGRIND_STOP_INSTRUMENTATION;
					CALLGRIND_DUMP_STATS;
			struct timeval a = GET_TIME();
//				_circuit->Output();

//				printf("Eval time: ");
			diff = GET_DIFF(&b, &a);
//			printf("#threads = %d, time = %lu\n", nthreads, diff);
			execs[nthreads].acc += diff;
			if (diff < execs[nthreads].min || execs[nthreads].min==0) {
				execs[nthreads].min = diff;
			}
		}
	}

#ifdef DEBUG
	_circuit->Output();
#endif

	//getting the best results
	unsigned long minmin = execs[1].min;
	unsigned long long avgmin = execs[1].acc;
	int best_minmin  = 1;
	int best_avgmin  = 1;
	for(int nthreads=1; nthreads<=_NUMTHREADS; nthreads++) {
		printf("#nthreads = %d, min = %lu, acc = %llu\n",nthreads, execs[nthreads].min, execs[nthreads].acc);
		if(execs[nthreads].min < minmin) {
			minmin = execs[nthreads].min;
			best_minmin = nthreads;
		}
		if(execs[nthreads].acc < avgmin) {
			avgmin = execs[nthreads].acc;
			best_avgmin = nthreads;
		}
	}
	printf("\nRESULTS SUM:\n");
	printf("Got minimal time with %d threads: %ld\n", best_minmin, minmin);
	printf("Got minimal AVERAGE time with %d threads: %llu\n", best_avgmin, avgmin/_NUMTRIES);
	fflush(0);
	done();
}

void BaseParty::Start()
{
	_node->Start();
}


void Party::_generate_prf_inputs()
{
	resize_garbled_tbl();
	for (gate_id_t g=1; g<=_G; g++)
	{
		_garbled_tbl[g-1].init_inputs(g, _N);
	}
}

void Party::_compute_prfs_outputs(Key* keys)
{
    receive_keys(keys);
    for(gate_id_t g=1; g<=_G; g++) {
		const Register* in_wires[2] = {&registers[_circuit->_gates[g]._left], &registers[_circuit->_gates[g]._right]};
		_garbled_tbl[g-1].compute_prfs_outputs(in_wires, _id, buffers[TYPE_PRF_OUTPUTS], g);
	}
	printf("\n\n");
}

void BaseParty::_send_prfs() {
	_node->Send(SERVER_ID, buffers[TYPE_PRF_OUTPUTS]);
#ifdef DEBUG2
	printf("Send PRFs:\n");
	phex(buffers[TYPE_PRF_OUTPUTS]);
#endif
}

void Party::_print_keys()
{
	for (wire_id_t w=0; w<_IO; w++) {
		registers[w].keys.print(w, _id);
	}
}


void Party::_printf_garbled_table()
{
	for (gate_id_t g=1; g<=get_garbled_tbl_size(); g++) {
		std::cout << "gate " << g << std::endl;
		for(int entry=0; entry<4; entry++) {
			for(party_id_t i=1; i<=_N; i++) {
				KeyVector& k = _garbled_entry(g, entry);
				std::cout << k[i-1] << " ";
			}
			std::cout << std::endl;
		}
		std::cout << std::endl;
	}
}

void Party::_print_keys_of_party(Key *keys, int id)
{
	printf("\nkeys for party %d", id);
	for(wire_id_t w=0; w<_IO; w++) {
		printf("k^%d_%lu: ", id, w);
		std::cout << keys[w] << std::endl;
	}
}

void Party::_print_input_keys_msg()
{
	for(wire_id_t w=0; w<_IO; w++) {
		std::cout << *(Key*)(_input_wire_keys_msg.data()+INPUT_KEYS_MSG_TYPE_SIZE+w*sizeof(Key)) << std::endl;
	}
}



void Party::_generate_external_values_msg()
{
	_initialize_input();
    prepare_input_regs(_id);
	_external_values_msg.clear();
	fill_message_type(_external_values_msg, TYPE_EXTERNAL_VALUES);
	unsigned int n = (*input_regs)[_id].size();
#ifdef DEBUG_ROUNDS
	cout << dec << _input.size() << "/" << n <<  " inputs" << " in round "
			<< input_regs.get_i() << endl;
#endif
	if (n != _input.size())
		throw runtime_error("number of inputs doesn't match");
	for(unsigned int i=0; i<n; i++) {
		if (input_mask == input_masks.end())
			throw runtime_error("not enough masks");
		char val = *input_mask^_input[i];
		input_mask++;
		_external_values_msg.push_back(val);
		get_reg((*input_regs)[_id][i]).set_external(val);
	}
#ifdef DEBUG_VALUES
	cout << "on registers:" << endl;
	for(unsigned int i=0; i<n; i++)
			cout << (*input_regs)[_id][i] << " ";
	cout << endl;
	cout << "inputs:\t\t\t";
	print_bit_array(_input.data(), n);
	cout << "masks:\t\t\t";
	print_bit_array(&*(input_mask - n), n);
	cout << "externals:\t\t";
	print_bit_array(&_external_values_msg[4], n);
#endif

#ifdef DEBUG2
					phex(&*(input_mask - n), n);
					phex(_input.data(), n);
					phex(_external_values_msg.data()+sizeof(MSG_TYPE), n);
#endif
}

void Party::_process_external_received(char* externals, party_id_t from)
{
	prepare_input_regs(from);

//	printf("received externals from %d\n", from);
//	phex(externals, n);
	for(unsigned int i=0; i<(*input_regs)[from].size(); i++) {
		get_reg((*input_regs)[from][i]).set_external(externals[i]);
	}
#ifdef DEBUG_VALUES
	cout << "externals from " << from << ":\t";
	print_bit_array(externals, (*input_regs)[from].size());
	cout << "masks:\t\t\t";
	print_masks((*input_regs)[from]);
	cout << "outputs:\t\t";
	print_masks((*input_regs)[from]);
	cout << "on registers:" << endl;
	print_indices((*input_regs)[from]);
#endif

		size_t num_received;
		{
		std::unique_lock<std::mutex> locker(_process_externals_mx);
		num_received = ++_num_externals_msg_received;
		}
		if(num_received == _N-1) {
			_num_externals_msg_received = 0;
			SendBuffer& buffer = get_buffer(TYPE_ALL_EXTERNAL_VALUES);
			int w = 0;
			for (size_t i = 0; i < _N; i++)
			{
				prepare_input_regs(i + 1);
				vector<int>& regs = (*input_regs)[i+1];
				for (unsigned int j = 0; j < regs.size(); j++)
				{
					buffer.push_back(registers[regs[j]].get_external());
					w++;
				}
			}
			_node->Broadcast2(buffer);
//			printf("sending all externals\n");
//			phex(all_externals+sizeof(MSG_TYPE), _IO);
		}
}

void Party::_process_all_external_received(char* externals)
{
//		phex(message+sizeof(MSG_TYPE), _IO);
		_input_wire_keys_msg.clear();
		fill_message_type(_input_wire_keys_msg, TYPE_KEY_PER_IN_WIRE);
		_input_wire_keys_msg.resize(INPUT_KEYS_MSG_TYPE_SIZE);

	int w = 0;
	for (size_t i = 0; i < _N; i++)
	{
		prepare_input_regs(i + 1);
		vector<int>& regs = (*input_regs)[i+1];
		for (unsigned int j = 0; j < regs.size(); j++)
		{
			get_reg(regs[j]).set_external(externals[w]);
			get_reg(regs[j]).external_key(_id).serialize(_input_wire_keys_msg);
#ifdef DEBUG
			printf("k^%d_{%u,%d}=", _id,w,registers[regs[j]].get_external());
			std::cout << registers[regs[j]].external_key(_id) << std::endl;
#endif
			w++;
		}
#ifdef DEBUG_VALUES
		cout << "on registers:" << endl;
		for (unsigned j = 0; j < (*input_regs)[i + 1].size(); j++)
			cout << (*input_regs)[i + 1][j] << " ";
		cout << endl;
		cout << "externals from " << (i + 1) << ":\t";
		print_bit_array(externals, regs.size());
		cout << "masks:\t\t\t";
		print_masks(output_regs);
		cout << "outputs:\t\t";
		print_outputs(output_regs);
#endif
	}

		_node->Send(1, _input_wire_keys_msg);
#ifdef DEBUG2
		printf("input wire keys:\n");
		phex(_input_wire_keys_msg);
#endif
}

//void Party::_process_input_keys(char* keys, party_id_t from)
//{
////	printf("keys from party: %d\n",from);
//	for(wire_id_t w=0; w<_IO; w++) {
//		unsigned int offset = w*2*_N+_circuit->_externals[w]*_N+from-1;
//		_circuit->_keys[offset] = *(Key*)(keys+w*sizeof(Key));
////		printf("received key k^%d_{%u,%d}=", from,w,_circuit->_externals[w]);
////		std::cout <<_circuit->_keys[offset] << std::endl;
//	}
//}

void Party::_process_input_keys(Key* keys, party_id_t from)
{
//	printf("keys from party: %d\n",from);
//	for(wire_id_t w=0; w<_IO; w++) {
//		printf("k^%d_{%u,%d}=", from,w,_circuit->_externals[w]);
//		std::cout << keys[w] << std::endl;
//	}

	int w = 0;
	for (size_t i = 0; i < _N; i++)
	{
		prepare_input_regs(i + 1);
		for (unsigned int j = 0; j < (*input_regs)[i+1].size(); j++)
		{
			get_reg((*input_regs)[i+1][j]).set_external_key(from, keys[w]);
			w++;
		}
	}

		size_t num_received;
		{
			std::unique_lock<std::mutex> locker(_process_keys_mx);
			num_received = ++_num_inputkeys_msg_received;
		}
//		printf("num_received = %d\n",num_received);
		if(num_received == _N-1)
		{
			_num_inputkeys_msg_received = 0;
#ifdef DEBUG_STEPS
			printf("received input keys from everyone\n");
#endif
			SendBuffer& buffer = get_buffer(TYPE_ALL_KEYS_PER_IN_WIRE);
			buffer.resize(INPUT_KEYS_MSG_TYPE_SIZE);
			int w = 0;
			for (size_t i = 0; i < _N; i++)
			{
				prepare_input_regs(i + 1);
				for (unsigned int j = 0; j < (*input_regs)[i+1].size(); j++)
				{
					get_reg((*input_regs)[i+1][j]).keys.serialize(buffer);
					w++;
				}
			}
#ifdef DEBUG2
						printf("all input keys:\n");
						_print_input_keys_checksum();
						phex(buffer);
#endif
			_node->Broadcast2(buffer);
			_check_evaluate();
		}

}


void Party::_process_all_input_keys(char* keys)
{
	/* keys: a block containing 2*_N keys for each input wire so the
	 * receiver only has to copy one time.
	*/
	int w = 0;
	for (size_t i = 0; i < _N; i++)
	{
		prepare_input_regs(i + 1);
		for (unsigned int j = 0; j < (*input_regs)[i+1].size(); j++)
		{
			get_reg((*input_regs)[i+1][j]).set_eval_keys((Key*)keys+w*2*_N, _N, _id - 1);
			w++;
		}
	}

#ifdef DEBUG
					printf("all input keys:\n");
//					phex(message+INPUT_KEYS_MSG_TYPE_SIZE, 2*_N*_IO*sizeof(Key));
					_print_input_keys_checksum();
#endif
		_check_evaluate();
}

void Party::_print_input_keys_checksum()
{
	int w = 0;
	for (unsigned int i = 0; i < (*input_regs).size(); i++)
		for (unsigned int j = 0; j < (*input_regs)[i].size(); j++)
			registers[(*input_regs)[i][j]].print_input(w++);
}

void Party::mask_output(ReceivedMsg& msg)
{
	char* masks = msg.consume(0);
	size_t n_masks = msg.left();
	prepare_output_regs();
	if (n_masks != output_regs.size())
		throw runtime_error("number of masks doesn't match");
	for (unsigned int i = 0; i < output_regs.size(); i++)
		get_reg(output_regs[i]).set_mask(masks[i]);
#ifdef DEBUG_VALUES
	cout << "received output masks in registers:" << endl;
	vector<char> externals(output_regs.size()), outputs(output_regs.size());
	for (unsigned int i = 0; i < output_regs.size(); i++)
	{
		externals[i] = get_reg(output_regs[i]).get_external_no_check();
		outputs[i] = get_reg(output_regs[i]).get_output_no_check();
		cout << output_regs[i] << " ";
	}
	cout << endl;
	cout << "masks:\t\t\t";
	print_bit_array(masks, output_regs.size());
	cout << "externals:\t\t";
	print_bit_array(externals);
	cout << "outputs:\t\t";
	print_bit_array(outputs);
#endif
}

void Party::mask_input(ReceivedMsg& msg)
{
	input_masks.insert(input_masks.end(), msg.data() + sizeof(MSG_TYPE), msg.data() + msg.size());
#ifdef DEBUG_COMM
	cout << "got " << dec << input_masks.size() << " input masks" << endl;
#endif
	input_mask = input_masks.begin();
}

void Party::receive_keys(Key* keys) {
	resize_registers();
	for (size_t w = 0; w < _W; w++)
	{
		registers[w].init(_N);
		for (int i = 0; i < 2; i++)
			registers[w].keys[i][_id - 1] = *(keys + w * 2 + i);
	}
#ifdef DEBUG
                _print_keys();
#endif
}

int Party::get_n_inputs() {
	int res = 0;
	for (size_t i = 0; i < _N; i++)
	{
		prepare_input_regs(i + 1);
		vector<int>& regs = (*input_regs)[i+1];
		res += regs.size();
	}
	return res;
}

void BaseParty::done() {
	cout << "Online phase took " << online_timer.elapsed() << " seconds" << endl;
	_node->Send(SERVER_ID, get_buffer(TYPE_DONE));
	_node->Stop();
}

ProgramParty::ProgramParty(int argc, char** argv) :
        		BaseParty(-1), keys_for_prf(0),
        		spdz_storage(0), garbled_storage(0), spdz_counters(SPDZ_OP_N),
				machine(dynamic_memory),
        		processor(machine), prf_machine(dynamic_memory),
        		prf_processor(prf_machine),
				MC(0)
{
	if (argc < 3)
	{
		cerr << "Usage: " << argv[0] << " <id> <program> [netmap]" << endl;
		exit(1);
	}

	_id = atoi(argv[1]);
	program.parse(string(argv[2]) + "-0");
	machine.reset(program);
	processor.reset(program);
	processor.open_input_file("user_inputs/user_" + to_string(_id - 1) + "_input.txt");
	prf_machine.reset(*reinterpret_cast<GC::Program<GC::Secret<PRFRegister> >* >(&program));
	prf_processor.reset(*reinterpret_cast<GC::Program<GC::Secret<PRFRegister> >* >(&program));
	if (singleton)
		throw runtime_error("there can only be one");
	singleton = this;
	if (argc > 3)
	{
		int n_parties = init(argv[3], _id);
		ifstream netmap(argv[3]);
		int tmp;
		string tmp2;
		netmap >> tmp >> tmp2 >> tmp;
		vector<string> hostnames(n_parties);
		for (int i = 0; i < n_parties; i++)
		{
			netmap >> hostnames[i];
			netmap >> tmp;
		}
		N.init(_id - 1, 5000, hostnames);
	}
	else
	{
		int n_parties = init("LOOPBACK", _id);
		N.init(_id - 1, 5000, vector<string>(n_parties, "localhost"));
	}
	prf_output = (char*)new __m128i[PAD_TO_8(get_n_parties())];
	mac_key = prng.get_word() & ((1ULL << GC::Secret<EvalRegister>::default_length) - 1);
	cout << "MAC key: " << hex << mac_key << endl;
	ifstream schfile((string("Programs/Schedules/") + argv[2] + ".sch").c_str());
	string curr, prev;
	while (schfile.good())
	{
		prev = curr;
		getline(schfile, curr);
	}
	cout << "Compiler: " << prev << endl;
	P = new PlainPlayer(N, 0);
	if (argc > 4)
		threshold = atoi(argv[4]);
	else
		threshold = 128;
	cout << "Threshold for multi-threaded evaluation: " << threshold << endl;
	eval_threads = new Worker<AndJob>[N_EVAL_THREADS];
	and_jobs.resize(N_EVAL_THREADS);
}

ProgramParty::~ProgramParty()
{
	reset();
	delete[] prf_output;
	delete P;
	if (MC)
		delete MC;
	delete[] eval_threads;
	cout << "SPDZ loading: " << spdz_counters[SPDZ_LOAD] << endl;
	cout << "SPDZ storing: " << spdz_counters[SPDZ_STORE] << endl;
	cout << "SPDZ wire storage: " << 1e-9 * spdz_storage << " GB" << endl;
	cout << "Dynamic storage: " << 1e-9 * dynamic_memory.capacity() * 
			sizeof(GC::Secret<EvalRegister>::DynamicType) << " GB" << endl;
	cout << "Maximum circuit storage: " << 1e-9 * garbled_storage << " GB" << endl;
}

void ProgramParty::_compute_prfs_outputs(Key* keys)
{
	keys_for_prf = keys;
	first_phase(program, prf_processor, prf_machine);
}

void ProgramParty::reset()
{
	CommonParty::reset();
}

void ProgramParty::store_garbled_circuit(ReceivedMsg& msg)
{
	garbled_storage = max(msg.size(), garbled_storage);
	garbled_circuits.push(msg);
}

void ProgramParty::load_garbled_circuit()
{
	if (not garbled_circuits.pop(garbled_circuit))
		throw runtime_error("no garbled circuit available");
	if (not output_masks_store.pop(output_masks))
		throw runtime_error("no output masks available");
	if (not input_masks_store.pop(input_masks))
		throw runtime_error("no input masks available");
#ifdef DEBUG_OUTPUT_MASKS
	cout << "loaded " << output_masks.left() << " output masks" << endl;
#endif
#ifdef DEBUG_INPUT
    cout << "loaded " << input_masks.left() << " input masks" << endl;
#endif
}

void ProgramParty::start_online_round()
{
	machine.reset_timer();
	_check_evaluate();
}

void ProgramParty::_check_evaluate()
{
#ifdef DEBUG_REGS
	print_round_regs();
#endif
	cout << "Online time at evaluation start: " << online_timer.elapsed()
			<< endl;
	GC::BreakType next = GC::TIME_BREAK;
	while (next == GC::TIME_BREAK)
	{
		load_garbled_circuit();
		next = second_phase(program, processor, machine);
	}
	cout << "Online time at evaluation stop: " << online_timer.elapsed()
			<< endl;
	if (next == GC::TIME_BREAK)
	{
#ifdef DEBUG_STEPS
		cout << "another round of garbling" << endl;
#endif
	}
	if (next != GC::DONE_BREAK)
	    {
#ifdef DEBUG_STEPS
		cout << "another round of evaluation" << endl;
#endif
		start_online_round();
	    }
	else
	{
		Timer timer;
		timer.start();
		MC->Check(*P);
		cout << "Final check took " << timer.elapsed() << endl;
		done();
	}
}

void ProgramParty::receive_keys(Register& reg)
{
	reg.init(_N);
	for (int i = 0; i < 2; i++)
		reg.keys[i][_id-1] = *(keys_for_prf++);
#ifdef DEBUG
	cout << "receive keys " << reg.get_id() << "(" << &reg << ") " << dec << reg.keys[0].size() << endl;
	reg.keys.print(reg.get_id(), _id);
	cout << "delta " << reg.get_id() << " " << (reg.keys[0][_id-1] ^ reg.keys[1][_id-1]) << endl;
#endif
}

void ProgramParty::receive_all_keys(Register& reg, bool external)
{
	reg.init(get_n_parties());
	for (int i = 0; i < get_n_parties(); i++)
		reg.keys[external][i] = *(keys_for_prf++);
}

void ProgramParty::receive_spdz_wires(ReceivedMsg& msg)
{
	int op;
	msg.unserialize(op);
	spdz_wires[op].push_back({});
	size_t l = msg.left();
	spdz_wires[op].back().append((octet*)msg.consume(l), l);
	spdz_storage += l;
#ifdef DEBUG_SPDZ_WIRES
	cout << "receive " << dec << spdz_wires[op].back().get_length() << "/"
			<< msg.size() << " bytes for type " << op << endl;
#endif
	if (op == SPDZ_MAC)
	{
		gf2n spdz_mac_key;
		spdz_mac_key.unpack(spdz_wires[op].back());
		if (!MC)
		{
			MC = new Passing_MAC_Check<gf2n>(spdz_mac_key, N, 0);
			cout << "MAC key: " << hex << spdz_mac_key << endl;
			mac_key = spdz_mac_key;
		}
	}
}

void ProgramParty::get_spdz_wire(SpdzOp op, SpdzWire& spdz_wire)
{
	while (true)
	{
		if (spdz_wires[op].empty())
			throw runtime_error("no SPDZ wires available");
		if (spdz_wires[op].front().done())
			spdz_wires[op].pop_front();
		else
			break;
	}
	spdz_wire.unpack(spdz_wires[op].front(), get_n_parties());
	spdz_counters[op]++;
#ifdef DEBUG_SPDZ_WIRE
	cout << "get SPDZ wire of type " << op << ", " << spdz_wires[op].front().left() << " bytes left" << endl;
	cout << "mask share for " << get_id() << ": " << spdz_wire.mask << endl;
#endif
}

void ProgramParty::store_wire(const Register& reg)
{
	wires.serialize(reg.key(get_id(), 0));
#ifndef FREE_XOR
	wires.serialize(reg.key(get_id(), 1));
#endif
#ifdef DEBUG
	cout << "storing wire" << endl;
	reg.print();
#endif
}

void ProgramParty::load_wire(Register& reg)
{
	wires.unserialize(reg.key(get_id(), 0));
#ifdef FREE_XOR
	reg.key(get_id(), 1) = reg.key(get_id(), 0) ^ get_delta();
#else
	wires.unserialize(reg.key(get_id(), 1));
#endif
#ifdef DEBUG
	cout << "loading wire" << endl;
	reg.print();
#endif
}
