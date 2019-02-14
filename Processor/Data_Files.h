#ifndef _Data_Files
#define _Data_Files

/* This class holds the Online data files all in one place
 * so the streams are easy to pass around and access
 */

#include "Math/gfp.h"
#include "Math/gf2n.h"
#include "Math/Share.h"
#include "Math/field_types.h"
#include "Tools/Buffer.h"
#include "Processor/InputTuple.h"
#include "Tools/Lock.h"
#include "Networking/Player.h"

#include <fstream>
#include <map>
using namespace std;

class DataTag
{
  int t[4];

public:
  // assume that tag is three integers
  DataTag(const int* tag)
  {
    strncpy((char*)t, (char*)tag, 3 * sizeof(int));
    t[3] = 0;
  }
  string get_string() const
  {
    return string((char*)t);
  }
  bool operator<(const DataTag& other) const
  {
    for (int i = 0; i < 3; i++)
      if (t[i] != other.t[i])
        return t[i] < other.t[i];
    return false;
  }
};

struct DataPositions
{
  static const char* dtype_names[N_DTYPE];
  static const char* field_names[N_DATA_FIELD_TYPE];
  static const int tuple_size[N_DTYPE];

  vector< vector<long long> > files;
  vector< vector<long long> > inputs;
  map<DataTag, long long> extended[N_DATA_FIELD_TYPE];

  DataPositions(int num_players = 0) { set_num_players(num_players); }
  void set_num_players(int num_players);
  void increase(const DataPositions& delta);
  void print_cost() const;
};

template<class sint, class sgf2n> class Processor;
template<class sint, class sgf2n> class Data_Files;
template<class sint, class sgf2n> class Machine;

template<class T>
class Preprocessing
{
public:
  template<class U, class V>
  static Preprocessing<T>* get_new(Machine<U, V>& machine, DataPositions& usage,
      SubProcessor<T>* proc);
  static Preprocessing<T>* get_live_prep(SubProcessor<T>* proc);

  virtual ~Preprocessing() {}

  virtual void set_protocol(typename T::Protocol& protocol) = 0;

  virtual void seekg(DataPositions& pos) { (void) pos; }
  virtual void prune() {}
  virtual void purge() {}

  virtual void get(Dtype dtype, T* a);
  virtual void get_three(Dtype dtype, T& a, T& b, T& c) = 0;
  virtual void get_two(Dtype dtype, T& a, T& b) = 0;
  virtual void get_one(Dtype dtype, T& a) = 0;
  virtual void get_input(T& a, typename T::clear& x, int i) = 0;
  virtual void get(vector<T>& S, DataTag tag, const vector<int>& regs,
      int vector_size) = 0;
};

template<class T>
class Sub_Data_Files : public Preprocessing<T>
{
  template<class U> friend class Sub_Data_Files;

  static map<DataTag, int> tuple_lengths;
  static Lock tuple_lengths_lock;

  static int tuple_length(int dtype);

  BufferOwner<T, T> buffers[N_DTYPE];
  vector<BufferOwner<T, T>> input_buffers;
  BufferOwner<InputTuple<T>, RefInputTuple<T>> my_input_buffers;
  map<DataTag, BufferOwner<T, T> > extended;

  int my_num,num_players;

  const string prep_data_dir;

  DataPositions& usage;

public:
  static string get_suffix(int thread_num);

  Sub_Data_Files(int my_num, int num_players, const string& prep_data_dir,
      DataPositions& usage, int thread_num = -1);
  Sub_Data_Files(const Names& N, const string& prep_data_dir,
      DataPositions& usage, int thread_num = -1) :
      Sub_Data_Files(N.my_num(), N.num_players(), prep_data_dir, usage, thread_num)
  {
  }
  ~Sub_Data_Files();

  void set_protocol(typename T::Protocol& protocol) { (void) protocol; }

  void seekg(DataPositions& pos);
  void prune();
  void purge();

  bool eof(Dtype dtype);
  bool input_eof(int player);

  void get(Dtype dtype, T* a);

  void get_three(Dtype dtype, T& a, T& b, T& c)
  {
    usage.files[T::field_type()][dtype]++;
    buffers[dtype].input(a);
    buffers[dtype].input(b);
    buffers[dtype].input(c);
  }

  void get_two(Dtype dtype, T& a, T& b)
  {
    usage.files[T::field_type()][dtype]++;
    buffers[dtype].input(a);
    buffers[dtype].input(b);
  }

  void get_one(Dtype dtype, T& a)
  {
    usage.files[T::field_type()][dtype]++;
    buffers[dtype].input(a);
  }

  void get_input(T& a,typename T::clear& x,int i)
  {
    usage.inputs[i][T::field_type()]++;
    RefInputTuple<T> tuple(a, x);
    if (i==my_num)
      my_input_buffers.input(tuple);
    else
      input_buffers[i].input(a);
  }

  void setup_extended(const DataTag& tag, int tuple_size = 0);
  void get(vector<T>& S, DataTag tag, const vector<int>& regs, int vector_size);
};

template<class sint, class sgf2n>
class Data_Files
{
  DataPositions usage;

  public:

  Preprocessing<sint>& DataFp;
  Preprocessing<sgf2n>& DataF2;

  Data_Files(Machine<sint, sgf2n>& machine, SubProcessor<sint>* procp = 0,
      SubProcessor<sgf2n>* proc2 = 0);
  ~Data_Files();

  DataPositions tellg();
  void seekg(DataPositions& pos);
  void skip(const DataPositions& pos);
  void prune();
  void purge();

  DataPositions get_usage()
  {
    return usage;
  }
};

template<class T> inline
bool Sub_Data_Files<T>::eof(Dtype dtype)
  { return buffers[dtype].eof; }

template<class T> inline
bool Sub_Data_Files<T>::input_eof(int player)
{
  if (player == my_num)
    return my_input_buffers.eof;
  else
    return input_buffers[player].eof;
}

template<class T>
inline void Sub_Data_Files<T>::get(Dtype dtype, T* a)
{
  usage.files[T::field_type()][dtype]++;
  for (int i = 0; i < DataPositions::tuple_size[dtype]; i++)
    buffers[dtype].input(a[i]);
}

template<class T>
inline void Preprocessing<T>::get(Dtype dtype, T* a)
{
  switch (dtype)
  {
  case DATA_TRIPLE:
      get_three(dtype, a[0], a[1], a[2]);
      break;
  case DATA_SQUARE:
  case DATA_INVERSE:
      get_two(dtype, a[0], a[1]);
      break;
  case DATA_BIT:
      get_one(dtype, a[0]);
      break;
  default:
      throw not_implemented();
  }
}

#endif
