/*
 * Element.h
 *
 */

#ifndef ECDSA_P256ELEMENT_H_
#define ECDSA_P256ELEMENT_H_

#include <cryptopp/eccrypto.h>

#include "Math/gfp.h"

class P256Element : public ValueInterface
{
public:
    typedef gfp_<2, 4> Scalar;

private:
    static CryptoPP::DL_GroupParameters_EC<CryptoPP::ECP> params;
    static CryptoPP::ECP curve;

    CryptoPP::ECP::Point point;

    static CryptoPP::Integer convert(const Scalar& other);

public:
    typedef void next;
    typedef void Square;

    static int size() { return 0; }
    static string type_string() { return "P256"; }

    static void init();

    P256Element();
    P256Element(const Scalar& other);
    P256Element(word other);

    void check();

    const CryptoPP::ECP::Point& get() const { return point; }
//    const unsigned char* get() const { return a; }

    Scalar x() const;

    P256Element operator+(const P256Element& other) const;
    P256Element operator-(const P256Element& other) const;
    P256Element operator*(const Scalar& other) const;

    P256Element& operator+=(const P256Element& other);
    P256Element& operator/=(const Scalar& other);

    bool operator==(const P256Element& other) const;
    bool operator!=(const P256Element& other) const;

    void assign_zero() { *this = 0; }
    bool is_zero() { return *this == 0; }
    void add(const P256Element& x, const P256Element& y) { *this = x + y; }
    void sub(const P256Element& x, const P256Element& y) { *this = x - y; }
    void mul(const P256Element& x, const Scalar& y) { *this = x * y; }
    void mul(const Scalar& x, const P256Element& y) { *this = y * x; }
    void add(octetStream& os) { *this += os.get<P256Element>(); }

    void pack(octetStream& os) const;
    void unpack(octetStream& os);
};

ostream& operator<<(ostream& s, const P256Element& x);

#endif /* ECDSA_P256ELEMENT_H_ */
