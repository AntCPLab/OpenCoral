/*
 * Buffer.h
 *
 */

#ifndef PROCESSOR_BUFFER_H_
#define PROCESSOR_BUFFER_H_

#include <fstream>
using namespace std;

#include "Math/Share.h"
#include "Math/field_types.h"
#include "Tools/time-func.h"
#include "config.h"

#ifndef BUFFER_SIZE
#define BUFFER_SIZE 101
#endif


class BufferBase
{
protected:
    static bool rewind;

    ifstream* file;
    int next;
    const char* data_type;
    const char* field_type;
    Timer timer;
    int tuple_length;
    string filename;

public:
    bool eof;

    BufferBase() : file(0), next(BUFFER_SIZE), data_type(0), field_type(0),
            tuple_length(-1), eof(false) {}
    void setup(ifstream* f, int length, string filename, const char* type = 0,
            const char* field = 0);
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
    ~Buffer();
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

    void setup(string filename, int tuple_length, const char* data_type = 0)
    {
        file = new ifstream(filename, ios::in | ios::binary);
        Buffer<U, V>::setup(file, tuple_length, filename, data_type, U::type_string().c_str());
    }

    void close()
    {
        if (file)
            delete file;
        file = 0;
    }
};

#endif /* PROCESSOR_BUFFER_H_ */
