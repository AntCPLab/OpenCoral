
#ifndef _Processor
#define _Processor

/* This is a representation of a processing element
 *   Consisting of 256 clear and 256 shared registers
 */

#include "Math/Share.h"
#include "Math/gf2n.h"
#include "Math/gfp.h"
#include "Math/Integer.h"
#include "Exceptions/Exceptions.h"
#include "Networking/Player.h"
#include "Data_Files.h"
#include "Input.h"
#include "ReplicatedInput.h"
#include "PrivateOutput.h"
#include "ReplicatedPrivateOutput.h"
#include "Machine.h"
#include "ExternalClients.h"
#include "Binary_File_IO.h"
#include "Instruction.h"
#include "SPDZ.h"
#include "Replicated.h"
#include "ProcessorBase.h"
#include "Tools/SwitchableOutput.h"

template <class T>
class SubProcessor
{
  vector<typename T::clear> C;
  vector<T> S;

  // This is the vector of partially opened values and shares we need to store
  // as the Open commands are split in two
  vector<typename T::clear> PO;
  vector<T> Sh_PO;

  void resize(int size)       { C.resize(size); S.resize(size); }

  template<class sint, class sgf2n> friend class Processor;
  template<class U> friend class SPDZ;
  template<class U> friend class ProtocolBase;
  template<class U> friend class Beaver;

public:
  ArithmeticProcessor& Proc;
  typename T::MAC_Check& MC;
  Player& P;
  Preprocessing<T>& DataF;

  typename T::Protocol protocol;
  typename T::Input input;

  SubProcessor(ArithmeticProcessor& Proc, typename T::MAC_Check& MC,
      Preprocessing<T>& DataF, Player& P);

  // Access to PO (via calls to POpen start/stop)
  void POpen_Start(const vector<int>& reg,const Player& P,int size);
  void POpen_Stop(const vector<int>& reg,const Player& P,int size);
  void POpen(const vector<int>& reg,const Player& P,int size);

  void muls(const vector<int>& reg, int size);
  void mulrs(const vector<int>& reg);
  void dotprods(const vector<int>& reg);

  vector<T>& get_S()
  {
    return S;
  }

  T& get_S_ref(int i)
  {
    return S[i];
  }

  typename T::clear& get_C_ref(int i)
  {
    return C[i];
  }
};

class ArithmeticProcessor : public ProcessorBase
{
public:
  int thread_num;

  PRNG secure_prng;

  string private_input_filename;

  ifstream private_input;
  ifstream public_input;
  ofstream public_output;
  ofstream private_output;

  int sent, rounds;

  OnlineOptions opts;

  ArithmeticProcessor(OnlineOptions opts, int thread_num) : thread_num(thread_num),
          sent(0), rounds(0), opts(opts) {}
};

template<class sint, class sgf2n>
class Processor : public ArithmeticProcessor
{
  vector<long> Ci;

  int reg_max2,reg_maxp,reg_maxi;

  // Data structure used for reading/writing data to/from a socket (i.e. an external party to SPDZ)
  octetStream socket_stream;

  #ifdef DEBUG
    vector<int> rw2;
    vector<int> rwp;
    vector<int> rwi;
  #endif

  template <class T>
  vector< Share<T> >& get_S();
  template <class T>
  vector<T>& get_C();

  template <class T>
  vector<T>& get_Sh_PO();
  template <class T>
  vector<typename T::clear>& get_PO();

  public:
  Data_Files<sint, sgf2n> DataF;
  Player& P;
  typename sgf2n::MAC_Check& MC2;
  typename sint::MAC_Check& MCp;
  Machine<sint, sgf2n>& machine;

  SubProcessor<sgf2n> Proc2;
  SubProcessor<sint>  Procp;

  typename sgf2n::PrivateOutput privateOutput2;
  typename sint::PrivateOutput privateOutputp;

  unsigned int PC;
  TempVars<sint, sgf2n> temp;

  PRNG shared_prng;

  ExternalClients external_clients;
  Binary_File_IO binary_file_io;
  
  // avoid re-computation of expensive division
  map<int, typename sint::clear> inverses2m;

  SwitchableOutput out;

  static const int reg_bytes = 4;
  
  void reset(const Program& program,int arg); // Reset the state of the processor
  string get_filename(const char* basename, bool use_number);

  Processor(int thread_num,Player& P,
          typename sgf2n::MAC_Check& MC2,typename sint::MAC_Check& MCp,
          Machine<sint, sgf2n>& machine,
          const Program& program);
  ~Processor();

  int get_thread_num()
    {
      return thread_num;
    }

