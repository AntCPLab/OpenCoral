/*
 * EdabitPackBuffer.h
 *
 */

#ifndef PROCESSOR_EDABITPACKBUFFER_H_
#define PROCESSOR_EDABITPACKBUFFER_H_

#include "Tools/Buffer.h"

template<class T>
class EdabitPackBuffer : public BufferOwner<T, T>
{
    int n_bits;

    int element_length()
    {
        return -1;
    }

public:
    EdabitPackBuffer(int n_bits = 0) :
            n_bits(n_bits)
    {
    }

    edabitpack<T> read()
    {
        if (not BufferBase::file)
        {
            if (this->open()->fail())
                throw runtime_error(
                        "error opening " + this->filename
                                + ", have you generated edaBitPacks, "
                                        "for example by running "
                                        "'./Fake-Offline.x -e "
                                + to_string(n_bits) + " ...'?");
        }

        assert(BufferBase::file);
        auto& buffer = *BufferBase::file;
        if (buffer.peek() == EOF)
        {
            this->try_rewind();
        }

        edabitpack<T> eb;
        eb.input(n_bits, buffer);
        if (buffer.fail())
            throw runtime_error("error reading edaBitPacks");
        return eb;
    }
};

#endif /* PROCESSOR_EDABITPACKBUFFER_H_ */
