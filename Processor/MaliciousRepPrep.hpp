/*
 * MaliciousRepPrep.cpp
 *
 */

#include "MaliciousRepPrep.h"
#include "Auth/Subroutines.h"
//#include "Auth/MaliciousRepMC.hpp"

template<class T>
MaliciousRepPrep<T>::MaliciousRepPrep(SubProcessor<T>* proc) :
        honest_prep(0), replicated(0)
{
    (void) proc;
}

template<class U>
MaliciousRepPrep<U>::~MaliciousRepPrep()
{
    if (replicated)
        delete replicated;
}

template<class T>
void MaliciousRepPrep<T>::set_protocol(Beaver<T>& protocol)
{
    replicated = new typename T::Honest::Protocol(protocol.P);
    honest_prep.set_protocol(*replicated);
}

template<class U>
void MaliciousRepPrep<U>::clear_tmp()
{
    masked.clear();
    checks.clear();
    check_triples.clear();
    check_squares.clear();
}

template<class T>
void MaliciousRepPrep<T>::buffer_triples()
{
    auto& triples = this->triples;
    auto buffer_size = this->buffer_size;
    clear_tmp();
    Player& P = honest_prep.protocol->P;
    triples.clear();
    for (int i = 0; i < buffer_size; i++)
    {
        T a, b, c;
        T f, g, h;
        honest_prep.get_three(DATA_TRIPLE, a, b, c);
        honest_prep.get_three(DATA_TRIPLE, f, g, h);
        triples.push_back({{a, b, c}});
        check_triples.push_back({{f, g, h}});
    }
    auto t = Create_Random<typename T::clear>(P);
    for (int i = 0; i < buffer_size; i++)
    {
        T& a = triples[i][0];
        T& b = triples[i][1];
        T& f = check_triples[i][0];
        T& g = check_triples[i][1];
        masked.push_back(a * t - f);
        masked.push_back(b - g);
    }
    MC.POpen(opened, masked, P);
    for (int i = 0; i < buffer_size; i++)
    {
        T& b = triples[i][1];
        T& c = triples[i][2];
        T& f = check_triples[i][0];
        T& h = check_triples[i][2];
        typename T::clear& rho = opened[2 * i];
        typename T::clear& sigma = opened[2 * i + 1];
        checks.push_back(t * c - h - rho * b - sigma * f);
    }
    MC.POpen(opened, checks, P);
    for (auto& check : opened)
        if (check != 0)
            throw Offline_Check_Error("triple");
    MC.Check(P);
}

template<class T>
void MaliciousRepPrep<T>::buffer_squares()
{
    auto& squares = this->squares;
    auto buffer_size = this->buffer_size;
    clear_tmp();
    Player& P = honest_prep.protocol->P;
    squares.clear();
    for (int i = 0; i < buffer_size; i++)
    {
        T a, b;
        T f, h;
        honest_prep.get_two(DATA_SQUARE, a, b);
        honest_prep.get_two(DATA_SQUARE, f, h);
        squares.push_back({{a, b}});
        check_squares.push_back({{f, h}});
    }
    auto t = Create_Random<typename T::clear>(P);
    for (int i = 0; i < buffer_size; i++)
    {
        T& a = squares[i][0];
        T& f = check_squares[i][0];
        masked.push_back(a * t - f);
    }
    MC.POpen(opened, masked, P);
    for (int i = 0; i < buffer_size; i++)
    {
        T& a = squares[i][0];
        T& b = squares[i][1];
        T& f = check_squares[i][0];
        T& h = check_squares[i][1];
        auto& rho = opened[i];
        checks.push_back(t * t * b - h - rho * (t * a + f));
    }
    MC.POpen(opened, checks, P);
    for (auto& check : opened)
        if (check != 0)
            throw Offline_Check_Error("square");
}

template<class T>
void MaliciousRepPrep<T>::buffer_inverses()
{
    BufferPrep<T>::buffer_inverses(MC, honest_prep.protocol->P);
}

template<class T>
void MaliciousRepPrep<T>::buffer_bits()
{
    auto& bits = this->bits;
    auto buffer_size = this->buffer_size;
    clear_tmp();
    Player& P = honest_prep.protocol->P;
    bits.clear();
    for (int i = 0; i < buffer_size; i++)
    {
        T a, f, h;
        honest_prep.get_one(DATA_BIT, a);
        honest_prep.get_two(DATA_SQUARE, f, h);
        bits.push_back(a);
        check_squares.push_back({{f, h}});
    }
    auto t = Create_Random<typename T::clear>(P);
    for (int i = 0; i < buffer_size; i++)
    {
        T& a = bits[i];
        T& f = check_squares[i][0];
        masked.push_back(t * a - f);
    }
    MC.POpen(opened, masked, P);
    for (int i = 0; i < buffer_size; i++)
    {
        T& a = bits[i];
        T& f = check_squares[i][0];
        T& h = check_squares[i][1];
        auto& rho = opened[i];
        masked.push_back(t * t * a - h - rho * (t * a + f));
    }
    MC.POpen(opened, checks, P);
    for (auto& check : opened)
        if (check != 0)
            throw Offline_Check_Error("bit");
}
