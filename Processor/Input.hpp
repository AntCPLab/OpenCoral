/*
 * Input.cpp
 *
 */

#include "Input.h"
#include "Processor.h"

template<class T>
InputBase<T>::InputBase(ArithmeticProcessor* proc) :
        P(0), values_input(0)
{
    if (proc)
        buffer.setup(&proc->private_input, -1, proc->private_input_filename);
}

template<class T>
Input<T>::Input(SubProcessor<T>& proc, MAC_Check& mc) :
        InputBase<T>(&proc.Proc), proc(proc), MC(mc),
        shares(proc.P.num_players())
{
}

template<class T>
Input<T>::Input(SubProcessor<T>* proc, Player& P) :
        InputBase<T>(&proc->Proc), proc(*proc), MC(proc->MC), shares(
                P.num_players())
{
    assert (proc != 0);
}

template<class T>
InputBase<T>::~InputBase()
{
#ifdef VERBOSE
    if (timer.elapsed() > 0)
        cerr << T::type_string() << " inputs: " << timer.elapsed() << endl;
#endif
}

template<class T>
void Input<T>::reset(int player)
{
    InputBase<T>::reset(player);
    shares[player].clear();
}

template<class T>
void InputBase<T>::reset(int player)
{
    os.resize(max(os.size(), player + 1UL));
    os[player].reset_write_head();
}

template<class T>
void InputBase<T>::reset_all(Player& P)
{
    this->P = &P;
    os.resize(P.num_players());
    for (int i = 0; i < P.num_players(); i++)
        reset(i);
}

template<class T>
void Input<T>::add_mine(const open_type& input)
{
    int player = proc.P.my_num();
    shares[player].push_back({});
    T& share = shares[player].back();
    proc.DataF.get_input(share, rr, player);
    t.sub(input, rr);
    t.pack(this->os[player]);
    share += T::constant(t, 0, MC.get_alphai());
    this->values_input++;
}

template<class T>
void Input<T>::add_other(int player)
{
    open_type t;
    shares[player].push_back({});
    proc.DataF.get_input(shares[player].back(), t, player);
}

template<class T>
void InputBase<T>::add_from_all(const clear& input)
{
    for (int i = 0; i < P->num_players(); i++)
        if (i == P->my_num())
            add_mine(input);
        else
            add_other(i);
}

template<class T>
void Input<T>::send_mine()
{
    proc.P.send_all(this->os[proc.P.my_num()], true);
}

template<class T>
void InputBase<T>::exchange()
{
    for (int i = 0; i < P->num_players(); i++)
        if (i == P->my_num())
            send_mine();
        else
            P->receive_player(i, os[i], true);
}

template<class T>
void Input<T>::start(int player, int n_inputs)
{
    reset(player);
    if (player == proc.P.my_num())
    {
        for (int i = 0; i < n_inputs; i++)
        {
            clear t;
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
    if (proc.P.my_num() == player)
        for (unsigned int i = 0; i < targets.size(); i++)
            proc.get_S_ref(targets[i]) = finalize_mine();
    else
    {
        octetStream o;
        this->timer.start();
        proc.P.receive_player(player, o, true);
        this->timer.stop();
        for (unsigned int i = 0; i < targets.size(); i++)
        {
            finalize_other(player, proc.get_S_ref(targets[i]), o);
        }
    }
}

template<class T>
T Input<T>::finalize_mine()
{
    return shares[proc.P.my_num()].next();
}

template<class T>
void Input<T>::finalize_other(int player, T& target,
        octetStream& o)
{
    target = shares[player].next();
    t.unpack(o);
    target += T::constant(t, 1, MC.get_alphai());
}

template<class T>
T InputBase<T>::finalize(int player)
{
    if (player == P->my_num())
        return finalize_mine();
    else
    {
        T res;
        finalize_other(player, res, os[player]);
        return res;
    }
}

template<class T>
template<class U>
void InputBase<T>::input(SubProcessor<T>& Proc,
        const vector<int>& args, int size)
{
    auto& input = Proc.input;
    input.reset_all(Proc.P);
    int n_arg_tuple = U::N_DEST + U::N_PARAM + 1;
    assert(args.size() % n_arg_tuple == 0);

    int n_from_me = 0;

    if (Proc.Proc.opts.interactive and Proc.Proc.thread_num == 0)
    {
        for (size_t i = n_arg_tuple - 1; i < args.size(); i += n_arg_tuple)
            n_from_me += (args[i] == Proc.P.my_num()) * size;
        if (n_from_me > 0)
            cout << "Please input " << n_from_me << " " << U::NAME << "(s):" << endl;
    }

    for (size_t i = U::N_DEST; i < args.size(); i += n_arg_tuple)
    {
        int n = args[i + U::N_PARAM];
        if (n == Proc.P.my_num())
        {
            for (int j = 0; j < size; j++)
            {
                U tuple = Proc.Proc.template get_input<U>(n_from_me > 0,
                        &args[i]);
                for (auto x : tuple.items)
                    input.add_mine(x);
            }
        }
        else
        {
            for (int j = 0; j < U::N_DEST * size; j++)
                input.add_other(n);
        }
    }

    if (n_from_me > 0)
        cout << "Thank you" << endl;

    input.exchange();

    for (size_t i = 0; i < args.size(); i += n_arg_tuple)
    {
        int player = args[i + n_arg_tuple - 1];
        for (int k = 0; k < size; k++)
            for (int j = 0; j < U::N_DEST; j++)
                Proc.get_S_ref(args[i + j] + k) = input.finalize(player);
    }
}
