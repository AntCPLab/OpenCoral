
#include "Processor/Data_Files.h"
#include "Processor/Processor.h"
#include "Processor/ReplicatedPrep.h"
#include "Processor/MaliciousRepPrep.h"
#include "GC/MaliciousRepSecret.h"
#include "Math/MaliciousRep3Share.h"
#include "Math/ShamirShare.h"
#include "Math/MaliciousShamirShare.h"

#include "Processor/MaliciousRepPrep.hpp"
//#include "Processor/Replicated.hpp"
#include "Processor/ReplicatedPrep.hpp"
//#include "Processor/Input.hpp"
//#include "Processor/ReplicatedInput.hpp"
//#include "Processor/Shamir.hpp"
//#include "Auth/MaliciousShamirMC.hpp"

#include <iomanip>
#include <numeric>

const char* DataPositions::field_names[] = { "int", "gf2n", "bit" };

const int DataPositions::tuple_size[N_DTYPE] = { 3, 2, 1, 2, 3, 3 };

template<class T>
Lock Sub_Data_Files<T>::tuple_lengths_lock;
template<class T>
map<DataTag, int> Sub_Data_Files<T>::tuple_lengths;

template<class T>
Preprocessing<T>* Preprocessing<T>::get_live_prep(SubProcessor<T>* proc)
{
  (void) proc;
  throw not_implemented();
}

template<class T>
template<class U, class V>
Preprocessing<T>* Preprocessing<T>::get_new(
    Machine<U, V>& machine,
    DataPositions& usage, SubProcessor<T>* proc)
{
  if (machine.live_prep)
    return get_live_prep(proc);
  else
    return new Sub_Data_Files<T>(machine.get_N(), machine.prep_dir_prefix, usage);
}

void DataPositions::set_num_players(int num_players)
{
  files.resize(N_DATA_FIELD_TYPE, vector<long long>(N_DTYPE));
  inputs.resize(num_players, vector<long long>(N_DATA_FIELD_TYPE));
}

void DataPositions::increase(const DataPositions& delta)
{
  if (inputs.size() != delta.inputs.size())
    throw invalid_length();
  for (unsigned int field_type = 0; field_type < N_DATA_FIELD_TYPE; field_type++)
    {
      for (unsigned int dtype = 0; dtype < N_DTYPE; dtype++)
        files[field_type][dtype] += delta.files[field_type][dtype];
      for (unsigned int j = 0; j < inputs.size(); j++)
        inputs[j][field_type] += delta.inputs[j][field_type];

      map<DataTag, long long>::const_iterator it;
      const map<DataTag, long long>& delta_ext = delta.extended[field_type];
      for (it = delta_ext.begin(); it != delta_ext.end(); it++)
          extended[field_type][it->first] += it->second;
    }
}

void DataPositions::print_cost() const
{
  ifstream file("cost");
  double total_cost = 0;
  for (int i = 0; i < N_DATA_FIELD_TYPE; i++)
    {
      if (accumulate(files[i].begin(), files[i].end(), 0) > 0)
        cerr << "  Type " << field_names[i] << endl;
      bool reading_field = true;
      for (int j = 0; j < N_DTYPE; j++)
        {
          double cost_per_item = 0;
          if (reading_field)
            file >> cost_per_item;
          if (cost_per_item < 0)
            {
              reading_field = false;
              cost_per_item = 0;
            }
          long long items_used = files[i][j];
          double cost = items_used * cost_per_item;
          total_cost += cost;
          cerr.fill(' ');
          if (items_used)
              cerr << "    " << setw(10) << cost << " = " << setw(10) << items_used
                  << " " << setw(14) << dtype_names[j] << " @ " << setw(11)
                  << cost_per_item << endl;
        }
        for (map<DataTag, long long>::const_iterator it = extended[i].begin();
                it != extended[i].end(); it++)
        {
          cerr.fill(' ');
          cerr << setw(27) << it->second << " " << setw(14) << it->first.get_string() << endl;
        }
    }

  if (total_cost > 0)
    cerr << "Total cost: " << total_cost << endl;
}


template<class T>
int Sub_Data_Files<T>::tuple_length(int dtype)
{
  return DataPositions::tuple_size[dtype] * T::size();
}

template<class T>
string Sub_Data_Files<T>::get_suffix(int thread_num)
{
#ifdef INSECURE
  (void) thread_num;
  return "";
#else
  if (thread_num >= 0)
    return "-T" + to_string(thread_num);
  else
    return "";
#endif
}

