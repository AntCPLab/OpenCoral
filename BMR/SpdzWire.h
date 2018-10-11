// (C) 2018 University of Bristol, Bar-Ilan University. See License.txt

/*
 * SpdzWire.h
 *
 */

#ifndef BMR_SPDZWIRE_H_
#define BMR_SPDZWIRE_H_

#include "Math/Share.h"
#include "Key.h"

class SpdzWire
{
public:
	Share<gf2n> mask;
	Key my_keys[2];

	SpdzWire();
	void pack(octetStream& os) const;
	void unpack(octetStream& os, size_t wanted_size);
};

#endif /* BMR_SPDZWIRE_H_ */
