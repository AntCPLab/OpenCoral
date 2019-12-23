/*
 * TinierSharePrep.cpp
 *
 */

#include "TinierSharePrep.h"

namespace GC
{

template<class T>
TinierSharePrep<T>::TinierSharePrep(DataPositions& usage) :
        BufferPrep<T>(usage), triple_generator(0)
{
}

template<class T>
TinierSharePrep<T>::~TinierSharePrep()
{
    if (triple_generator)
        delete triple_generator;
}

template<class T>
void TinierSharePrep<T>::set_protocol(typename T::Protocol& protocol)
{
    params.generateMACs = true;
    params.amplify = false;
    params.check = false;
    auto& thread = ShareThread<TinierSecret<typename T::mac_key_type>>::s();
    triple_generator = new typename T::TripleGenerator(
            BaseMachine::s().fresh_ot_setup(), protocol.P.N, -1,
            OnlineOptions::singleton.batch_size
                    * TinierSecret<typename T::mac_key_type>::default_length, 1,
            params, thread.MC->get_alphai(), &protocol.P);
    triple_generator->multi_threaded = false;
    this->inputs.resize(thread.P->num_players());
}

template<class T>
void TinierSharePrep<T>::buffer_inputs(int player)
{
    auto& inputs = this->inputs;
    assert(triple_generator);
    triple_generator->generateInputs(player);
    for (auto& x : triple_generator->inputs)
        inputs.at(player).push_back(x);
}

template<class T>
size_t TinierSharePrep<T>::data_sent()
{
    if (triple_generator)
        return triple_generator->data_sent();
    else
        return 0;
}

}
