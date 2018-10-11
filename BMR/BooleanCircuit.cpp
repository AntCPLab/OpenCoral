// (C) 2018 University of Bristol, Bar-Ilan University. See License.txt


#include "BooleanCircuit.h"

#include "prf.h"

//static void throw_party_exists(pid_t pid, unsigned int pos) {
//	std::cout << "ERROR: in circuit description" << std::endl
//			  << "\tPosition: " << pos << std::endl
//			  << "\tPlayer id " << pid << " already exists" << std::endl;
//	throw std::invalid_argument( "player id error" );
//}

static void throw_bad_circuit_file() {
	std::cout << "ERROR: could not read circuit file" << std::endl;
	throw std::invalid_argument( "bad circuit file" );
}

void BooleanCircuit::_parse_circuit(const char* desc_file)
{
	std::ifstream circuit_file(desc_file);
	if(!circuit_file.good()) throw_bad_circuit_file();

	circuit_file >> _num_gates;
		_gates.resize(_num_gates+1); //+1 since gates are indexed from 1...
	circuit_file >> _num_parties;
		_parties.resize(_num_parties+1); //+1 since parties are indexed from 1...
	unsigned int total_input_wires;
	circuit_file >> total_input_wires;

	for (size_t idx_party = 1; idx_party <= _num_parties; idx_party++) {
		unsigned int num_input_wires = total_input_wires/_num_parties;
		if (idx_party == _num_parties) {
			num_input_wires = total_input_wires-(total_input_wires/_num_parties)*(_num_parties-1);
		}
		_parties[idx_party].init(_num_input_wires,num_input_wires);
		std::cout << "party " << idx_party << ": " <<  _parties[idx_party].wires << " " << _parties[idx_party].n_wires << std::endl;
		_num_input_wires += num_input_wires;
	}

	/* Parse output wires */
	circuit_file >> _num_output_wires;
	circuit_file >> _output_start;
	std::cout << "# output wires: " << _num_output_wires << " starts at " << _output_start << std::endl;
	int largest_output_wire = _output_start;
	for (unsigned int i = 1; i < _num_output_wires; i++) //NOTE: starts with i=1 !
		circuit_file >> largest_output_wire; // ignore all but the last one
	_wires.resize(_output_start, Wire(false));
	_wires.resize(largest_output_wire+1, Wire(true));
//	for(int j=0; j<_wires.size(); j++) {
//			_wires[j].print(j);
//		}

	/* Parse gates */
	for (gate_id_t gate_id = 1; gate_id <= _num_gates; gate_id++)
	{
		size_t fan_in, fan_out, left, right, out;
		std::string func;

		circuit_file >> fan_in >> fan_out >> left >> right >> out >> func;
		_gates[gate_id].init(left, right, out, func);
		_wires[left]._enters_to.push_back(gate_id);
		_wires[right]._enters_to.push_back(gate_id);
		_wires[out]._out_from = gate_id;

		if(left < _num_input_wires && right < _num_input_wires)
			_ready_gates_set.insert(gate_id);
	}

//	for (gate_id_t gate_id = 1; gate_id <= _num_gates; gate_id++)
//	{
//		_gates[gate_id].print(gate_id);
//	}

	circuit_file.close();
}

//void BooleanCircuit::_parse_circuit(const char* desc_file)
//{
//	std::ifstream circuit_file(desc_file);
//	if(!circuit_file.good()) throw_bad_circuit_file();
//
//	circuit_file >> _num_gates;
//		_gates.resize(_num_gates+1); //+1 since gates are indexed from 1...
//	circuit_file >> _num_parties;
//		_parties.resize(_num_parties+1); //+1 since parties are indexed from 1...
//
//	int wire_id;
//	/* Parse circuit input wires */
//	for (int idx_party = 1; idx_party <= _num_parties; idx_party++)
//	{
//		unsigned int p_id, num_input_wires;
//		circuit_file >> p_id >> num_input_wires;
//		if ( p_id != idx_party )
//			throw_party_exists(p_id, circuit_file.tellg());
//		for (unsigned int i = 0; i < num_input_wires; i++)
//			circuit_file >> wire_id; //ignore this wire_id
//
//		_parties[p_id].init(_num_input_wires,num_input_wires); //note the difference between the two args
//		std::cout << "party " << p_id << ": " <<  _parties[p_id].wires << " " << _parties[p_id].n_wires << std::endl;
//		_num_input_wires += num_input_wires;
//	}
//
//	/* Parse output wires */
//	circuit_file >> _num_output_wires;
//	circuit_file >> _output_start;
//	std::cout << "# output wires: " << _num_output_wires << " starts at " << _output_start << std::endl;
//	int largest_output_wire;
//	for (unsigned int i = 1; i < _num_output_wires; i++) //NOTE: starts with i=1 !
//		circuit_file >> largest_output_wire; // ignore all but the last one
//	_wires.resize(_output_start, Wire(false));
//	_wires.resize(largest_output_wire+1, Wire(true));
////	for(int j=0; j<_wires.size(); j++) {
////			_wires[j].print(j);
////		}
//
//	/* Parse gates */
//	for (gate_id_t gate_id = 1; gate_id <= _num_gates; gate_id++)
//	{
//		int fan_in, fan_out, left, right, out;
//		std::string func;
//
//		circuit_file >> fan_in >> fan_out >> left >> right >> out >> func;
//		_gates[gate_id].init(left, right, out, func);
//		_wires[left]._enters_to.push_back(gate_id);
//		_wires[right]._enters_to.push_back(gate_id);
//		_wires[out]._out_from = gate_id;
//
//		if(left < _num_input_wires && right < _num_input_wires)
//			_ready_gates_set.insert(gate_id);
//	}
//
////	for (gate_id_t gate_id = 1; gate_id <= _num_gates; gate_id++)
////	{
////		_gates[gate_id].print(gate_id);
////	}
//
//	circuit_file.close();
//}

