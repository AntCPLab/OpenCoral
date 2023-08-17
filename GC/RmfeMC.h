#ifndef GC_RMFE_MC_H_
#define GC_RMFE_MC_H_

template<class T>
class RmfeMC : public MAC_Check_<T>
{
public:
    RmfeMC(const typename T::mac_key_type::Scalar& mac_key)
    :  Tree_MAC_Check<T>(mac_key), MAC_Check_<T>(mac_key)
    {
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

        this->popen_cnt += this->values.size();
    }
    void CheckFor(const typename T::open_type&, const vector<T>&, const Player&) {
    }

    void Check (const Player& P) {

    }

    RmfeMC<typename T::part_type>& get_part_MC()
    {
        return *this;
    }

    int number()
    {
        return 0;
    }

    typename T::raw_type finalize_open_decoded() {
        return typename T::raw_type(this->finalize_open());
    }
};

#endif