/*
 * DataPositions.cpp
 *
 */

#include "Data_Files.h"

#include <iomanip>
#include <numeric>

const char* DataPositions::field_names[] = { "int", "gf2n", "bit" };

const int DataPositions::tuple_size[N_DTYPE] = { 3, 2, 1, 2, 3, 3 };

void DataPositions::set_num_players(int num_players)
{
  files.resize(N_DATA_FIELD_TYPE, vector<long long>(N_DTYPE));
  inputs.resize(num_players, vector<long long>(N_DATA_FIELD_TYPE));
}

void DataPositions::increase(const DataPositions& delta)
{
  inputs.resize(max(inputs.size(), delta.inputs.size()), vector<long long>(N_DATA_FIELD_TYPE));
  for (unsigned int field_type = 0; field_type < N_DATA_FIELD_TYPE; field_type++)
    {
      for (unsigned int dtype = 0; dtype < N_DTYPE; dtype++)
        files[field_type][dtype] += delta.files[field_type][dtype];
      for (unsigned int j = 0; j < delta.inputs.size(); j++)
        inputs[j][field_type] += delta.inputs[j][field_type];

      map<DataTag, long long>::const_iterator it;
      const map<DataTag, long long>& delta_ext = delta.extended[field_type];
      for (it = delta_ext.begin(); it != delta_ext.end(); it++)
          extended[field_type][it->first] += it->second;
    }
  for (auto it = delta.edabits.begin(); it != delta.edabits.end(); it++)
    edabits[it->first] += it->second;
}

DataPositions& DataPositions::operator-=(const DataPositions& delta)
{
  inputs.resize(max(inputs.size(), delta.inputs.size()),
      vector<long long>(N_DATA_FIELD_TYPE));
  for (unsigned int field_type = 0; field_type < N_DATA_FIELD_TYPE;
      field_type++)
    {
      for (unsigned int dtype = 0; dtype < N_DTYPE; dtype++)
        files[field_type][dtype] -= delta.files[field_type][dtype];
      for (unsigned int j = 0; j < delta.inputs.size(); j++)
        inputs[j][field_type] -= delta.inputs[j][field_type];

      map<DataTag, long long>::const_iterator it;
      const map<DataTag, long long> &delta_ext = delta.extended[field_type];
      for (it = delta_ext.begin(); it != delta_ext.end(); it++)
        extended[field_type][it->first] -= it->second;
    }
  for (auto it = delta.edabits.begin(); it != delta.edabits.end(); it++)
    edabits[it->first] -= it->second;
  return *this;
}

DataPositions DataPositions::operator-(const DataPositions& other) const
{
  DataPositions res = *this;
  res -= other;
  return res;
}

void DataPositions::print_cost() const
{
  ifstream file("cost");
  bool print_verbose = file.good();
  double total_cost = 0;
  for (int i = 0; i < N_DATA_FIELD_TYPE; i++)
    {
      if (accumulate(files[i].begin(), files[i].end(), 0) > 0)
        cerr << "  Type " << field_names[i] << endl;
      bool reading_field = true;
      for (int j = 0; j < N_DTYPE; j++)
        process_line(files[i][j], dtype_names[j], file, print_verbose, total_cost, reading_field);

      long long n_inputs = 0;
      stringstream ss;
      ss << " (";
      for (auto& x : inputs)
        {
          n_inputs += x[i];
          ss << x[i] << " ";
        }
      string s = ss.str();
      s.pop_back();
      s += ")";
      process_line(n_inputs, "Input tuples", file, print_verbose, total_cost, reading_field, s);

      for (map<DataTag, long long>::const_iterator it = extended[i].begin();
          it != extended[i].end(); it++)
        {
          cerr.fill(' ');
          cerr << setw(27) << it->second << " " << setw(14) << it->first.get_string() << endl;
        }
    }

  if (total_cost > 0)
    cerr << "Total cost: " << total_cost << endl;

  if (not edabits.empty())
    cerr << "  edaBits" << endl;
  for (auto it = edabits.begin(); it != edabits.end(); it++)
    {
      if (print_verbose)
        cerr << setw(13) << "";
      cerr << "    " << setw(10) << it->second << " of length "
          << it->first.second;
      if (it->first.first)
        cerr << " (strict)";
      cerr << endl;
    }
}

void DataPositions::process_line(long long items_used, const char* name,
    ifstream& file, bool print_verbose, double& total_cost,
    bool& reading_field, string suffix) const
{
  double cost_per_item = 0;
  if (reading_field)
    file >> cost_per_item;
  if (cost_per_item < 0)
    {
      reading_field = false;
      cost_per_item = 0;
    }
  double cost = items_used * cost_per_item;
  total_cost += cost;
  cerr.fill(' ');
  if (items_used)
    {
      cerr << "    ";
      if (print_verbose)
        cerr << setw(10) << cost << " = ";
      cerr << setw(10) << items_used << " " << setw(14)
                      << name;
      if (print_verbose)
        cerr << " @ " << setw(11) << cost_per_item;
      cerr << suffix << endl;
    }
}
