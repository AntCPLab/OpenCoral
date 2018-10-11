// (C) 2018 University of Bristol, Bar-Ilan University. See License.txt

/*
 * SpdzWire.cpp
 *
 */

#include "SpdzWire.h"

SpdzWire::SpdzWire()
{

}

void SpdzWire::pack(octetStream& os) const
{
	mask.pack(os);
	os.serialize(my_keys);
}

void SpdzWire::unpack(octetStream& os, size_t wanted_size)
{
	(void)wanted_size;
	mask.unpack(os);
	os.unserialize(my_keys);
}
