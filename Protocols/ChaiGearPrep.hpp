/*
 * ChaiGearPrep.cpp
 *
 */

#include "ChaiGearPrep.h"
#include "CowGearOptions.h"

#include "FHEOffline/SimpleGenerator.h"
#include "FHEOffline/SimpleMachine.h"
#include "FHEOffline/Producer.h"

template<class T>
MultiplicativeMachine* ChaiGearPrep<T>::machine = 0;
template<class T>
Lock ChaiGearPrep<T>::lock;

template<class T>
ChaiGearPrep<T>::~ChaiGearPrep()
{
    if (generator)
        delete generator;
    if (square_producer)
        delete square_producer;
    if (input_producer)
        delete input_producer;
}

template<class T>
void ChaiGearPrep<T>::teardown()
{
    if (machine)
        delete machine;
}

template<class T>
void ChaiGearPrep<T>::basic_setup(Player& P)
{
    Timer timer;
    timer.start();
    assert(machine == 0);
    machine = new MultiplicativeMachine;
    auto& setup = machine->setup.part<FD>();
    auto& options = CowGearOptions::singleton;
#ifdef VERBOSE
    cerr << "Covert security parameter for key and MAC generation: "
            << options.covert_security << endl;
    cerr << "Triple generation security parameter: "
            << options.lowgear_security << endl;
#endif
    machine->sec = options.lowgear_security;
    setup.secure_init(P, *machine, T::clear::length(), options.lowgear_security);
#ifdef VERBOSE
    cerr << T::type_string() << " parameter setup took " << timer.elapsed()
            << " seconds" << endl;
#endif
}

template<class T>
void ChaiGearPrep<T>::key_setup(Player& P, mac_key_type alphai)
{
    Timer timer;
    timer.start();
    assert(machine);
    auto& setup = machine->setup.part<FD>();
    auto& options = CowGearOptions::singleton;
    setup.covert_secrets_generation(P, *machine, options.covert_security);

    // adjust mac key
    mac_key_type diff = alphai - setup.alphai;
    setup.alphai = alphai;
    Bundle<octetStream> bundle(P);
    diff.pack(bundle.mine);
    P.Broadcast_Receive(bundle, true);
    for (int i = 0; i < P.num_players(); i++)
    {
        Plaintext_<FD> mess(setup.FieldD);
        mess.assign_constant(bundle[i].get<mac_key_type>(), Polynomial);
        setup.calpha += mess;
    }

    // generate minimal number of items
    machine->nTriplesPerThread = 1;
#ifdef VERBOSE
    cerr << T::type_string() << " key setup took " << timer.elapsed()
            << " seconds" << endl;
#endif
}

template<class T>
typename ChaiGearPrep<T>::Generator& ChaiGearPrep<T>::get_generator()
{
    auto& proc = this->proc;
    assert(proc != 0);
    lock.lock();
    if (machine == 0 or machine->setup.part<FD>().alphai == 0)
    {
        PlainPlayer P(proc->P.N, T::clear::type_char());
        if (machine == 0)
            basic_setup(P);
        key_setup(P, proc->MC.get_alphai());
    }
    lock.unlock();
    if (generator == 0)
        generator = new SimpleGenerator<SummingEncCommit, FD>(proc->P.N,
                machine->setup.part<FD>(), *machine, 0, DATA_TRIPLE, &proc->P);
    return *generator;
}

template<class T>
void ChaiGearPrep<T>::buffer_triples()
{
    auto& generator = get_generator();
    generator.run(false);
    assert(generator.producer);
    auto& producer = *dynamic_cast<TripleProducer_<FD>*>(generator.producer);
    assert(not producer.triples.empty());
    for (auto& triple : producer.triples)
        this->triples.push_back({{triple[0], triple[1], triple[2]}});
#ifdef VERBOSE_HE
    cerr << "generated " << producer.triples.size() << " triples, now got "
            << this->triples.size() << endl;
#endif
}

template<class T>
void ChaiGearPrep<T>::buffer_squares()
{
    auto& generator = get_generator();
    assert(this->proc);
    auto& setup = machine->setup.part<FD>();
    if (not square_producer)
        square_producer = new SquareProducer<FD>(setup.FieldD,
                this->proc->P.my_num(), 0, false);
    assert(machine);
    square_producer->run(this->proc->P, setup.pk, setup.calpha, generator.EC,
            generator.dd, {});
    MAC_Check<typename T::clear> MC(this->proc->MC.get_alphai());
    square_producer->sacrifice(this->proc->P, MC);
    MC.Check(this->proc->P);
    auto& squares = square_producer->tuples;
    assert(not squares.empty());
    for (auto& square : squares)
        this->squares.push_back({{square[0], square[1]}});
}

template<class T>
void ChaiGearPrep<T>::buffer_inverses()
{
    assert(this->proc != 0);
    ::buffer_inverses(this->inverses, *this, this->proc->MC, this->proc->P);
}

template<class T>
void ChaiGearPrep<T>::buffer_inputs(int player)
{
    auto& generator = get_generator();
    assert(this->proc);
    if (not input_producer)
        input_producer = new InputProducer<FD>(this->proc->P, -1, false);
    assert(machine);
    auto& setup = machine->setup.part<FD>();
    input_producer->run(this->proc->P, setup.pk, setup.calpha, generator.EC,
            generator.dd, {}, player);
    auto& inputs = input_producer->inputs;
    assert(not inputs.empty());
    this->inputs.resize(this->proc->P.num_players());
    for (auto& input : inputs[player])
        this->inputs[player].push_back(input);
#ifdef VERBOSE_HE
    cerr << "generated " << inputs.size() << " inputs, now got "
            << this->inputs[player].size() << endl;
#endif
}

template<>
inline void ChaiGearPrep<ChaiGearShare<gfp>>::buffer_bits()
{
    buffer_bits_from_squares(*this);
}

template<>
inline void ChaiGearPrep<ChaiGearShare<gf2n_short>>::buffer_bits()
{
    buffer_bits_without_check();
    assert(not this->bits.empty());
    for (auto& bit : this->bits)
        bit.force_to_bit();
}
