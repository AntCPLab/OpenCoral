/*
 * YaoWire.cpp
 *
 */

#include "YaoGarbleWire.h"
#include "YaoGate.h"
#include "YaoGarbler.h"
#include "GC/ArgTuples.h"

#include "GC/Processor.hpp"
#include "GC/Secret.hpp"
#include "GC/Thread.hpp"

void YaoGarbleWire::random()
{
	mask = YaoGarbler::s().prng.get_bit();
	key = 0;
}

void YaoGarbleWire::public_input(bool value)
{
	mask = value;
	key = 0;
}

void YaoGarbleWire::and_(GC::Processor<GC::Secret<YaoGarbleWire> >& processor,
		const vector<int>& args, bool repeat)
{
#ifdef YAO_TIMINGS
	auto& garbler = YaoGarbler::s();
	TimeScope ts(garbler.and_timer), ts2(garbler.and_proc_timer),
			ts3(garbler.and_main_thread_timer);
#endif
	and_multithread(processor, args, repeat);
}

void YaoGarbleWire::and_multithread(GC::Processor<GC::Secret<YaoGarbleWire> >& processor,
		const vector<int>& args, bool repeat)
{
	YaoGarbler& party = YaoGarbler::s();
	int total = processor.check_args(args, 4);
	if (total < party.get_threshold())
	{
		// run in single thread
		and_singlethread(processor, args, repeat);
		return;
	}

	party.and_prepare_timer.start();
	processor.complexity += total;
	SendBuffer& gates = party.gates;
	gates.allocate(total * sizeof(YaoGate));
	int max_gates_per_thread = max(party.get_threshold() / 2,
			(total + party.get_n_worker_threads() - 1) / party.get_n_worker_threads());
	int i_thread = 0, i_gate = 0, start = 0;
	for (size_t j = 0; j < args.size(); j += 4)
	{
		i_gate += args[j];
		size_t end = j + 4;
		if (i_gate >= max_gates_per_thread or end >= args.size())
		{
			YaoGate* gate = (YaoGate*)gates.end();
			gates.skip(i_gate * sizeof(YaoGate));
			party.timers["Dispatch"].start();
			party.and_jobs[i_thread++]->dispatch(processor.S, args, start, end,
					i_gate, gate, party.get_gate_id(), repeat);
			party.timers["Dispatch"].stop();
			party.counter += i_gate;
			i_gate = 0;
			start = end;
		}
	}
	party.and_prepare_timer.stop();
	party.and_wait_timer.start();
	for (int i = 0; i < i_thread; i++)
		party.and_jobs[i]->worker.done();
	party.and_wait_timer.stop();
}

void YaoGarbleWire::and_singlethread(GC::Processor<GC::Secret<YaoGarbleWire> >& processor,
		const vector<int>& args, bool repeat)
{
	int total_ands = processor.check_args(args, 4);
	if (total_ands < 10)
		return processor.andrs(args);
	processor.complexity += total_ands;
	size_t n_args = args.size();
	auto& garbler = YaoGarbler::s();
	SendBuffer& gates = garbler.gates;
	YaoGate* gate = (YaoGate*)gates.allocate_and_skip(total_ands * sizeof(YaoGate));
	long counter = garbler.get_gate_id();
	and_(processor.S, args, 0, n_args, total_ands, gate, counter,
			garbler.prng, garbler.timers, repeat, garbler);
	garbler.counter += counter - garbler.get_gate_id();
}

