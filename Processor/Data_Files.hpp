#ifndef PROCESSOR_DATA_FILES_HPP_
#define PROCESSOR_DATA_FILES_HPP_

#include "Processor/Data_Files.h"
#include "Processor/Processor.h"
#include "Protocols/dabit.h"
#include "Math/Setup.h"

template<class T>
Lock Sub_Data_Files<T>::tuple_lengths_lock;
template<class T>
map<DataTag, int> Sub_Data_Files<T>::tuple_lengths;

template<class T>
Preprocessing<T>* Preprocessing<T>::get_live_prep(SubProcessor<T>* proc,
    DataPositions& usage)
{
  return new typename T::LivePrep(proc, usage);
}

template<class T>
template<class U, class V>
Preprocessing<T>* Preprocessing<T>::get_new(
    Machine<U, V>& machine,
    DataPositions& usage, SubProcessor<T>* proc)
{
  if (machine.live_prep)
    return get_live_prep(proc, usage);
  else
    return new Sub_Data_Files<T>(machine.get_N(),
        machine.template prep_dir_prefix<T>(), usage);
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
    Preprocessing<T>(usage),
    my_num(my_num), num_players(num_players), prep_data_dir(prep_data_dir),
    thread_num(thread_num), part(0)
{
#ifdef DEBUG_FILES
  cerr << "Setting up Data_Files in: " << prep_data_dir << endl;
#endif
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

  sprintf(filename, (prep_data_dir + "%s-%s-P%d%s").c_str(),
      DataPositions::dtype_names[DATA_DABIT], (T::type_short()).c_str(), my_num,
      suffix.c_str());
  dabit_buffer.setup(filename, 1, DataPositions::dtype_names[DATA_DABIT]);

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

#ifdef DEBUG_FILES
  cerr << "done\n";
#endif
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
#ifdef VERBOSE
  if (DataFp.data_sent())
    cerr << "Sent for " << sint::type_string() << " preprocessing threads: " <<
        DataFp.data_sent() * 1e-6 << " MB" << endl;
#endif
  delete &DataFp;
#ifdef VERBOSE
  if (DataF2.data_sent())
    cerr << "Sent for " << sgf2n::type_string() << " preprocessing threads: " <<
        DataF2.data_sent() * 1e-6 << " MB" << endl;
#endif
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
  dabit_buffer.close();
  for (auto& x: edabit_buffers)
    {
      x.second->close();
      delete x.second;
    }
  if (part != 0)
    delete part;
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
  skipped.increase(pos);
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
void Sub_Data_Files<T>::get_no_count(vector<T>& S, DataTag tag, const vector<int>& regs, int vector_size)
{
  setup_extended(tag, regs.size());
  for (int j = 0; j < vector_size; j++)
    for (unsigned int i = 0; i < regs.size(); i++)
      extended[tag].input(S[regs[i] + j]);
}

template<class T>
void Sub_Data_Files<T>::get_dabit_no_count(T& a, typename T::bit_type& b)
{
  dabit<T> tmp;
  dabit_buffer.input(tmp);
  a = tmp.first;
  b = tmp.second;
}

template<class T>
void Sub_Data_Files<T>::buffer_edabits_with_queues(bool strict, int n_bits)
{
#ifndef INSECURE
  throw runtime_error("no secure implementation of reading edaBits from files");
#endif
  if (edabit_buffers.find(n_bits) == edabit_buffers.end())
    {
      string filename = prep_data_dir + "edaBits-" + to_string(n_bits) + "-P"
          + to_string(my_num);
      ifstream* f = new ifstream(filename);
      if (f->fail())
        throw runtime_error("cannot open " + filename);
      edabit_buffers[n_bits] = f;
    }
  auto& buffer = *edabit_buffers[n_bits];
  if (buffer.peek() == EOF)
    buffer.seekg(0);
  edabitvec<T> eb;
  eb.input(n_bits, buffer);
  this->edabits[{strict, n_bits}].push_back(eb);
  if (buffer.fail())
    throw runtime_error("error reading edaBits");
}

template<class T>
Preprocessing<typename T::part_type>& Sub_Data_Files<T>::get_part()
{
  if (part == 0)
    part = new Sub_Data_Files<typename T::part_type>(my_num, num_players,
        get_prep_sub_dir<typename T::part_type>(num_players), this->usage,
        thread_num);
  return *part;
}

#endif