void BooleanCircuit::_make_layers()
{
	_max_layer_sz = 0;
	for (wire_id_t w=_output_start; w<_wires.size(); w++) {
		gate_id_t output_gate = _wires[w]._out_from;
//		printf("\nmaking layers from gate %u\n",output_gate);
		__make_layers(output_gate);
	}
	_validate_layers();
}

int BooleanCircuit::__make_layers(gate_id_t g)
{
//	printf("gate %u\n",g);
	if(_gates[g]._layer != NO_LAYER){
//		printf("already in layer %d\n", _gates[g]._layer);
		return _gates[g]._layer;
	}

	if (_gates[g]._left < _num_input_wires && _gates[g]._right < _num_input_wires) {
//		printf("it's in layer 0\n");
		_gates[g]._layer = 0;
		_add_to_layer(0, g);
		return 0;
	}
	int layer_left, layer_right;
	if(_gates[g]._left == 0)
		layer_left = -1;
	else
		layer_left = __make_layers(_wires[_gates[g]._left]._out_from);
	if(_gates[g]._right == 0)
		layer_right = -1;
	else
		layer_right = __make_layers(_wires[_gates[g]._right]._out_from);

	int max = layer_left>layer_right? layer_left : layer_right;
	int my_layer = 1 + max;
//	printf("gate %u: left=%d, right=%d, my=%d\n", g, layer_left, layer_right, my_layer);
	_gates[g]._layer = my_layer;
	_add_to_layer(my_layer, g);
	return my_layer;
}

void BooleanCircuit::_add_to_layer(int layer, gate_id_t g)
{
//	printf("adding %u to layer %d\n", g, layer);
	if (_layers.size()<(size_t)layer+1) {
//		printf("layer doesn't exist, creating it\n");
		_layers.resize(layer+1);
	}
	_layers[layer].push_back(g);
}

void BooleanCircuit::_print_layers()
{
	printf("num layers = %lu\n",_layers.size());
	for(size_t i=0; i<_layers.size(); i++) {
		printf ("\nlayer %lu size=%lu\n",i,_layers[i].size());
		for(size_t j=0; j<_layers[i].size(); j++) {
			printf("%lu  ", _layers[i][j]);
		}
		printf ("\n");
	}
}

void BooleanCircuit::_validate_layers()
{
	_max_layer_sz = 0;
	for (gate_id_t g=1; g<_num_gates; g++){
		if(_gates[g]._layer == NO_LAYER)
			printf("gate %lu has no layer!\n",g);
		assert(_gates[g]._layer != NO_LAYER);
	}
	gate_id_t sum_gates_in_layers = 0;
	for(size_t i=0; i<_layers.size(); i++) {
		sum_gates_in_layers += _layers[i].size();
		if(_layers[i].size() > _max_layer_sz) {
			_max_layer_sz = _layers[i].size();
		}
	}
	assert(sum_gates_in_layers == _num_gates);
}


///*  - if less inputs are given then it autocompletes to zeros
// *  - if more is given then it is shortened */
//void BooleanCircuit::RawInputs(std::string raw_inputs)
//{
//	raw_inputs.resize(_num_input_wires,'0');
//	std::cout << "Input:" << std::endl << raw_inputs << std::endl;
//	for (int i=0; i<_num_input_wires; i++)
//		_wires[i].Sig(raw_inputs[i]=='0'?0:1);
//
////	for(int j=0; j<_wires.size(); j++) {
////		_wires[j].print(j);
////	}
//}

