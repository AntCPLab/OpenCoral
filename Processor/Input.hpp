/*
 * Input.cpp
 *
 */

#include "Input.h"
#include "Processor.h"
#include "Auth/MAC_Check.h"

template<class T>
InputBase<T>::InputBase(ArithmeticProcessor* proc) :
        values_input(0)
{
    if (proc)
        buffer.setup(&proc->private_input, -1, proc->private_input_filename);
}

template<class T>
Input<T>::Input(SubProcessor<Share<T>>& proc, MAC_Check<T>& mc) :
        InputBase<Share<T>>(&proc.Proc), proc(proc), MC(mc), shares(proc.P.num_players())
{
}

template<class T>
InputBase<T>::~InputBase()
{
    if (timer.elapsed() > 0)
        cerr << T::type_string() << " inputs: " << timer.elapsed() << endl;
}

template<class T>
void Input<T>::adjust_mac(Share<T>& share, T& value)
{
    T tmp;
    tmp.mul(MC.get_alphai(), value);
    tmp.add(share.get_mac(),tmp);
    share.set_mac(tmp);
}

template<class T>
void Input<T>::reset(int player)
{
    shares[player].clear();
    if (player == proc.P.my_num())
        o.reset_write_head();
}

template<class T>
void Input<T>::add_mine(const T& input)
{
    int player = proc.P.my_num();
    T rr, t = input;
    shares[player].push_back({});
    Share<T>& share = shares[player].back();
    proc.DataF.get_input(share, rr, player);
    T xi;
    t.sub(t, rr);
    t.pack(o);
    xi.add(t, share.get_share());
    share.set_share(xi);
    adjust_mac(share, t);
    this->values_input++;
}

template<class T>
void Input<T>::add_other(int player)
{
    T t;
    shares[player].push_back({});
    proc.DataF.get_input(shares[player].back(), t, player);
}

template<class T>
void Input<T>::send_mine()
{
    proc.P.send_all(o, true);
}

template<class T>
void Input<T>::start(int player, int n_inputs)
{
    reset(player);
    if (player == proc.P.my_num())
    {
        for (int i = 0; i < n_inputs; i++)
        {
            T t;
            try
            {
                this->buffer.input(t);
            }
            catch (not_enough_to_buffer& e)
            {
                throw runtime_error("Insufficient input data to buffer");
            }
            add_mine(t);
        }
        send_mine();
    }
    else
    {
        for (int i = 0; i < n_inputs; i++)
            add_other(player);
    }
}

template<class T>
void Input<T>::stop(int player, const vector<int>& targets)
{
    for (unsigned int i = 0; i < targets.size(); i++)
        proc.get_S_ref(targets[i]) = shares[player][i];

    if (proc.P.my_num() != player)
    {
        T t;
        octetStream o;
        this->timer.start();
        proc.P.receive_player(player, o, true);
        this->timer.stop();
        for (unsigned int i = 0; i < targets.size(); i++)
        {
            Share<T>& share = proc.get_S_ref(targets[i]);
            t.unpack(o);
            adjust_mac(share, t);
        }
    }
}

template<class T>
void InputBase<T>::input(SubProcessor<T>& Proc,
        const vector<int>& args)
{
    auto& input = Proc.input;
    for (int i = 0; i < Proc.P.num_players(); i++)
        input.reset(i);
    assert(args.size() % 2 == 0);

    int n_from_me = 0;

    if (Proc.Proc.opts.interactive and Proc.Proc.thread_num == 0)
    {
        for (size_t i = 1; i < args.size(); i += 2)
            n_from_me += (args[i] == Proc.P.my_num());
        if (n_from_me > 0)
            cout << "Please input " << n_from_me << " numbers:" << endl;
    }

    for (size_t i = 0; i < args.size(); i += 2)
    {
        int n = args[i + 1];
        if (n == Proc.P.my_num())
        {
            long x = Proc.Proc.get_input(n_from_me > 0);
            input.add_mine(x);
        }
        else
        {
            input.add_other(n);
        }
    }

    if (n_from_me > 0)
        cout << "Thank you" << endl;

    input.send_mine();

    vector<vector<int>> regs(Proc.P.num_players());
    for (size_t i = 0; i < args.size(); i += 2)
    {
        regs[args[i + 1]].push_back(args[i]);
    }
    for (int i = 0; i < Proc.P.num_players(); i++)
        input.stop(i, regs[i]);
}
