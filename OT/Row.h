#ifndef OT_ROW_H_
#define OT_ROW_H_

#include "Math/Z2k.h"
#include "Math/gf2nlong.h"
#define VOLE_HASH_SIZE crypto_generichash_BYTES

template <class T>
class Row
{
public:

    vector<T> rows;

    Row(int size) : rows(size) {}

    Row() : rows() {}

    Row(const vector<T>& _rows) : rows(_rows) {}

    bool operator==(const Row<T>& other) const;
    bool operator!=(const Row<T>& other) const { return not (*this == other); }

    Row<T>& operator+=(const Row<T>& other);
    Row<T>& operator-=(const Row<T>& other);
    
    Row<T>& operator*=(const T& other);

    Row<T> operator*(const T& other);
    Row<T> operator+(const Row<T> & other);
    Row<T> operator-(const Row<T> & other);

    Row<T> operator<<(int i) const;

    // fine, since elements in vector are allocated contiguously
    const void* get_ptr() const { return rows[0].get_ptr(); }
    
    void randomize(PRNG& G);

    void pack(octetStream& o) const;
    void unpack(octetStream& o);

    size_t size() const { return rows.size(); }

    template <class V>
    friend ostream& operator<<(ostream& o, const Row<V>& x);
};

template <int K>
using Z2kRow = Row<Z2<K>>;

#endif /* OT_ROW_H_ */