void BooleanCircuit::Inputs(const char* inputs_file_path)
{
	std::ifstream inputs_file(inputs_file_path);
	std::string raw_inputs;
	getline (inputs_file, raw_inputs, (char) inputs_file.eof());
	inputs_file.close();

}

BooleanCircuit::BooleanCircuit(const char* desc_file)
:_num_evaluated_out_wires(0)
{
	_parse_circuit(desc_file);
	_make_layers();
//	_print_layers();
}


void BooleanCircuit::EvaluateByLayerLinearly(party_id_t my_id) {
	char* prf_output = new char[PAD_TO_8(_num_parties)*16];
#ifdef __PURE_SHE__
	mpz_t temp_mpz;
	init_temp_mpz_t(temp_mpz);
#endif
	for(size_t i=0; i<_layers.size(); i++) {
		for (size_t j=0; j<_layers[i].size(); j++) {
			gate_id_t gid = _layers[i][j];
#ifdef __PURE_SHE__
			_eval_gate(gid, my_id, prf_output, temp_mpz);
#else
			_eval_gate(gid, my_id, prf_output);
#endif
		}
	}
	delete[] prf_output;
}

void BooleanCircuit::EvaluateByLayer(int num_threads, party_id_t my_id)
{
	_evaluator_initiated = 0;
	boost::thread_group tg;
	for (int i=0; i<num_threads; i++) {
		boost::thread* evaluator = new boost::thread(&BooleanCircuit::_eval_by_layer, this,i, num_threads, my_id);
		tg.add_thread(evaluator);
	}
	tg.join_all();
}

void BooleanCircuit::_eval_by_layer(int i, int num_threads, party_id_t my_id)
{
	char* prf_output = new char[PAD_TO_8(_num_parties)*16];
#ifdef __PURE_SHE__
	mpz_t temp_mpz;
	init_temp_mpz_t(temp_mpz);
#endif
	for(size_t l=0; l<_layers.size(); l++) {
		int layer_sz = _layers[l].size();
		int start_idx = (layer_sz/num_threads)*i;
		int end_idx = (layer_sz/num_threads)*(i+1)-1;
		if (i==num_threads-1)
			end_idx = layer_sz-1;
//		printf("thread %d eval layer %d indices %d - %d\n", i, l, start_idx, end_idx);
		for(int g=start_idx; g<=end_idx; g++) {
			gate_id_t gid = _layers[l][g];
#ifdef __PURE_SHE__
			_eval_gate(gid, my_id, prf_output, temp_mpz);
#else
			_eval_gate(gid, my_id, prf_output);
#endif
		}
//		printf("done eval layer %d\n", l);

		if( i == num_threads-1) { // last thread is the coordinator
			_num_threads_finished++;
			while(true) {
				std::unique_lock<std::mutex> locker(_coordinator_mx);
				if(_num_threads_finished == num_threads)
					break;
				_coordinator_cv.wait(locker);
			}
//			printf("all threads are done for layer %d\n", l);
			_num_threads_finished = 0;
			std::unique_lock<std::mutex> locker(_layers_mx);
			_layers_cv.notify_all();
		} else { // everyone else
			std::unique_lock<std::mutex> locker(_layers_mx);
			_num_threads_finished++;
			{
				std::unique_lock<std::mutex> locker(_coordinator_mx);
				_coordinator_cv.notify_one();
			}
			_layers_cv.wait(locker);
		}
	}

}

//void BooleanCircuit::Evaluate(int num_threads, party_id_t my_id) {
//	boost::thread_group tg;
//	for (int i=0; i<num_threads; i++) {
//		boost::thread* evaluator = new boost::thread(&BooleanCircuit::_eval_thread, this, my_id);
//		tg.add_thread(evaluator);
//	}
//	tg.join_all();
//}

//void BooleanCircuit::_eval_thread(party_id_t my_id) {
//	char* prf_output = new char[PAD_TO_8(_num_parties)*16];
//	while (_num_evaluated_out_wires != _num_output_wires) {
//		_ready_mx.lock();
//		if (_ready_gates_set.empty()) {
//			_ready_mx.unlock();
//			continue;
//		}
//		gate_id_t g = *(_ready_gates_set.begin());
//		_ready_gates_set.erase(g);
//		_ready_mx.unlock();
//
//		signal_t s = _eval_gate(g, my_id, prf_output); //_num_eval_gates++;
//		Wire* out_wire = &_wires[_gates[g]._out];
//		if(out_wire->_is_output) {
//			_num_evaluated_out_wires++;
//		}
//
//		_ready_mx.lock();
//		_externals[_gates[g]._out] = s;
//		gate_id_t next_gate;
//		for(int i=0; i < out_wire->_enters_to.size(); i++) {
//			next_gate = out_wire->_enters_to[i];
//			if(is_gate_ready(next_gate)) {
//				_ready_gates_set.insert(next_gate);
//			}
//		}
//		_ready_mx.unlock();
//	}
//}