void YaoGarbleWire::and_(GC::Memory<GC::Secret<YaoGarbleWire> >& S,
		const vector<int>& args, size_t start, size_t end, size_t total_ands,
		YaoGate* gate, long& counter, PRNG& prng, map<string, Timer>& timers,
		bool repeat, YaoGarbler& garbler)
{
	(void)timers;
	Key* labels;
	Key* hashes;
	vector<Key> label_vec, hash_vec;
	size_t n_hashes = 4 * total_ands;
	Key label_arr[400], hash_arr[400];
	if (total_ands < 100)
	{
		labels = label_arr;
		hashes = hash_arr;
	}
	else
	{
		label_vec.resize(n_hashes);
		hash_vec.resize(n_hashes);
		labels = label_vec.data();
		hashes = hash_vec.data();
	}
	//timers["Hash input"].start();
	const Key& delta = garbler.get_delta();
	size_t i_label = 0;
	int dl = GC::Secret<YaoGarbleWire>::default_length;
	Key left_delta = delta.doubling(1);
	Key right_delta = delta.doubling(2);
	for (auto it = args.begin() + start; it < args.begin() + end; it += 4)
	{
		if (*it == 1)
		{
			counter++;
			YaoGate::E_inputs(&labels[i_label], S[*(it + 2)].get_reg(0).key,
					S[*(it + 3)].get_reg(0).key, left_delta, right_delta,
					counter);
			i_label += 4;
		}
		else
		{
			int n_units = DIV_CEIL(*it, dl);
			for (int j = 0; j < n_units; j++)
			{
				int left = min(dl, *it - j * dl);
				for (int k = 0; k < left; k++)
				{
					auto& left_wire = S[*(it + 2) + j].get_reg(k);
					const Key& right_key = S[*(it + 3) + j].get_reg(
							repeat ? 0 : k).key;
					counter++;
					YaoGate::E_inputs(&labels[i_label], left_wire.key,
							right_key, left_delta, right_delta, counter);
					i_label += 4;
				}
			}
		}
	}
	//timers["Hash input"].stop();
	//timers["Hashing"].start();
	MMO& mmo = garbler.mmo;
	size_t i;
	for (i = 0; i + 8 <= n_hashes; i += 8)
		mmo.hash<8>(&hashes[i], &labels[i]);
	for (; i < n_hashes; i++)
		hashes[i] = mmo.hash(labels[i]);
	//timers["Hashing"].stop();
	//timers["Garbling"].start();
	size_t i_hash = 0;
	for (auto it = args.begin() + start; it < args.begin() + end; it += 4)
	{
		if (*it == 1)
		{
			auto& out = S[*(it + 1)];
			out.resize_regs(1);
			out.get_reg(0).randomize(prng);
			(gate++)->and_garble(out.get_reg(0), &hashes[i_hash],
					S[*(it + 2)].get_reg(0).mask,
					S[*(it + 3)].get_reg(0).mask, garbler.get_delta());
			//timers["Gate computation"].stop();
			i_hash += 4;
		}
		else
		{
			int n_units = DIV_CEIL(*it, dl);
			for (int j = 0; j < n_units; j++)
			{
				//timers["Outer ref"].start();
				auto& out = S[*(it + 1) + j];
				//timers["Outer ref"].stop();
				//timers["Resizing"].start();
				int n = min(dl, *it - j * dl);
				out.resize_regs(n);
				//timers["Resizing"].stop();
				for (int k = 0; k < n; k++)
				{
					YaoGarbleWire& right_wire =
							S[*(it + 3) + (repeat ? 0 : j)].get_reg(
									repeat ? 0 : k);
					//timers["Inner ref"].start();
					auto& left_wire = S[*(it + 2) + j].get_reg(k);
					//timers["Inner ref"].stop();
					//timers["Randomizing"].start();
					out.get_reg(k).randomize(prng);
					//timers["Randomizing"].stop();
					//timers["Gate computation"].start();
					(gate++)->and_garble(out.get_reg(k), &hashes[i_hash],
							left_wire.mask, right_wire.mask,
							garbler.get_delta());
					//timers["Gate computation"].stop();
					i_hash += 4;
				}
			}
		}
	}
	//timers["Garbling"].stop();
}


void YaoGarbleWire::inputb(GC::Processor<GC::Secret<YaoGarbleWire>>& processor,
        const vector<int>& args)
{
	InputArgList a(args);
	int n_evaluator_bits = 0;
	auto& garbler = YaoGarbler::s();
	bool interactive = garbler.n_interactive_inputs_from_me(a) > 0;
	for (auto x : a)
	{
		auto& dest = processor.S[x.dest];
		dest.resize_regs(x.n_bits);
		if (x.from == 0)
		{
			long long input = processor.get_input(x.params, interactive);
			for (auto& reg : dest.get_regs())
			{
				reg.public_input(input & 1);
				input >>= 1;
			}
		}
		else
		{
			n_evaluator_bits += x.n_bits;
		}
	}

	if (interactive)
	    cout << "Thank you";

	garbler.receiver_input_keys.push_back({});

	for (auto x : a)
	{
		if (x.from == 1)
		{
			for (auto& reg : processor.S[x.dest].get_regs())
			{
				reg.set(garbler.prng.get_doubleword(), 0);
				garbler.receiver_input_keys.back().push_back(reg.key);
			}
		}
	}
}

inline void YaoGarbler::store_gate(const YaoGate& gate)
{
	gates.serialize(gate);
}

void YaoGarbleWire::op(const YaoGarbleWire& left, const YaoGarbleWire& right,
		Function func)
{
	auto& garbler = YaoGarbler::s();
	randomize(garbler.prng);
	YaoGarbler::s().counter++;
	YaoGate gate(*this, left, right, func);
	YaoGarbler::s().store_gate(gate);
}

char YaoGarbleWire::get_output()
{
	YaoGarbler::s().taint();
	YaoGarbler::s().output_masks.push_back(mask);
	return 0;
}

void YaoGarbleWire::convcbit(Integer& dest, const GC::Clear& source,
		GC::Processor<GC::Secret<YaoGarbleWire>>&)
{
	(void) source;
	auto& garbler = YaoGarbler::s();
	garbler.untaint();
	dest = garbler.P->receive_long(1);
}
