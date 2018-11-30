/*
 * DummyProtocol.h
 *
 */

#ifndef PROCESSOR_DUMMYPROTOCOL_H_
#define PROCESSOR_DUMMYPROTOCOL_H_

class Player;

class DummyMC
{
public:
    void Check(Player& P)
    {
        (void) P;
    }
};

class DummyProtocol
{
public:
    DummyProtocol(Player& P)
    {
        (void) P;
    }
};

#endif /* PROCESSOR_DUMMYPROTOCOL_H_ */