#ifdef __PURE_SHE__
void BooleanCircuit::_eval_gate(gate_id_t g, party_id_t my_id, char* prf_output, mpz_t& tmp_mpz)
#else
void BooleanCircuit::_eval_gate(gate_id_t g, party_id_t my_id, char* prf_output)
#endif
{
//	std::cout << std::endl << "evaluate gate " << g << std::endl;
	wire_id_t w_l = _gates[g]._left;
	wire_id_t w_r = _gates[g]._right;
	wire_id_t w_o = _gates[g]._out;
	party->registers[w_o].eval(party->registers[w_l], party->registers[w_r],
	        party->_garbled_tbl[g-1], my_id, prf_output, w_o, w_l, w_r);
}


//signal_t BooleanCircuit::_eval_gate(gate_id_t g, party_id_t my_id, char* dummy)
//{
////	std::cout << std::endl << "evaluate gate " << g << std::endl;
//	wire_id_t w_l = _gates[g]._left;
//	wire_id_t w_r = _gates[g]._right;
//	wire_id_t w_o = _gates[g]._out;
//	int sig_l = _externals[w_l];
//	int sig_r = _externals[w_r];
//	int entry = 2 * sig_l + sig_r;
//
//	Key* garbled_entry = _garbled_entry(g, entry);
//
////	printf("garbled entry %d\n",entry);
////	for(party_id_t i=1; i<=_num_parties; i++) {
////		std::cout << garbled_entry[i-1] << "  ";
////	}
////	std::cout << std::endl;
//
//	int ext_l = entry%2 ? 1 : 0 ;
//	int ext_r = entry<2 ? 0 : 1 ;
//
//	char prf_output[16];
//	Key k;
//	for(party_id_t i=1; i<=_num_parties; i++) {
//		for(party_id_t j=1; j<=_num_parties; j++) {
//			PRF_single(_key(i,w_l,sig_l), _input(ext_l,g,j) , prf_output);
//			k = *(Key*)prf_output;
////						std::cout << "using key: " << *_key(i,w_l,sig_l) << ": ";
////						printf("Fk^%d_{%u,%d}(%d,%u,%d) = ",i, w_l, sig_l,ext_l,g,j);
////						std::cout << k << std::endl;
//			garbled_entry[j-1] -= k;
//
//			PRF_single(_key(i,w_r,sig_r), _input(ext_r,g,j) , prf_output);
//			k = *(Key*)prf_output;
////						std::cout << "using key: " << *_key(i,w_r,sig_r) << ": ";
////						printf("Fk^%d_{%u,%d}(%d,%u,%d) = ",i, w_r, sig_r,ext_r,g,j);
////						std::cout << k << std::endl;
//			garbled_entry[j-1] -= k;
//		}
//	}
//
////	printf("garbled entry %d\n",entry);
////	for(party_id_t i=1; i<=_num_parties; i++) {
////		std::cout << garbled_entry[i-1] << "  ";
////	}
////	std::cout << std::endl;
//
//	if(garbled_entry[my_id-1] == *_key(my_id, w_o, 0)) {
////		std::cout << "k^"<<my_id<<"_{"<<w_o<<",0} = " << *_key(my_id, w_o, 0) << std::endl;
//		memcpy(_key(1,w_o,0), garbled_entry, sizeof(Key)*_num_parties);
//		return 0;
//	} else if (garbled_entry[my_id-1] == *_key(my_id, w_o, 1)) {
////		std::cout << "k^"<<my_id<<"_{"<<w_o<<",1} = " << *_key(my_id, w_o, 1) << std::endl;
//		memcpy(_key(1,w_o,1), garbled_entry, sizeof(Key)*_num_parties);
//		return 1;
//	} else {
//		printf("\nERROR!!!\n");
//		throw std::invalid_argument("result key doesn't fit any of my keys");
//	}
//
//}

//signal_t BooleanCircuit::_eval_gate(gate_id_t g)
//{
//	signal_t l = _wires[_gates[g]._left].Sig();
//	signal_t r = _wires[_gates[g]._right].Sig();
//	signal_t s = _gates[g]._func[2*l+r];
//	return s;
//}


std::string BooleanCircuit::Output()
{
	std::cout << "output:" <<std::endl;
	std::stringstream ss;
	printf("masks/externals\n");
	for( size_t i=0; i<_num_output_wires; i++ ) {
		cout << "mask " << i << ": " << (int)party->registers[_output_start+i].mask << endl;
		cout << "external " << i << ": " << (int)party->registers[_output_start+i].get_external() << endl;
		int output = party->registers[_output_start+i].get_output();
		ss << output;
	}
	std::cout << ss.str();
	std::cout<<std::endl;
	return ss.str();
}