template<class T>
Sub_Data_Files<T>::Sub_Data_Files(int my_num, int num_players,
    const string& prep_data_dir, DataPositions& usage, int thread_num) :
    my_num(my_num), num_players(num_players), prep_data_dir(prep_data_dir),
    usage(usage)
{
  cerr << "Setting up Data_Files in: " << prep_data_dir << endl;
  char filename[1024];
  string suffix = get_suffix(thread_num);
  for (int dtype = 0; dtype < N_DTYPE; dtype++)
    {
      if (T::clear::allows(Dtype(dtype)))
        {
          sprintf(filename,(prep_data_dir + "%s-%s-P%d%s").c_str(),DataPositions::dtype_names[dtype],
              (T::type_short()).c_str(),my_num,suffix.c_str());
          buffers[dtype].setup(filename,
              tuple_length(dtype), DataPositions::dtype_names[dtype]);
        }
    }

  input_buffers.resize(num_players);
  for (int i=0; i<num_players; i++)
    {
      sprintf(filename,(prep_data_dir + "Inputs-%s-P%d-%d%s").c_str(),
          (T::type_short()).c_str(),my_num,i,suffix.c_str());
      if (i == my_num)
        my_input_buffers.setup(filename,
            T::size() * 3 / 2);
      else
        input_buffers[i].setup(filename,
            T::size());
    }

  cerr << "done\n";
}

template<class sint, class sgf2n>
Data_Files<sint, sgf2n>::Data_Files(Machine<sint, sgf2n>& machine, SubProcessor<sint>* procp,
    SubProcessor<sgf2n>* proc2) :
    usage(machine.get_N().num_players()),
    DataFp(*Preprocessing<sint>::get_new(machine, usage, procp)),
    DataF2(*Preprocessing<sgf2n>::get_new(machine, usage, proc2))
{
}

template<class sint, class sgf2n>
Data_Files<sint, sgf2n>::~Data_Files()
{
  delete &DataFp;
  delete &DataF2;
}

template<class T>
Sub_Data_Files<T>::~Sub_Data_Files()
{
  for (int i = 0; i < N_DTYPE; i++)
    buffers[i].close();
  for (int i = 0; i < num_players; i++)
    input_buffers[i].close();
  my_input_buffers.close();
  for (auto it =
      extended.begin(); it != extended.end(); it++)
    it->second.close();
}

template<class T>
void Sub_Data_Files<T>::seekg(DataPositions& pos)
{
  DataFieldType field_type = T::field_type();
  for (int dtype = 0; dtype < N_DTYPE; dtype++)
    if (T::clear::allows(Dtype(dtype)))
      buffers[dtype].seekg(pos.files[field_type][dtype]);
  for (int j = 0; j < num_players; j++)
    if (j == my_num)
      my_input_buffers.seekg(pos.inputs[j][field_type]);
    else
      input_buffers[j].seekg(pos.inputs[j][field_type]);
  for (map<DataTag, long long>::const_iterator it = pos.extended[field_type].begin();
      it != pos.extended[field_type].end(); it++)
    {
      setup_extended(it->first);
      extended[it->first].seekg(it->second);
    }
}

template<class sint, class sgf2n>
void Data_Files<sint, sgf2n>::seekg(DataPositions& pos)
{
  DataFp.seekg(pos);
  DataF2.seekg(pos);
  usage = pos;
}

template<class sint, class sgf2n>
void Data_Files<sint, sgf2n>::skip(const DataPositions& pos)
{
  DataPositions new_pos = usage;
  new_pos.increase(pos);
  seekg(new_pos);
}

template<class T>
void Sub_Data_Files<T>::prune()
{
  for (auto& buffer : buffers)
    buffer.prune();
  my_input_buffers.prune();
  for (int j = 0; j < num_players; j++)
    input_buffers[j].prune();
  for (auto it : extended)
    it.second.prune();
}

template<class sint, class sgf2n>
void Data_Files<sint, sgf2n>::prune()
{
  DataFp.prune();
  DataF2.prune();
}

template<class T>
void Sub_Data_Files<T>::purge()
{
  for (auto& buffer : buffers)
    buffer.purge();
  my_input_buffers.purge();
  for (int j = 0; j < num_players; j++)
    input_buffers[j].purge();
  for (auto it : extended)
    it.second.purge();
}

template<class sint, class sgf2n>
void Data_Files<sint, sgf2n>::purge()
{
  DataFp.purge();
  DataF2.purge();
}

template<class T>
void Sub_Data_Files<T>::setup_extended(const DataTag& tag, int tuple_size)
{
  BufferBase& buffer = extended[tag];
  tuple_lengths_lock.lock();
  int tuple_length = tuple_lengths[tag];
  int my_tuple_length = tuple_size * T::size();
  if (tuple_length > 0)
    {
      if (tuple_size > 0 && my_tuple_length != tuple_length)
        {
          stringstream ss;
          ss << "Inconsistent size of " << T::type_string() << " "
              << tag.get_string() << ": " << my_tuple_length << " vs "
              << tuple_length;
          throw Processor_Error(ss.str());
        }
    }
  else
    tuple_lengths[tag] = my_tuple_length;
  tuple_lengths_lock.unlock();

  if (!buffer.is_up())
    {
      stringstream ss;
      ss << prep_data_dir << tag.get_string() << "-" << T::type_short() << "-P" << my_num;
      extended[tag].setup(ss.str(), tuple_length);
    }
}

template<class T>
void Sub_Data_Files<T>::get(vector<T>& S, DataTag tag, const vector<int>& regs, int vector_size)
{
  usage.extended[T::field_type()][tag] += vector_size;
  setup_extended(tag, regs.size());
  for (int j = 0; j < vector_size; j++)
    for (unsigned int i = 0; i < regs.size(); i++)
      extended[tag].input(S[regs[i] + j]);
}
