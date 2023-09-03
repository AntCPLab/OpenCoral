/*
 * Clear.h
 *
 */

#ifndef GC_CLEAR_H_
#define GC_CLEAR_H_

#include "Math/Integer.h"

namespace GC
{

#ifdef RMFE_UNIT
#define GC_UNIT 12
#else
#define GC_UNIT 64
#endif

class Clear : public Integer
{
public:
    static const int N_BITS = GC_UNIT;

    static string type_string() { return "clear"; }

    Clear() : Integer() {}
    Clear(long a) : Integer(a) {}
    Clear(const IntBase& x) { IntBase::operator=(x); }

    void xor_(const Clear& x, const Clear& y) { a = x.a ^ y.a; }
    void xor_(const Clear& x, long y) { a = x.a ^ y; }
};

} /* namespace GC */

#endif /* GC_CLEAR_H_ */
