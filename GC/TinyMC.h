/*
 * TinyMC.h
 *
 */

#ifndef GC_TINYMC_H_
#define GC_TINYMC_H_

#include "Protocols/MAC_Check_Base.h"

namespace GC
{

template<class T>
class TinyMC : public MAC_Check_Base<T>
{
    typename T::part_type::MAC_Check part_MC;
    vector<typename T::part_type::open_type> part_values;
    vector<typename T::part_type::super> part_shares;

public:
    TinyMC(typename T::mac_key_type mac_key) :
            part_MC(mac_key)
    {
        this->alphai = mac_key;
    }

    typename T::part_type::MAC_Check& get_part_MC()
    {
        return part_MC;
    }

    void POpen_Begin(vector<typename T::open_type>& values, const vector<T>& S,
            const Player& P)
    {
        values.clear();
        part_shares.clear();
        for (auto& share : S)
            for (auto& part : share.get_regs())
                part_shares.push_back(part);
        part_MC.POpen_Begin(part_values, part_shares, P);
    }

    void POpen_End(vector<typename T::open_type>& values, const vector<T>& S,
            const Player& P)
    {
        values.clear();
        part_MC.POpen_End(part_values, part_shares, P);
        int i = 0;
        for (auto& share : S)
        {
            typename T::open_type opened = 0;
            for (size_t j = 0; j < share.get_regs().size(); j++)
                opened += typename T::open_type(part_values[i++].get_bit(0)) << j;
            values.push_back(opened);
        }
    }

    void Check(const Player& P)
    {
        part_MC.Check(P);
    }
};

} /* namespace GC */

#endif /* GC_TINYMC_H_ */
