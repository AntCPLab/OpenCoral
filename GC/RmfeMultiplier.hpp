#ifndef GC_RMFEMULTIPLIER_HPP_
#define GC_RMFEMULTIPLIER_HPP_

#include "GC/RmfeMultiplier.h"

template<class T>
void RmfeMultiplier<T>::after_correlation() {
    throw runtime_error("not implemented for rmfe multiplier");
}

template<class T>
void RmfeMultiplier<T>::init_authenticator(const BitVector& keyBits,
		const vector< array<BitVector, 2> >& senderOutput,
		const vector<BitVector>& receiverOutput) {
    typename T::mac_key_type key = keyBits.get_portion<typename T::mac_key_type>(0);
    typename T::encoded_mac_type encoded_key(key);
    this->auth_ot_ext.init(encoded_key);
}

template<class T>
RmfeMultiplier<T>::RmfeMultiplier(OTTripleGenerator<T>& generator, int thread_num) :
        OTMultiplier<T>(generator, thread_num),
    auth_ot_ext(generator.players[thread_num], BOTH, true, BaseMachine::thread_num) {
}

template<class T>
RmfeMultiplier<T>::~RmfeMultiplier() {
}

template<class T>
void RmfeMultiplier<T>::multiply() {
    // [zico] Multiplier is run in a separate thread, so we need to setup MFE again for the `thread_local` singleton.
    RmfeBeaver<T>::setup_mfe();

    OTMultiplier<T>::multiply();

    RmfeBeaver<T>::teardown_mfe();
}

template<class T>
void RmfeMultiplier<T>::multiplyForInputs(MultJob job) {
    assert(job.input);
    assert(job.n_inputs % 128 == 0);
    auto& generator = this->generator;
    bool mine = job.player == generator.my_num;
    auth_ot_ext.set_role(mine ? SENDER : RECEIVER);
    if (mine)
        this->inbox.pop();

    int last_part_n_bytes = DIV_CEIL(T::encoded_mac_type::DEFAULT_LENGTH % 128, 8);
    int n_parts = DIV_CEIL(T::encoded_mac_type::DEFAULT_LENGTH, 128);
    vector<BitMatrix> senderInput(n_parts);
    if (mine) {
        for (size_t i = 0; i < senderInput.size(); i++) {
            senderInput[i].resize(job.n_inputs);
        }
        for (int j = 0; j < job.n_inputs; j++) {
            typename T::encoded_mac_type input(generator.valueBits[0].template get_portion<typename T::open_type>(j));
            octet* ptr = input.get_ptr();
            for(int i = 0; i < n_parts; i++) {
                if (i < n_parts - 1 || last_part_n_bytes == 0) {
                    senderInput[i][j] = *((__m128i*) ptr + i);
                }
                else {
                    memcpy((octet*)(&senderInput[i][j]), ptr + i*sizeof(__m128i), last_part_n_bytes);
                }
            }
        }
        for (size_t i = 0; i < senderInput.size(); i++)
            senderInput[i].transpose();
    }

    vector<BitMatrix> output;
    auth_ot_ext.correlate(output, senderInput, job.n_inputs);
    for (size_t i = 0; i < output.size(); i++)
        output[i].transpose();

    auto& input_macs = this->input_macs;
    input_macs.resize(job.n_inputs);
    for (int j = 0; j < job.n_inputs; j++) {
        typename T::encoded_mac_type encoded_mac;
        octet* ptr = encoded_mac.get_ptr();
        for (int i = 0; i < n_parts; i++) {
            if (i < n_parts - 1 || last_part_n_bytes == 0) {
                *((__m128i*) ptr + i) = output[i][j];
            }
            else {
                memcpy(ptr + i*sizeof(__m128i), (octet*)(&output[i][j]), last_part_n_bytes);
            }
        }
        input_macs[j] = typename T::mac_type(encoded_mac);   
    }
    this->outbox.push(job);
}

// [zico] There could be multithread bug here because now 'thread_num' is always -1
template<class T>
Fole<T>::Fole(TwoPartyPlayer* player, OT_ROLE role, bool passive, int thread_num)
    : role(role), player(player), ot(0), ot_reversed(0) {
    ios = new EmpChannel*[threads];
	ios[0] = new EmpChannel(player);
    int emp_party = player->my_num() < player->other_player_num() ? emp::ALICE : emp::BOB;

    string prep_dir = OnlineOptions::singleton.prep_dir_prefix<T>(player->num_players());

    string ferret_pre_file = PrepBase::get_ferret_filename(prep_dir, 
        player->my_num(), player->other_player_num(), emp_party == emp::ALICE, thread_num);
    ot = new SilentOT<EmpChannel>(emp_party, threads, ios, 
        !passive, true, ferret_pre_file);

    if (role == BOTH) {
        string reversed_ferret_pre_file = PrepBase::get_ferret_filename(prep_dir, 
            player->my_num(), player->other_player_num(), emp_party == emp::BOB, thread_num);
        ot_reversed = new SilentOT<EmpChannel>(3 - emp_party, threads, ios, 
            !passive, true, reversed_ferret_pre_file);
    }
}

template<class T>
Fole<T>::~Fole() {
    if (ios) {
        delete ios[0];
        delete[] ios;
    }
    if (ot) {
        delete ot;
    }
    if (ot_reversed)
        delete ot_reversed;
}

template<class T>
void Fole<T>::init(const BitVector& keyBits) {
    this->keyBits = keyBits;
}

template<class T>
void Fole<T>::set_role(OT_ROLE role) {
    this->role = role;
}

template<class T>
void Fole<T>::correlate(vector<BitMatrix>& output, const vector<BitMatrix>& senderInput, int n) {
    // [zico] Don't allow waste for best efficiency 
    assert(n % 128 == 0);
    if (role == SENDER) {
        assert(senderInput.size() == (size_t) DIV_CEIL(this->keyBits.size(), 128));
        for (size_t i = 0; i < senderInput.size(); i++)
            assert(senderInput[i].vertical_size() == (size_t) n);
    }
    int emp_party = player->my_num() < player->other_player_num() ? emp::ALICE : emp::BOB;

    output.resize(DIV_CEIL(this->keyBits.size(), 128));
    for(size_t i = 0; i < output.size(); i++)
        output[i].resize(n);

    bool* choices = new bool[keyBits.size()];
    for (size_t i = 0; i < keyBits.size(); i++)
        choices[i] = keyBits.get_bit(i);

    for (size_t i = 0; i < output.size(); i++) {
        int corr_size = std::min(128, (int)(this->keyBits.size() - i * 128));
        for (int j = 0; j < n / 128; j++) {
            if (emp_party == emp::ALICE && role == SENDER) {
                ot->send_ot_cxm_cc(output[i].squares[j].rows, senderInput[i].squares[j].rows, corr_size);
            }
            else if (emp_party == emp::BOB && role == RECEIVER) {
                ot->recv_ot_cxm_cc(output[i].squares[j].rows, choices + i*128, corr_size);
            }
            else if (emp_party == emp::ALICE && role == RECEIVER) {
                ot_reversed->recv_ot_cxm_cc(output[i].squares[j].rows, choices + i*128, corr_size);
            }
            else if (emp_party == emp::BOB && role == SENDER) {
                ot_reversed->send_ot_cxm_cc(output[i].squares[j].rows, senderInput[i].squares[j].rows, corr_size);
            }
        }
    }

    delete[] choices;

}

#endif