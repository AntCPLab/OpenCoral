#ifndef _Data_Files
#define _Data_Files

/* This class holds the Online data files all in one place
 * so the streams are easy to pass around and access
 */

#include "Math/gfp.h"
#include "Math/gf2n.h"
#include "Math/Share.h"
#include "Math/field_types.h"
#include "Processor/Buffer.h"
#include "Processor/InputTuple.h"
#include "Tools/Lock.h"
#include "Networking/Player.h"

#include <fstream>
#include <map>
using namespace std;

enum Dtype { DATA_TRIPLE, DATA_SQUARE, DATA_BIT, DATA_INVERSE, DATA_BITTRIPLE, DATA_BITGF2NTRIPLE, N_DTYPE };

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

template<class sint> class Processor;
template<class sint> class Data_Files;

template<class T>
class Sub_Data_Files
{
  static const bool implemented[N_DTYPE];

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
  Sub_Data_Files(int my_num, int num_players, const string& prep_data_dir,
      DataPositions& usage);
  ~Sub_Data_Files();

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
  void get(SubProcessor<T>& proc, DataTag tag, const vector<int>& regs, int vector_size);
};

template<class sint>
class Data_Files
{
  DataPositions usage;

  public:

  Sub_Data_Files<sint> DataFp;
  Sub_Data_Files<sgf2n> DataF2;

  const string& prep_data_dir;

  Data_Files(int my_num,int n,const string& prep_data_dir);
  Data_Files(Names& N, const string& prep_data_dir) :
      Data_Files(N.my_num(), N.num_players(), prep_data_dir) {}

  DataPositions tellg();
  void seekg(DataPositions& pos);
  void skip(const DataPositions& pos);
  void prune();
  void purge();

  template<class T>
  bool eof(Dtype dtype)
  {
    return get_sub<T>().eof(dtype);
  }
  template<class T>
  bool input_eof(int player)
  {
    return get_sub<T>().input_eof(player);
  }

  void setup_extended(DataFieldType field_type, const DataTag& tag, int tuple_size = 0);
  template<class T>
  void get(SubProcessor<T>& proc, DataTag tag, const vector<int>& regs, int vector_size)
  {
    get_sub<T>().get(proc, tag, regs, vector_size);
  }

  DataPositions get_usage()
  {
    return usage;
  }

  template<class T>
  Sub_Data_Files<T>& get_sub();

  template <class T>
  void get(Dtype dtype, T* a)
  {
    get_sub<T>().get(dtype, a);
  }

  template <class T>
  void get_three(DataFieldType field_type, Dtype dtype, T& a, T& b, T& c)
  {
    (void)field_type;
    get_sub<T>().get_three(dtype, a, b, c);
  }

  template <class T>
  void get_two(DataFieldType field_type, Dtype dtype, T& a, T& b)
  {
    (void)field_type;
    get_sub<T>().get_two(dtype, a, b);
  }

  template <class T>
  void get_one(DataFieldType field_type, Dtype dtype, T& a)
  {
    (void)field_type;
    get_sub<T>().get_one(dtype, a);
  }

  template <class T>
  void get_input(T& a,typename T::clear& x,int i)
  {
    get_sub<T>().get_input(a, x, i);
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

template<>
template<>
inline Sub_Data_Files<sgfp>& Data_Files<sgfp>::get_sub<sgfp>()
{
  return DataFp;
}

template<>
template<>
inline Sub_Data_Files<sgf2n>& Data_Files<sgfp>::get_sub<sgf2n>()
{
  return DataF2;
}

template<>
template<>
inline Sub_Data_Files<Rep3Share>& Data_Files<Rep3Share>::get_sub<Rep3Share>()
{
  return DataFp;
}

template<>
template<>
inline Sub_Data_Files<sgf2n>& Data_Files<Rep3Share>::get_sub<sgf2n>()
{
  return DataF2;
}

#endif
