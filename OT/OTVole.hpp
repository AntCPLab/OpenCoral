#include "config.h"

#include "OTVole.h"
#include "Tools/oct.h"
#include "Tools/Subroutines.h"

//#define OTVOLE_TIMER

template <class T>
void OTVoleBase<T>::evaluate(vector<T>& output, const vector<T>& newReceiverInput) {
    const int N1 = newReceiverInput.size() + 1;
    output.resize(newReceiverInput.size());
    vector<octetStream> os(2);

    if (this->ot_role & SENDER) {
        T extra;
        extra.randomize(local_prng);
        vector<T> _corr(newReceiverInput);
        _corr.push_back(extra);
        corr_prime = Row<T>(_corr);
        for (int i = 0; i < S; ++i)
        {
            t0[i] = Row<T>(N1);
            t0[i].randomize(this->G_sender[i][0]);
            t1[i] = Row<T>(N1);
            t1[i].randomize(this->G_sender[i][1]);
            Row<T> u  = corr_prime + t1[i] + t0[i];
            u.pack(os[0]);
        }
    }
    send_if_ot_sender(this->player, os, this->ot_role);
    if (this->ot_role & RECEIVER) {
        for (int i = 0; i < S; ++i)
        {
            t[i] = Row<T>(N1);
            t[i].randomize(this->G_receiver[i]);

            int choice_bit = this->baseReceiverInput.get_bit(i);
            if (choice_bit == 1) {
                u[i].unpack(os[1]);
                a[i] = u[i] - t[i];
            } else {
                a[i] = t[i];
                u[i].unpack(os[1]);
            }
        }
    }
    os[0].reset_write_head();
    os[1].reset_write_head();

    if (!this->passive_only) {
        this->consistency_check(os);
    }

    Row<T> res(N1);
    if (this->ot_role & RECEIVER) {
        for (int i = 0; i < S; ++i)
            res += (a[i] << i);
    }
    if (this->ot_role & SENDER) {
        for (int i = 0; i < S; ++i)
            res -= (t0[i] << i);
    }
    for (int i = 0; i < N1 - 1; ++i)
        output[i] = res.rows[i];
}

template <class T>
void OTVoleBase<T>::evaluate(vector<T>& output, int nValues, const BitVector& newReceiverInput) {
    vector<T> values(nValues);
    if (ot_role & SENDER)
    {
        if (newReceiverInput.size() != (size_t) nValues * 8 * T::N_BYTES)
            throw invalid_length();
        for (int i = 0; i < nValues; ++i)
            values[i] = T(newReceiverInput.get_ptr_to_byte(i, T::N_BYTES));
    }
    evaluate(output, values);
}

template <class T>
void OTVoleBase<T>::set_coeffs(__m128i* coefficients, PRNG& G, int num_blocks) const {
    G.get_octets((octet*) coefficients, num_blocks * sizeof(__m128i));
}

template <class T>
void OTVoleBase<T>::hash_row(octetStream& os, const Row<T>& row, const __m128i* coefficients) {
	octet hash[VOLE_HASH_SIZE] = {0};
	this->hash_row(hash, row, coefficients);
	os.append(hash, VOLE_HASH_SIZE);
}

template <class T>
void OTVoleBase<T>::hash_row(octet* hash, const Row<T>& row, const __m128i* coefficients) {
	int num_blocks = DIV_CEIL(row.size() * T::size(), 16);

	os.clear();
	for(auto& x : row.rows)
	    x.pack(os);
	os.serialize(int128());

	__m128i prods[2];
	avx_memzero(prods, sizeof(prods));
	__m128i res[2];
	avx_memzero(res, sizeof(res));

	for (int i = 0; i < num_blocks; ++i) {
		__m128i block;
		os.unserialize(block);
		mul128(block, coefficients[i], &prods[0], &prods[1]);
		res[0] ^= prods[0];
		res[1] ^= prods[1];
	}

	crypto_generichash(hash, crypto_generichash_BYTES,
			(octet*) res, crypto_generichash_BYTES, NULL, 0);
}

