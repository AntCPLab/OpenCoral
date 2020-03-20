#ifndef _Verifier
#define _Verifier

#include "Proof.h"

/* Defines the Verifier */
template <class FD, class S>
class Verifier
{
  AddableVector<S> z;
  AddableMatrix<Int_Random_Coins::value_type::value_type> t;

  Proof& P;

public:
  Verifier(Proof& proof);

  void Stage_2(
      AddableVector<Ciphertext>& c, octetStream& ciphertexts,
      octetStream& cleartexts,const FHE_PK& pk,bool Diag,bool binary=false);

  /* This is the non-interactive version using the ROM
      - Creates space for all output values
      - Diag flag mirrors that in Prover
  */
  void NIZKPoK(AddableVector<Ciphertext>& c,octetStream& ciphertexts,octetStream& cleartexts,
               const FHE_PK& pk,bool Diag,bool binary=false);

  size_t report_size(ReportType type) { return z.report_size(type) + t.report_size(type); }
};

#endif
