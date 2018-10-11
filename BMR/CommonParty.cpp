/*
 * CommonParty.cpp
 *
 */

#include "CommonParty.h"
#include "BooleanCircuit.h"
#include "Tools/benchmarking.h"

CommonParty* CommonParty::singleton = 0;

CommonParty::CommonParty() :
		_node(0), gate_counter(0), gate_counter2(0), garbled_tbl_size(0),
		cpu_timer(CLOCK_PROCESS_CPUTIME_ID), buffers(TYPE_MAX)
{
	insecure("MPC emulation");
	if (singleton != 0)
		throw runtime_error("there can only be one");
	singleton = this;
	prng.ReSeed();
#ifdef DEBUG_PRNG
	octet seed[SEED_SIZE];
	memset(seed, 0, sizeof(seed));
	prng.SetSeed(seed);
#endif
	cpu_timer.start();
	timer.start();
	gf2n::init_field(128);
	mac_key.randomize(prng);
}

CommonParty::~CommonParty()
{
	if (_node)
		delete _node;
	cout << "Wire storage: " << 1e-9 * wires.capacity() << " GB" << endl;
	cout << "CPU time: " << cpu_timer.elapsed() << endl;
	cout << "Total time: " << timer.elapsed() << endl;
	cout << "First phase time: " << timers[0].elapsed() << endl;
	cout << "Second phase time: " << timers[1].elapsed() << endl;
	cout << "Number of gates: " << gate_counter << endl;
}

void CommonParty::init(const char* netmap_file, int id, int n_parties)
{
#ifdef N_PARTIES
	if (n_parties != N_PARTIES)
	    throw runtime_error("wrong number of parties");
#else
#ifdef MAX_N_PARTIES
	if (n_parties > MAX_N_PARTIES)
	    throw runtime_error("too many parties");
#endif
	_N = n_parties;
#endif // N_PARTIES
	printf("netmap_file: %s\n", netmap_file);
	if (0 == strcmp(netmap_file, LOOPBACK_STR)) {
		_node = new Node( NULL, id, this, _N + 1);
	} else {
		_node = new Node(netmap_file, id, this);
	}
}

int CommonParty::init(const char* netmap_file, int id)
{
	int n_parties;
	if (string(netmap_file) != string(LOOPBACK_STR))
	{
		ifstream(netmap_file) >> n_parties;
		n_parties--;
	}
	else
		n_parties = 2;
	init(netmap_file, id, n_parties);
	return n_parties;
}

void CommonParty::reset()
{
	garbled_tbl_size = 0;
}

gate_id_t CommonParty::new_gate()
{
    gate_counter++;
    garbled_tbl_size++;
    return gate_counter;
}

void CommonParty::next_gate(GarbledGate& gate)
{
    gate_counter2++;
    gate.init_inputs(gate_counter2, _N);
}

SendBuffer& CommonParty::get_buffer(MSG_TYPE type)
{
	SendBuffer& buffer = buffers[type];
	buffer.clear();
	fill_message_type(buffer, type);
#ifdef DEBUG_BUFFER
	cout << type << " buffer:";
	phex(buffers.data(), 4);
#endif
	return buffer;
}

void CommonCircuitParty::print_masks(const vector<int>& indices)
{
	vector<char> bits;
	for (auto i = indices.begin(); i != indices.end(); i++)
		bits.push_back(registers[*i].get_mask_no_check());
	print_bit_array(bits);
}

void CommonCircuitParty::print_outputs(const vector<int>& indices)
{
	vector<char> bits;
	for (auto i = indices.begin(); i != indices.end(); i++)
		bits.push_back(registers[*i].get_output_no_check());
	print_bit_array(bits);
}


template <class T, class U>
GC::BreakType CommonParty::first_phase(GC::Program<U>& program,
		GC::Processor<T>& processor, GC::Machine<T>& machine)
{
	(void)machine;
	timers[0].start();
	reset();
	wires.clear();
	GC::BreakType next = (reinterpret_cast<GC::Program<T>*>(&program))->execute(processor);
#ifdef DEBUG_ROUNDS
	cout << "finished first phase at pc " << processor.PC
			<< " reason " << next << endl;
#endif
	timers[0].stop();
	cout << "First round time: " << timers[0].elapsed() << " / "
			<< timer.elapsed() << endl;
#ifdef DEBUG_WIRES
	cout << "Storing wires with " << 1e-9 * wires.size() << " GB on disk" << endl;
#endif
	wire_storage.push(wires);
	return next;
}

template<class T>
GC::BreakType CommonParty::second_phase(GC::Program<T>& program,
		GC::Processor<T>& processor, GC::Machine<T>& machine)
{
    (void)machine;
    wire_storage.pop(wires);
    wires.reset_head();
	timers[1].start();
	GC::BreakType next = GC::TIME_BREAK;
	next = program.execute(processor);
#ifdef DEBUG_ROUNDS
	cout << "finished second phase at " << processor.PC
			<< " reason " << next << endl;
#endif
	timers[1].stop();
//	cout << "Second round time: " << timers[1].elapsed() << ", ";
//	cout << "total time: " << timer.elapsed() << endl;
	if (false)
		return GC::CAP_BREAK;
	else
		return next;
}

void CommonCircuitParty::prepare_input_regs(party_id_t from)
{
    party_t sender = _circuit->_parties[from];
    wire_id_t s = sender.wires; //the beginning of my wires
    wire_id_t n = sender.n_wires; // number of my wires
    input_regs_queue.clear();
    input_regs_queue.push_back(_N + 1);
    (*input_regs)[from].clear();
    for (wire_id_t i = 0; i < n; i++) {
        wire_id_t w = s + i;
        (*input_regs)[from].push_back(w);
    }
}

void CommonCircuitParty::prepare_output_regs()
{
    output_regs.clear();
    for (size_t i = 0; i < _OW; i++)
        output_regs.push_back(_circuit->OutWiresStart()+i);
}

template GC::BreakType CommonParty::first_phase(
		GC::Program<GC::Secret<GarbleRegister> >& program,
		GC::Processor<GC::Secret<RandomRegister> >& processor,
		GC::Machine<GC::Secret<RandomRegister> >& machine);

template GC::BreakType CommonParty::first_phase(
		GC::Program<GC::Secret<EvalRegister> >& program,
		GC::Processor<GC::Secret<PRFRegister> >& processor,
		GC::Machine<GC::Secret<PRFRegister> >& machine);

template GC::BreakType CommonParty::second_phase(
		GC::Program<GC::Secret<GarbleRegister> >& program,
		GC::Processor<GC::Secret<GarbleRegister> >& processor,
		GC::Machine<GC::Secret<GarbleRegister> >& machine);

template GC::BreakType CommonParty::second_phase(
		GC::Program<GC::Secret<EvalRegister> >& program,
		GC::Processor<GC::Secret<EvalRegister> >& processor,
		GC::Machine<GC::Secret<EvalRegister> >& machine);
