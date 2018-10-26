
#include "Processor/Data_Files.h"
#include "Processor/Processor.h"

#include <iomanip>

const char* DataPositions::field_names[] = { "sint", "sgf2n" };

template<>
const bool Sub_Data_Files<sgfp>::implemented[N_DTYPE] =
    { true, true, true, true, false, false }
;

template<>
const bool Sub_Data_Files<sgf2n>::implemented[N_DTYPE] =
    { true, true, true, true, true, true }
;

template<>
const bool Sub_Data_Files<Rep3Share>::implemented[N_DTYPE] =
    { false, false, true, false, false, false }
;

const int DataPositions::tuple_size[N_DTYPE] = { 3, 2, 1, 2, 3, 3 };

template<class T>
Lock Sub_Data_Files<T>::tuple_lengths_lock;
template<class T>
map<DataTag, int> Sub_Data_Files<T>::tuple_lengths;


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
      cerr << "  Type " << field_names[i] << endl;
      for (int j = 0; j < N_DTYPE; j++)
        {
          double cost_per_item = 0;
          file >> cost_per_item;
          if (cost_per_item < 0)
            break;
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

  cerr << "Total cost: " << total_cost << endl;
}


template<class T>
int Sub_Data_Files<T>::tuple_length(int dtype)
{
  return DataPositions::tuple_size[dtype] * T::size();
}

template<class T>
Sub_Data_Files<T>::Sub_Data_Files(int my_num, int num_players,
    const string& prep_data_dir, DataPositions& usage) :
    my_num(my_num), num_players(num_players), prep_data_dir(prep_data_dir),
    usage(usage)
{
  cerr << "Setting up Data_Files in: " << prep_data_dir << endl;
  char filename[1024];
  for (int dtype = 0; dtype < N_DTYPE; dtype++)
    {
      if (implemented[dtype])
        {
          sprintf(filename,(prep_data_dir + "%s-%s-P%d").c_str(),DataPositions::dtype_names[dtype],
              string(1, T::type_char()).c_str(),my_num);
          buffers[dtype].setup(filename,
              tuple_length(dtype), DataPositions::dtype_names[dtype]);
        }
    }

  input_buffers.resize(num_players);
  for (int i=0; i<num_players; i++)
    {
      sprintf(filename,(prep_data_dir + "Inputs-%s-P%d-%d").c_str(),
          string(1, T::type_char()).c_str(),my_num,i);
      if (i == my_num)
        my_input_buffers.setup(filename,
            T::size() * 3 / 2);
      else
        input_buffers[i].setup(filename,
            T::size());
    }

  cerr << "done\n";
}

template<class sint>
Data_Files<sint>::Data_Files(int myn, int n, const string& prep_data_dir) :
    usage(n), DataFp(myn, n, prep_data_dir, usage),
    DataF2(myn, n, prep_data_dir, usage), prep_data_dir(prep_data_dir)
{
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
    if (implemented[dtype])
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

template<class sint>
void Data_Files<sint>::seekg(DataPositions& pos)
{
  DataFp.seekg(pos);
  DataF2.seekg(pos);
  usage = pos;
}

template<class sint>
void Data_Files<sint>::skip(const DataPositions& pos)
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

template<class sint>
void Data_Files<sint>::prune()
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

template<class sint>
void Data_Files<sint>::purge()
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
      ss << prep_data_dir << tag.get_string() << "-" << T::type_char() << "-P" << my_num;
      extended[tag].setup(ss.str(), tuple_length);
    }
}

template<class T>
void Sub_Data_Files<T>::get(SubProcessor<T>& proc, DataTag tag, const vector<int>& regs, int vector_size)
{
  usage.extended[T::field_type()][tag] += vector_size;
  setup_extended(tag, regs.size());
  for (int j = 0; j < vector_size; j++)
    for (unsigned int i = 0; i < regs.size(); i++)
      extended[tag].input(proc.get_S_ref(regs[i] + j));
}

template class Sub_Data_Files<sgf2n>;
template class Sub_Data_Files<sgfp>;
template class Sub_Data_Files<Rep3Share>;

template class Data_Files<sgfp>;
template class Data_Files<Rep3Share>;
