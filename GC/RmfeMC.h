#ifndef GC_RMFE_MC_H_
#define GC_RMFE_MC_H_

#include "Tools/debug.h"

template<class T>
class RmfeMC : public MAC_Check_<T>
{
public:
    RmfeMC(const typename T::mac_key_type::Scalar& mac_key)
    :  Tree_MAC_Check<T>(mac_key), MAC_Check_<T>(mac_key)
    {
    }

    virtual ~RmfeMC() {
        print_general("Waiting for check", this->WaitingForCheck(), "RmfeMC destroy");
    }

    void prepare_open(const T& secret, int = -1)
    {
        this->values.push_back(secret.get_share());
        this->macs.push_back(secret.get_mac());
    }

    /**
     * Insecure. Just open the values.
    */
    void exchange(const Player& P)
    {
        vector<octetStream> oss;
        oss.resize(P.num_players());
        oss[P.my_num()].reset_write_head();
        oss[P.my_num()].reserve(this->values.size() * T::open_type::size());

        for (auto& x : this->values)
            x.pack(oss[P.my_num()]);
        oss[P.my_num()].append(0);

        P.unchecked_broadcast(oss);

        direct_add_openings<typename T::open_type>(this->values, P, oss);

        this->AddToValues(this->values);
        this->popen_cnt += this->values.size();
        this->CheckIfNeeded(P);
    }

    void Check (const Player& P) {
        // [zico] Need to update

        assert(T::mac_type::invertible);
        check_field_size<typename T::mac_type>();

        if (this->WaitingForCheck() == 0)
            return;

        auto& vals = this->vals;
        auto& macs = this->macs;
        auto& popen_cnt = this->popen_cnt;
        assert(int(macs.size()) == popen_cnt);
        assert(int(vals.size()) == popen_cnt);
        assert(this->coordinator);

        this->vals.erase(this->vals.begin(), this->vals.begin() + this->popen_cnt);
        this->macs.erase(this->macs.begin(), this->macs.begin() + this->popen_cnt);

        this->popen_cnt=0;
    }

    RmfeMC<typename T::part_type>& get_part_MC()
    {
        return *this;
    }

    typename T::raw_type finalize_open_decoded() {
        return typename T::raw_type(this->finalize_open());
    }
};

#endif