/*
 * P256Element.cpp
 *
 */

#include "P256Element.h"

#include "Math/gfp.hpp"

#include <cryptopp/oids.h>
#include <cryptopp/misc.h>

CryptoPP::DL_GroupParameters_EC<CryptoPP::ECP> P256Element::params;
CryptoPP::ECP P256Element::curve;

void P256Element::init()
{
    params = CryptoPP::DL_GroupParameters_EC<CryptoPP::ECP>(CryptoPP::ASN1::secp256k1());
    curve = params.GetCurve();
    auto mod = params.GetSubgroupOrder();
    Scalar::init_field(CryptoPP::IntToString(mod).c_str(), false);
}

CryptoPP::Integer P256Element::convert(const Scalar& other)
{
    return CryptoPP::Integer((unsigned char*) other.get_ptr(), other.size(),
            CryptoPP::Integer::UNSIGNED, CryptoPP::LITTLE_ENDIAN_ORDER);
}

P256Element::P256Element()
{
    point = curve.Identity();
}

P256Element::P256Element(const Scalar& other)
{
    point = params.ExponentiateBase(convert(other));
}

P256Element::P256Element(word other)
{
    point = params.ExponentiateBase(other);
}

void P256Element::check()
{
    curve.VerifyPoint(point);
}

P256Element::Scalar P256Element::x() const
{
    return bigint(IntToString(point.x));
}

P256Element P256Element::operator +(const P256Element& other) const
{
    P256Element res;
    res.point = curve.Add(point, other.point);
    return res;
}

P256Element P256Element::operator -(const P256Element& other) const
{
    P256Element res;
    res.point = curve.Add(point, curve.Inverse(other.point));
    return res;
}

P256Element P256Element::operator *(const Scalar& other) const
{
    P256Element res;
    res.point = curve.Multiply(convert(other), point);
    return res;
}

P256Element& P256Element::operator +=(const P256Element& other)
{
    *this = *this + other;
    return *this;
}

P256Element& P256Element::operator /=(const Scalar& other)
{
    auto tmp = other;
    tmp.invert();
    *this = *this * tmp;
    return *this;
}

bool P256Element::operator ==(const P256Element& other) const
{
    return point == other.point;
}

bool P256Element::operator !=(const P256Element& other) const
{
    return not (*this == other);
}

void P256Element::pack(octetStream& os) const
{
    os.serialize(point.identity);
    size_t l;
    l = point.x.MinEncodedSize();
    os.serialize(l);
    point.x.Encode(os.append(l), l);
    l = point.y.MinEncodedSize();
    os.serialize(l);
    point.y.Encode(os.append(l), l);
}

void P256Element::unpack(octetStream& os)
{
    os.unserialize(point.identity);
    size_t l;
    os.unserialize(l);
    point.x.Decode(os.consume(l), l);
    os.unserialize(l);
    point.y.Decode(os.consume(l), l);
}

ostream& operator <<(ostream& s, const P256Element& x)
{
    auto& point = x.get();
    if (point.identity)
        s << "ID" << endl;
    else
        s << point.x << "," << point.y;
    return s;
}
