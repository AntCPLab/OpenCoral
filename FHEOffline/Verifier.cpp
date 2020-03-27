#include "Verifier.h"
#include "Math/Z2k.hpp"

template <class FD, class S>
Verifier<FD,S>::Verifier(Proof& proof) : P(proof)
{
#ifdef LESS_ALLOC_MORE_MEM
  z.resize(proof.phim);
  z.allocate_slots(bigint(1) << proof.B_plain_length);
  t.resize(3, proof.phim);
  t.allocate_slots(bigint(1) << proof.B_rand_length);
#endif
}


template <class T, class FD, class S>
bool Check_Decoding(const Plaintext<T,FD,S>& AE,bool Diag)
{
//  // Now check decoding z[i]
//  if (!AE.to_type(0))
//    { cout << "Fail Check 4 " << endl;
//      return false;
//    }
  if (Diag && !AE.is_diagonal())
    { cout << "Fail Check 5 " << endl;
      return false;
    }
  return true;
}

template <class S>
bool Check_Decoding(const vector<S>& AE, bool Diag)
{
  (void)AE;
  if (Diag)
    throw not_implemented();
  return true;
}



template <class FD, class S>
void Verifier<FD,S>::Stage_2(
                          AddableVector<Ciphertext>& c,octetStream& ciphertexts,
                          octetStream& cleartexts,
                          const FHE_PK& pk,bool Diag,bool binary)
{
  unsigned int i, V;

  c.unpack(ciphertexts, pk);
  if (c.size() != P.U)
    throw length_error("number of received ciphertexts incorrect");

  // Now check the encryptions are correct
  Ciphertext d1(pk.get_params()), d2(pk.get_params());
  Random_Coins rc(pk.get_params());
  ciphertexts.get(V);
  if (V != P.V)
    throw length_error("number of received commitments incorrect");
  cleartexts.get(V);
  if (V != P.V)
    throw length_error("number of received cleartexts incorrect");
  for (i=0; i<V; i++)
    {
      z.unpack(cleartexts);
      t.unpack(cleartexts);
      if (!P.check_bounds(z, t, i))
        throw runtime_error("preimage out of bounds");
      d1.unpack(ciphertexts);
      P.apply_challenge(i, d1, c, pk);
      rc.assign(t[0], t[1], t[2]);
      pk.encrypt(d2,z,rc);
      if (!(d1 == d2))
        { cout << "Fail Check 6 " << i << endl; 
          throw runtime_error("ciphertexts don't match");
        }
    }

  // Now check decoding z[i]
  for (i=0; i<V; i++)
    {
      if (!Check_Decoding(z,Diag))
         { cout << "\tCheck : " << i << endl; 
           throw runtime_error("cleartext isn't diagonal");
         }
      if (binary && !z.is_binary())
        {
          cout << "Not binary " << i << endl;
          throw runtime_error("cleartext isn't binary");
        }
    }
}

  

/* This is the non-interactive version using the ROM
*/
template <class FD, class S>
void Verifier<FD,S>::NIZKPoK(AddableVector<Ciphertext>& c,
                          octetStream& ciphertexts, octetStream& cleartexts,
                          const FHE_PK& pk,bool Diag,
                          bool binary)
{
  P.set_challenge(ciphertexts);

  Stage_2(c,ciphertexts,cleartexts,pk,Diag,binary);

  if (P.top_gear)
    {
      assert(not Diag);
      assert(not binary);
      c += c;
    }
}


template class Verifier<FFT_Data, fixint<GFP_MOD_SZ>>;
template class Verifier<P2Data, fixint<GFP_MOD_SZ>>;
