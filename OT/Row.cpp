#include "OT/Row.h"
#include "Exceptions/Exceptions.h"

template<class T>
bool Row<T>::operator ==(const Row<T>& other) const
{
    return rows == other.rows;
}

template<class T>
Row<T>& Row<T>::operator +=(const Row<T>& other)
{
    if (rows.size() != other.size()) {
        throw invalid_length();
    }
    for (size_t i = 0; i < this->size(); i++)
        rows[i] += other.rows[i];
    return *this;
}

template<class T>
Row<T>& Row<T>::operator -=(const Row<T>& other)
{
    if (rows.size() != other.size()) {
        throw invalid_length();
    }
    for (size_t i = 0; i < this->size(); i++)
        rows[i] -= other.rows[i];
    return *this;
}

template<class T>
Row<T>& Row<T>::operator *=(const T& other)
{
    for (size_t i = 0; i < this->size(); i++)
        rows[i] = rows[i] * other;
    return *this;
}

template<class T>
Row<T> Row<T>::operator *(const T& other)
{
    Row<T> res = *this;
    res *= other;
    return res;
}

template<class T>
Row<T> Row<T>::operator +(const Row<T>& other)
{
    Row<T> res = other;
    res += *this;
    return res;
}

template<class T>
Row<T> Row<T>::operator -(const Row<T>& other)
{
    Row<T> res = *this;
    res-=other;
    return res;
}

template<class T>
void Row<T>::randomize(PRNG& G)
{
    for (size_t i = 0; i < this->size(); i++)
        rows[i].randomize(G);
}

template<class T>
Row<T> Row<T>::operator<<(int i) const {
    if (i >= T::size() * 8) {
        throw invalid_params();
    }
    Row<T> res = *this;
    for (size_t j = 0; j < this->size(); j++)
        res.rows[j] = res.rows[j] << i;
    return res;
}

template<class T>
void Row<T>::pack(octetStream& o) const
{
    o.store(this->size());
    for (size_t i = 0; i < this->size(); i++)
        rows[i].pack(o);
}

template<class T>
void Row<T>::unpack(octetStream& o)
{
    size_t size;
    o.get(size);
    this->rows.resize(size);
    for (size_t i = 0; i < this->size(); i++)
        rows[i].unpack(o);
}

template <class V>
ostream& operator<<(ostream& o, const Row<V>& x)
{
    for (size_t i = 0; i < x.size(); ++i)
        o << x.rows[i] << " | ";
    return o;
}

template class Row<Z2<96>>;
template ostream& operator<<(ostream& o, const Row<Z2<96>>& x);
template class Row<Z2<192>>;
template ostream& operator<<(ostream& o, const Row<Z2<192>>& x);
template class Row<Z2<256>>;
template ostream& operator<<(ostream& o, const Row<Z2<256>>& x);
