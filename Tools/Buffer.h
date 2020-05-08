/*
 * Buffer.h
 *
 */

#ifndef TOOLS_BUFFER_H_
#define TOOLS_BUFFER_H_

#include <fstream>
#include <iostream>
using namespace std;

#include "Math/field_types.h"
#include "Tools/time-func.h"

#ifndef BUFFER_SIZE
#define BUFFER_SIZE 101
#endif

class BufferBase
{
protected:
    static bool rewind;

    ifstream* file;
    int next;
    string data_type;
    string field_type;
    Timer timer;
    int tuple_length;
    string filename;

public:
    bool eof;

    BufferBase() : file(0), next(BUFFER_SIZE),
            tuple_length(-1), eof(false) {}
    ~BufferBase() {}
    virtual ifstream* open() = 0;
    void setup(ifstream* f, int length, string filename, const char* type = "",
            const char* field = "");
    void seekg(int pos);
    bool is_up() { return file != 0; }
    void try_rewind();
    void prune();
    void purge();
};


template <class T, class U>
class Buffer : public BufferBase
{
    T buffer[BUFFER_SIZE];

    void read(char* read_buffer);

public:
    virtual ~Buffer();
    virtual ifstream* open();
    void input(U& a);
    void fill_buffer();
};

template<class U, class V>
class BufferOwner : public Buffer<U, V>
{
    ifstream* file;

public:
    BufferOwner() :
            file(0)
    {
    }

    ifstream* open()
    {
        file = new ifstream(this->filename, ios::in | ios::binary);
        return file;
    }

    void setup(string filename, int tuple_length, const char* data_type = "")
    {
        Buffer<U, V>::setup(file, tuple_length, filename, data_type, U::type_string().c_str());
    }

    void close()
    {
        if (file)
            delete file;
        file = 0;
    }
};

template<class T, class U>
inline Buffer<T, U>::~Buffer()
{
    if (timer.elapsed() && data_type.size())
        cerr << T::type_string() << " " << data_type << " reading: "
                << timer.elapsed() << endl;
}

template<class T, class U>
inline void Buffer<T, U>::fill_buffer()
{
  if (T::size() == sizeof(T))
    {
      // read directly
      read((char*)buffer);
    }
  else
    {
      char read_buffer[BUFFER_SIZE * T::size()];
      read(read_buffer);
      //memset(buffer, 0, sizeof(buffer));
      for (int i = 0; i < BUFFER_SIZE; i++)
        buffer[i].assign(&read_buffer[i*T::size()]);
    }
}

template<class T, class U>
ifstream* Buffer<T, U>::open()
{
    throw IO_Error(T::type_string() + " buffer not set up");
}

template<class T, class U>
inline void Buffer<T, U>::read(char* read_buffer)
{
    int size_in_bytes = T::size() * BUFFER_SIZE;
    int n_read = 0;
    timer.start();
    if (not file)
        file = open();
    do
    {
        file->read(read_buffer + n_read, size_in_bytes - n_read);
        n_read += file->gcount();
        if (file->eof())
        {
            try_rewind();
        }
        if (file->fail())
          {
            stringstream ss;
            ss << "IO problem when buffering " << T::type_string();
            if (data_type.size())
              ss << " " << data_type;
            ss << " from " << filename;
            throw file_error(ss.str());
          }
    }
    while (n_read < size_in_bytes);
    timer.stop();
}

template <class T, class U>
inline void Buffer<T,U>::input(U& a)
{
    if (next == BUFFER_SIZE)
    {
        fill_buffer();
        next = 0;
    }

    a = buffer[next];
    next++;
}

#endif /* TOOLS_BUFFER_H_ */
