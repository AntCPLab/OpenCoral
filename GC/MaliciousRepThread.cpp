/*
 * MalicousRepParty.cpp
 *
 */

#include "Protocols/MaliciousRepMC.h"
#include "MaliciousRepThread.h"
#include "Math/Setup.h"

#include "Protocols/MaliciousRepMC.hpp"
#include "Protocols/MAC_Check_Base.hpp"
#include "Protocols/Beaver.hpp"
#include "Processor/Data_Files.hpp"

namespace GC
{

thread_local MaliciousRepThread* MaliciousRepThread::singleton = 0;

MaliciousRepThread::MaliciousRepThread(int i,
        ThreadMaster<MaliciousRepSecret>& master) :
        Thread<MaliciousRepSecret>(i, master), DataF(usage)
{
}

void MaliciousRepThread::pre_run()
{
    if (singleton)
        throw runtime_error("there can only be one");
    singleton = this;
    DataF.set_protocol(*protocol);
}

void MaliciousRepThread::post_run()
{
#ifndef INSECURE
    cerr << "Removing used pre-processed data" << endl;
    DataF.prune();
#endif
}

void MaliciousRepThread::and_(Processor<MaliciousRepSecret>& processor,
        const vector<int>& args, bool repeat)
{
    assert(P->num_players() == 3);
    processor.check_args(args, 4);
    protocol->init_mul(DataF, *MC);
    for (size_t i = 0; i < args.size(); i += 4)
    {
        int n_bits = args[i];
        int left = args[i + 2];
        int right = args[i + 3];
        MaliciousRepSecret y_ext;
        if (repeat)
            y_ext = processor.S[right].extend_bit();
        else
            y_ext = processor.S[right];
        protocol->prepare_mul(processor.S[left].mask(n_bits), y_ext.mask(n_bits));
    }

    protocol->exchange();

    for (size_t i = 0; i < args.size(); i += 4)
    {
        int n_bits = args[i];
        int out = args[i + 1];
        processor.S[out] = protocol->finalize_mul().mask(n_bits);
    }
}

} /* namespace GC */
