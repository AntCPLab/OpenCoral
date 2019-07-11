/*
 * EvalSecret.cpp
 *
 */

#include "Secret.h"
#include "Secret_inline.h"

namespace GC
{

template<class T>
Secret<T> Secret<T>::input(Processor<Secret<T>>& processor, const InputArgs& args)
{
    return T::get_input(processor, args);
}

template<class T>
Secret<T> Secret<T>::input(party_id_t from, const int128& input, int n_bits)
{
    Secret<T> res;
    if (n_bits < 0)
        n_bits = default_length;
#ifdef DEBUG_INPUT
    cout << "input " << input << endl;
#endif
    for (int i = 0; i < n_bits; i++)
    {
        res.get_new_reg().input(from, input.get_bit(i));
#ifdef DEBUG_INPUT
        cout << (int)input.get_bit(i);
#endif
    }
#ifdef DEBUG_INPUT
    cout << " input" << endl;
#endif
    if ((size_t)n_bits != res.registers.size())
    {
    	cout << n_bits << " " << res.registers.size() << endl;
    	throw runtime_error("wrong bit length in input()");
    }
#ifdef DEBUG_INPUT
	for (auto& reg : res.registers)
	    cout << (int)reg.get_mask_no_check();
	cout << " mask " <<  endl;
	for (auto& reg : res.registers)
	    cout << (int)reg.get_external_no_check();
	cout << " ext " << endl;
    int128 a;
    res.reveal(a);
	cout << " input " << hex << a << "(" << res.size() << ") from " << from
			<< " (" << input << ", " << dec << n_bits << ")" << endl;
#endif
    return res;
}

template<class T>
void Secret<T>::random(int n_bits, int128 share)
{
    (void)share;
    if (n_bits > 128)
        throw not_implemented();
#ifdef NO_INPUT
    resize_regs(n_bits);
    for (int i = 0; i < n_bits; i++)
    	get_reg(i).random();
#else
    for (int i = 0; i < CommonParty::singleton->get_n_parties(); i++)
    {
    	Secret<T> tmp = *this;
    	Secret<T> s = input(i + 1, share, n_bits);
    	*this = tmp + s;
#ifdef DEBUG_DYNAMIC
    	int128 a,b,c;
    	tmp.reveal(a);
    	s.reveal(b);
    	reveal(c);
    	cout << c << " = " << a << " ^ " << b << " (" << dec << n_bits << ", " << share << ")" << endl;
#endif
    }
#endif
#ifdef DEBUG_RANDOM
    int128 revealed;
    reveal(revealed);
    cout << "random " << revealed << " share " << share << endl;
#endif
    if ((size_t)n_bits != registers.size())
    {
    	cout << n_bits << " " << registers.size() << endl;
    	throw runtime_error("wrong bit length in random()");
    }
}

template <class T>
void Secret<T>::random_bit()
{
#ifdef NO_INPUT
	return random(1, 0);
#else
    return random(1, CommonParty::s().prng.get_uchar() & 1);
#endif
}

template <class T>
template <class U>
void Secret<T>::store(U& mem,
        vector<WriteAccess<Secret<T> > >& accesses)
{
    T::store(mem, accesses);
}

template <class T>
void Secret<T>::output(T& reg)
{
    cast(reg).output();
}

template <class T>
Secret<T> Secret<T>::carryless_mult(const Secret<T>& x, const Secret<T>& y)
{
    Secret<T> res;
    if (x.registers.size() == 0)
        throw not_implemented();
#ifdef DEBUG_DYNAMIC2
    for (int i = 0; i < x.registers.size(); i++)
        output(x.registers[i]);
    for (int i = 0; i < y.registers.size(); i++)
        output(y.registers[i]);
#endif
    for (size_t i = 0; i < x.registers.size() + y.registers.size() - 1; i++)
    {
        int start = max((size_t)0, i - y.registers.size() + 1);
        int stop = min(i + 1, x.registers.size());
        T sum = AND<T>(x.get_reg(start), y.get_reg(i - start));
#ifdef DEBUG_DYNAMIC2
        output(sum);
        cout << "carryless " << i << " " << start << " " << i - start <<
                " sum "  << (int)cast(sum).get_output() <<
                " x " << (int)x.get_reg(start).get_output() <<
                " y " << (int)y.get_reg(i - start).get_output() <<
                " sum id " << sum.get_reg().get_id() << endl;
#endif
        for (int j = start + 1; j < stop; j++)
        {
            T product = AND<T>(x.get_reg(j), y.get_reg(i - j));
            sum = XOR<T>(sum, product);
#ifdef DEBUG_DYNAMIC2
            cout << "carryless " <<
                    " prod id " << product.get_reg().get_id() <<
                    " sum id " << sum.get_reg().get_id() << endl << flush;
            output(product);
            output(sum);
            cout << "carryless " << i << " " << j << " " << i - j <<
                    " prod " << (int)cast(product).get_output() <<
                    " sum "  << (int)cast(sum).get_output() <<
                    " x " << (int)x.get_reg(j).get_output() <<
                    " y " << (int)y.get_reg(i - j).get_output() << endl;
#endif
        }
        res.registers.push_back(sum);
    }
#ifdef DEBUG_DYNAMIC
    word a, b;
    int128 c;
    x.reveal(a);
    y.reveal(b);
    res.reveal(c);
    cout << typeid(T).name() << endl;
    cout << c << " = " << hex << a << " * " << b << endl;
    AuthValue d;
    d.assign(a, b, false);
    if (d.mac != c)
    	throw runtime_error("carryless mult");
#endif
    return res;
}

template <class T>
Secret<T>::Secret()
{

}


template<class T>
T& GC::Secret<T>::get_new_reg()
{
	registers.push_back(T::new_reg());
	T &res = cast(registers.back());
#ifdef DEBUG_REGS
	cout << "Secret: new " << typeid(T).name() << " " << res.get_id() << " at " << &res << endl;
#endif
	return res;
}

template <class T>
void Secret<T>::load(int n, const Integer& x)
{
    if ((unsigned)n < 8 * sizeof(x) and abs(x.get()) > (1LL << n))
        throw out_of_range("public value too long");
#ifdef DEBUG_ROUNDS2
    cout << "secret from integer " << hex << this << dec << " " << endl;
#endif
    resize_regs(n);
    for (int i = 0; i < n; i++)
    {
        get_reg(i).public_input((x.get() >> i) & 1);
    }
#ifdef DEBUG_VALUES
    cout << "input " << x << endl;
    for (int i = 0; i < n; i++)
        cout << ((x.get() >> i) & 1);
    cout << endl;
    cout << "on registers: " << endl;
    for (int i = 0; i < n; i++)
        cout << get_reg(i).get_id() << " ";
    cout << endl;
#endif
}

template <class T>
template <class U>
void Secret<T>::load(vector<ReadAccess < Secret<T> > >& accesses, const U& mem)
{
    for (auto&& access : accesses)
    {
        int n = access.length;
        if (n <= 0 || n > default_length)
            throw runtime_error("invalid length for dynamic loading");
        access.dest.resize_regs(n);
    }
    T::load(accesses, mem);
}

template <class T>
Secret<T> Secret<T>::operator<<(int i)
{
    Secret<T> res;
    for (int j = 0; j < i; j++)
        res.get_new_reg().public_input(0);
    res.registers.insert(res.registers.end(), registers.begin(),
            registers.end());
    return res;
}

template <class T>
Secret<T> Secret<T>::operator>>(int i)
{
    Secret<T> res;
    res.registers.insert(res.registers.end(), registers.begin() + i, registers.end());
    return res;
}

template <class T>
void Secret<T>::bitcom(Memory<Secret>& S, const vector<int>& regs)
{
    registers.clear();
    for (unsigned int i = 0; i < regs.size(); i++)
    {
    	if (S[regs[i]].registers.size() != 1)
    		throw Processor_Error("can only compose bits");
        registers.push_back(S[regs[i]].registers.at(0));
    }
}

template <class T>
void Secret<T>::bitdec(Memory<Secret>& S, const vector<int>& regs) const
{
    if (regs.size() > registers.size())
        throw out_of_range(
                "not enough bits for bit decomposition: "
                        + to_string(regs.size()) + " > "
                        + to_string(registers.size()));
    for (unsigned int i = 0; i < regs.size(); i++)
    {
        Secret& secret = S[regs[i]];
        secret.registers.clear();
        secret.registers.push_back(registers.at(i));
    }
}

template<class T>
void Secret<T>::trans(Processor<Secret<T> >& processor, int n_outputs,
        const vector<int>& args)
{
    int n_inputs = args.size() - n_outputs;
    for (int i = 0; i < n_outputs; i++)
    {
        processor.S[args[i]].resize_regs(n_inputs);
        for (int j = 0; j < n_inputs; j++)
            processor.S[args[i]].registers[j] =
                    processor.S[args[n_outputs + j]].registers[i];
    }
}

template <class T>
template <class U>
void Secret<T>::reveal(size_t n_bits, U& x)
{
#ifdef DEBUG_OUTPUT
    cout << "revealing " << this << " with min(" << 8 * sizeof(U) << ","
            << registers.size() << ") bits" << endl;
#endif
    if (n_bits > registers.size())
        throw out_of_range("not enough wires for revealing");
    x = 0;
    for (unsigned int i = 0; i < min(8 * sizeof(U), registers.size()); i++)
    {
        get_reg(i).output();
        char out = get_reg(i).get_output();
        x ^= U(out) << i;
#ifdef DEBUG_OUTPUT
        cout << (int)out;
#endif
    }
#ifdef DEBUG_OUTPUT
    cout << " output" << endl;
    for (auto& reg : registers)
        cout << (int)reg.get_mask_no_check();
    cout << " mask" << endl;
    for (auto& reg: registers)
        cout << (int)reg.get_external_no_check();
    cout << " ext" << endl;
#endif
#ifdef DEBUG_VALUES
    cout << typeid(T).name() << " " << &x << endl;
    cout << "reveal " << registers.size() << " bits: " << hex << showbase << x << dec << endl;
    cout << "from registers:" << endl;
    for (unsigned int i = 0; i < registers.size(); i++)
        cout << registers[i].get_id() << " ";
    cout << endl;
#endif
}

} /* namespace GC */