  #ifdef DEBUG  
    const gf2n& read_C2(int i) const
      { if (rw2[i]==0)
	  { throw Processor_Error("Invalid read on clear register"); }
        return Proc2.C.at(i);
      }
    const Share<gf2n> & read_S2(int i) const
      { if (rw2[i+reg_max2]==0)
          { throw Processor_Error("Invalid read on shared register"); }
        return Proc2.S.at(i);
      }
    gf2n& get_C2_ref(int i)
      { rw2[i]=1;
        return Proc2.C.at(i);
      }
    Share<gf2n> & get_S2_ref(int i)
      { rw2[i+reg_max2]=1;
        return Proc2.S.at(i);
      }
    void write_C2(int i,const gf2n& x)
      { rw2[i]=1;
        Proc2.C.at(i)=x;
      }
    void write_S2(int i,const Share<gf2n> & x)
      { rw2[i+reg_max2]=1;
        Proc2.S.at(i)=x;
      }

    const sint::clear& read_Cp(int i) const
      { if (rwp[i]==0)
	  { throw Processor_Error("Invalid read on clear register"); }
        return Procp.C.at(i);
      }
    const sint & read_Sp(int i) const
      { if (rwp[i+reg_maxp]==0)
          { throw Processor_Error("Invalid read on shared register"); }
        return Procp.S.at(i);
      }
    sint::clear& get_Cp_ref(int i)
      { rwp[i]=1;
        return Procp.C.at(i);
      }
    sint & get_Sp_ref(int i)
      { rwp[i+reg_maxp]=1;
        return Procp.S.at(i);
      }
    void write_Cp(int i,const sint::clear& x)
      { rwp[i]=1;
        Procp.C.at(i)=x;
      }
    void write_Sp(int i,const sint & x)
      { rwp[i+reg_maxp]=1;
        Procp.S.at(i)=x;
      }

    const long& read_Ci(int i) const
      { if (rwi[i]==0)
          { throw Processor_Error("Invalid read on integer register"); }
        return Ci.at(i);
      }
    long& get_Ci_ref(int i)
      { rwi[i]=1;
        return Ci.at(i);
      }
    void write_Ci(int i,const long& x)
      { rwi[i]=1;
        Ci.at(i)=x;
      }
 #else
    const gf2n& read_C2(int i) const
      { return Proc2.C[i]; }
    const sgf2n& read_S2(int i) const
      { return Proc2.S[i]; }
    gf2n& get_C2_ref(int i)
      { return Proc2.C[i]; }
    sgf2n& get_S2_ref(int i)
      { return Proc2.S[i]; }
    void write_C2(int i,const gf2n& x)
      { Proc2.C[i]=x; }
    void write_S2(int i,const sgf2n& x)
      { Proc2.S[i]=x; }
  
    const typename sint::clear& read_Cp(int i) const
      { return Procp.C[i]; }
    const sint & read_Sp(int i) const
      { return Procp.S[i]; }
    typename sint::clear& get_Cp_ref(int i)
      { return Procp.C[i]; }
    sint & get_Sp_ref(int i)
      { return Procp.S[i]; }
    void write_Cp(int i,const typename sint::clear& x)
      { Procp.C[i]=x; }
    void write_Sp(int i,const sint & x)
      { Procp.S[i]=x; }

    const long& read_Ci(int i) const
      { return Ci[i]; }
    long& get_Ci_ref(int i)
      { return Ci[i]; }
    void write_Ci(int i,const long& x)
      { Ci[i]=x; }
  #endif

  // Access to external client sockets for reading clear/shared data
  void read_socket_ints(int client_id, const vector<int>& registers);
  // Setup client public key
  void read_client_public_key(int client_id, const vector<int>& registers);
  void init_secure_socket(int client_id, const vector<int>& registers);
  void init_secure_socket_internal(int client_id, const vector<int>& registers);
  void resp_secure_socket(int client_id, const vector<int>& registers);
  void resp_secure_socket_internal(int client_id, const vector<int>& registers);
  
  void write_socket(const RegType reg_type, const SecrecyType secrecy_type, const bool send_macs,
                             int socket_id, int message_type, const vector<int>& registers);

  void read_socket_vector(int client_id, const vector<int>& registers);
  void read_socket_private(int client_id, const vector<int>& registers, bool send_macs);

  // Read and write secret numeric data to file (name hardcoded at present)
  void read_shares_from_file(int start_file_pos, int end_file_pos_register, const vector<int>& data_registers);
  void write_shares_to_file(const vector<int>& data_registers);
  
  // Print the processor state
  template<class T, class U>
  friend ostream& operator<<(ostream& s,const Processor<T, U>& P);

  private:
    void maybe_decrypt_sequence(int client_id);
    void maybe_encrypt_sequence(int client_id);

  template<class T> friend class SPDZ;
  template<class T> friend class SubProcessor;
};

#endif