template <class T>
void OTVoleBase<T>::consistency_check(vector<octetStream>& os) {
	PRNG coef_prng_sender;
	PRNG coef_prng_receiver;

    if (this->ot_role & RECEIVER) {
    	coef_prng_receiver.ReSeed();
    	os[0].append(coef_prng_receiver.get_seed(), SEED_SIZE);
    }
    send_if_ot_receiver(this->player, os, this->ot_role);
    if (this->ot_role & SENDER) {
        octet seed[SEED_SIZE];
        os[1].consume(seed, SEED_SIZE);
        coef_prng_sender.SetSeed(seed);
    }
    os[0].reset_write_head();
    os[1].reset_write_head();

    if (this->ot_role & SENDER) {
#ifdef OTVOLE_TIMER
        timeval totalstartv, totalendv;
        gettimeofday(&totalstartv, NULL);
#endif
        int total_bytes = t0[0].size() * T::size();
        int num_blocks = (total_bytes) / 16 + ((total_bytes % 16) != 0);
        __m128i coefficients[num_blocks];
        this->set_coeffs(coefficients, coef_prng_sender, num_blocks);

        Row<T> t00(t0.size()), t01(t0.size()), t10(t0.size()), t11(t0.size());
        for (int alpha = 0; alpha < S; ++alpha)
        {
            for (int i = 0; i < n_challenges(); i++)
            {
                int beta = get_challenge(coef_prng_sender, i);

                t00 = t0[alpha] - t0[beta];
                t01 = t0[alpha] - t1[beta];
                t10 = t1[alpha] - t0[beta];
                t11 = t1[alpha] - t1[beta];

                this->hash_row(os[0], t00, coefficients);
                this->hash_row(os[0], t01, coefficients);
                this->hash_row(os[0], t10, coefficients);
                this->hash_row(os[0], t11, coefficients);
            }
        }
#ifdef OTVOLE_TIMER
        gettimeofday(&totalendv, NULL);
        double elapsed = timeval_diff(&totalstartv, &totalendv);
        cout << "\t\tCheck time sender: " << elapsed/1000000 << endl << flush;
#endif
    }
    
    send_if_ot_sender(this->player, os, this->ot_role);
    if (this->ot_role & RECEIVER) {
#ifdef OTVOLE_TIMER
        timeval totalstartv, totalendv;
        gettimeofday(&totalstartv, NULL);
#endif
        int total_bytes = t[0].size() * T::size();
        int num_blocks = (total_bytes) / 16 + ((total_bytes % 16) != 0);
        __m128i coefficients[num_blocks];
        this->set_coeffs(coefficients, coef_prng_receiver, num_blocks);

        octet h00[VOLE_HASH_SIZE] = {0};
        octet h01[VOLE_HASH_SIZE] = {0};
        octet h10[VOLE_HASH_SIZE] = {0};
        octet h11[VOLE_HASH_SIZE] = {0};
        vector<vector<octet*>> hashes(2);
        hashes[0] = {h00, h01};
        hashes[1] = {h10, h11};

        for (int alpha = 0; alpha < S; ++alpha)
        {
            for (int i = 0; i < n_challenges(); i++)
            {
                int beta = get_challenge(coef_prng_receiver, i);

                os[1].consume(hashes[0][0], VOLE_HASH_SIZE);
                os[1].consume(hashes[0][1], VOLE_HASH_SIZE);
                os[1].consume(hashes[1][0], VOLE_HASH_SIZE);
                os[1].consume(hashes[1][1], VOLE_HASH_SIZE);

                int choice_alpha = this->baseReceiverInput.get_bit(alpha);
                int choice_beta = this->baseReceiverInput.get_bit(beta);

                tmp = t[alpha] - t[beta];
                octet* choice_hash = hashes[choice_alpha][choice_beta];
                octet diff_t[VOLE_HASH_SIZE] = {0};
                this->hash_row(diff_t, tmp, coefficients);
                
                octet* not_choice_hash = hashes[1 - choice_alpha][1 - choice_beta];
                octet other_diff[VOLE_HASH_SIZE] = {0};
                tmp = u[alpha] - u[beta];
                tmp -= t[alpha];
                tmp += t[beta];
                this->hash_row(other_diff, tmp, coefficients);
                
                if (!OCTETS_EQUAL(choice_hash, diff_t, VOLE_HASH_SIZE)) {
                    throw consistency_check_fail();
                }
                if (!OCTETS_EQUAL(not_choice_hash, other_diff, VOLE_HASH_SIZE)) {
                    throw consistency_check_fail();
                }
                if (alpha != beta && u[alpha] == u[beta]) {
                    throw consistency_check_fail();
                }
            }
        }
#ifdef OTVOLE_TIMER
        gettimeofday(&totalendv, NULL);
        double elapsed = timeval_diff(&totalstartv, &totalendv);
        cout << "\t\tCheck receiver: " << elapsed/1000000 << endl << flush;
#endif
    }
}
