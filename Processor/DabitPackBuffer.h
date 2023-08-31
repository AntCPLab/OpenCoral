/*
 * DabitPackBuffer.h
 *
 */

#ifndef PROCESSOR_DABITPACKBUFFER_H_
#define PROCESSOR_DABITPACKBUFFER_H_

#include "Tools/Buffer.h"

template<class T>
class DabitPackBuffer : public BufferOwner<T, T>
{

    int element_length()
    {
        return -1;
    }

public:
    DabitPackBuffer()
    {
    }

    dabitpack<T> read()
    {
        if (not BufferBase::file)
        {
            if (this->open()->fail())
                throw runtime_error(
                        "error opening " + this->filename
                                + ", have you generated daBitPacks, "
                                        "for example by running "
                                        "'./Fake-Offline.x"
                                + " ...'?");
        }

        assert(BufferBase::file);
        auto& buffer = *BufferBase::file;
        if (buffer.peek() == EOF)
        {
            this->try_rewind();
        }

        dabitpack<T> db;
        db.input(buffer);
        if (buffer.fail())
            throw runtime_error("error reading daBitPacks");
        return db;
    }
};

#endif /* PROCESSOR_DABITPACKBUFFER_H_ */
