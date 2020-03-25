/*
 * SohoPrep.cpp
 *
 */

#include "SohoPrep.h"
#include "FHEOffline/DataSetup.h"

template<class T>
PartSetup<typename SohoPrep<T>::FD>* SohoPrep<T>::setup = 0;

template<class T>
Lock SohoPrep<T>::lock;

template<class T>
void SohoPrep<T>::basic_setup(Player& P)
{
    assert(not setup);
    setup = new PartSetup<FD>;
    MachineBase machine;
    setup->secure_init(P, machine, T::clear::length(), 0);
    setup->covert_secrets_generation(P, machine, 1);
}

template<class T>
void SohoPrep<T>::teardown()
{
    if (setup)
        delete setup;
}

template<class T>
void SohoPrep<T>::buffer_triples()
{
    auto& proc = this->proc;
    assert(proc != 0);
    lock.lock();
    if (not setup)
    {
        PlainPlayer P(proc->P.N, T::clear::type_char());
        basic_setup(P);
    }
    lock.unlock();

    Plaintext_<FD> ai(setup->FieldD), bi(setup->FieldD);
    SeededPRNG G;
    ai.randomize(G);
    bi.randomize(G);
    Ciphertext Ca = setup->pk.encrypt(ai);
    Ciphertext Cb = setup->pk.encrypt(bi);
    octetStream os;
    Ca.pack(os);
    Cb.pack(os);

    for (int i = 1; i < proc->P.num_players(); i++)
    {
        proc->P.pass_around(os);
        Ca.add<0>(os);
        Cb.add<0>(os);
    }

    Ciphertext Cc = Ca.mul(setup->pk, Cb);
    Plaintext_<FD> ci(setup->FieldD);
    SimpleDistDecrypt<FD> dd(proc->P, *setup);
    EncCommitBase_<FD> EC;
    dd.reshare(ci, Cc, EC);

    for (unsigned i = 0; i < ai.num_slots(); i++)
        this->triples.push_back({{ai.element(i), bi.element(i),
            ci.element(i)}});
}

template<class T>
void SohoPrep<T>::buffer_inverses()
{
    assert(this->proc != 0);
    ::buffer_inverses(this->inverses, *this, this->proc->MC, this->proc->P);
}
