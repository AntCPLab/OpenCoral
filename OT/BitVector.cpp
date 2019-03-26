// (C) 2018 University of Bristol. See License.txt


#include "OT/BitVector.h"
#include "OT/Rectangle.h"
#include "Tools/random.h"
#include "Tools/octetStream.h"
#include "Math/gf2n.h"
#include "Math/gfp.h"
#include "Math/Z2k.h"

#include <fstream>
#include <assert.h>

void BitVector::resize_zero(size_t new_nbits)
{
    size_t old_nbytes = nbytes;
    resize(new_nbits);
    avx_memzero(bytes + old_nbytes, nbytes - old_nbytes);
}

const void* BitVector::get_ptr_to_byte(size_t i, size_t block_size) const
{
    assert(i + block_size <= nbytes);
    return bytes + i * block_size;
}

const void* BitVector::get_ptr_to_bit(size_t i, size_t block_size) const
{
    assert((i * block_size) % 8 == 0);
    assert((i + 1) * block_size <= nbits);
    return bytes + block_size * i / 8;
}

void BitVector::randomize(PRNG& G)
{
    G.get_octets(bytes, nbytes);
}

template<>
void BitVector::randomize_blocks<gf2n>(PRNG& G)
{
    randomize(G);
}

template<>
void BitVector::randomize_blocks<gfp>(PRNG& G)
{
    gfp tmp;
    for (size_t i = 0; i < (nbits / 128); i++)
    {
        tmp.randomize(G);
        for (int j = 0; j < 2; j++)
            ((mp_limb_t*)bytes)[2*i+j] = tmp.get().get_limb(j);
    }
}

template<>
void BitVector::randomize_blocks<Z2<160> >(PRNG& G)
{
    randomize_blocks<gf2n>(G);
}

void BitVector::randomize_at(int a, int nb, PRNG& G)
{
    if (nb < 1)
        throw invalid_length();
    G.get_octets(bytes + a, nb);
}

/*
 */

void BitVector::output(ostream& s,bool human) const
{
    if (human)
    {
        s << nbits << " " << hex;
        for (unsigned int i = 0; i < nbytes; i++)
        {
            s << int(bytes[i]) << " ";
        }
        s << dec << endl;
    }
    else
    {
        int len = nbits;
        s.write((char*) &len, sizeof(int));
        s.write((char*) bytes, nbytes);
    }
}


void BitVector::input(istream& s,bool human)
{
    if (s.peek() == EOF)
    {
        if (s.tellg() == 0)
        {
            cout << "IO problem. Empty file?" << endl;
            throw file_error();
        }
      throw end_of_file();
    }
    int len;
    if (human)
    {
        s >> len >> hex;
        resize(len);
        for (size_t i = 0; i < nbytes; i++)
        {
            s >> bytes[i];
        }
        s >> dec;
    }
    else
    {
        s.read((char*) &len, sizeof(int));
        resize(len);
        s.read((char*) bytes, nbytes);
    }
}


void BitVector::pack(octetStream& o) const
{
    o.store(nbytes);
    o.append((octet*)bytes, nbytes);
}

void BitVector::unpack(octetStream& o)
{
    o.get(nbytes);
    o.consume((octet*)bytes, nbytes);
}


