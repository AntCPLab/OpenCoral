/*
 * MalicousRepParty.cpp
 *
 */

#include "Auth/MaliciousRepMC.h"
#include "MaliciousRepThread.h"
#include "Math/Setup.h"

#include "Auth/MaliciousRepMC.hpp"
#include "Processor/Data_Files.hpp"

namespace GC
{

thread_local MaliciousRepThread* MaliciousRepThread::singleton = 0;

MaliciousRepThread::MaliciousRepThread(int i,
        ThreadMaster<MaliciousRepSecret>& master) :
        Thread<MaliciousRepSecret>(i, master), DataF(N.my_num(),
                N.num_players(),
                get_prep_dir(N.num_players(), 128, gf2n::default_degree()),
                usage, i)
{
}

MaliciousRepMC<MaliciousRepSecret>* MaliciousRepThread::new_mc()
{
    if (machine.more_comm_less_comp)
        return new CommMaliciousRepMC<MaliciousRepSecret>;
    else
        return new HashMaliciousRepMC<MaliciousRepSecret>;
}

void MaliciousRepThread::pre_run()
{
    if (singleton)
        throw runtime_error("there can only be one");
    singleton = this;
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
    os.resize(2);
    for (auto& o : os)
        o.reset_write_head();
    processor.check_args(args, 4);
    shares.clear();
    triples.clear();
    for (size_t i = 0; i < args.size(); i += 4)
    {
        int n_bits = args[i];
        int left = args[i + 2];
        int right = args[i + 3];
        triples.push_back({{0}});
        DataF.get(DATA_TRIPLE, triples.back().data());
        shares.push_back((processor.S[left] - triples.back()[0]).mask(n_bits));
        MaliciousRepSecret y_ext;
        if (repeat)
            y_ext = processor.S[right].extend_bit();
        else
            y_ext = processor.S[right];
        shares.push_back((y_ext - triples.back()[1]).mask(n_bits));
    }

    MC->POpen_Begin(opened, shares, *P);
    MC->POpen_End(opened, shares, *P);
    auto it = opened.begin();

    for (size_t i = 0; i < args.size(); i += 4)
    {
        int n_bits = args[i];
        int out = args[i + 1];
        MaliciousRepSecret tmp = triples[i / 4][2];
        BitVec masked[2];
        for (int k = 0; k < 2; k++)
        {
            masked[k] = *it++;
            tmp += triples[i / 4][1 - k] & masked[k];
        }
        processor.S[out] = (tmp + (masked[0] & masked[1])).mask(n_bits);
    }
}

} /* namespace GC */
