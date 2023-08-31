/*
 * dabit.h
 *
 */

#ifndef PROTOCOLS_DABIT_H_
#define PROTOCOLS_DABIT_H_

#include <array>
using namespace std;

template<class T>
class dabit : public pair<T, typename T::bit_type::part_type>
{
    typedef pair<T, typename T::bit_type::part_type> super;

public:
    typedef typename T::bit_type::part_type bit_type;

    static int size()
    {
        return T::size() + bit_type::size();
    }

    static string type_string()
    {
        return T::type_string();
    }

    static void specification(octetStream& os)
    {
        T::specification(os);
    }

    dabit()
    {
    }

    dabit(const T& a, const bit_type& b) :
            super(a, b)
    {
    }

    void assign(const char* buffer)
    {
        this->first.assign(buffer);
        this->second.assign(buffer + T::size());
    }

    void output(ostream& out, bool human)
    {
        this->first.output(out, human);
        this->second.output(out, human);
    }
};

template<class T>
class dabitpack: public pair<FixedVector<T, T::bit_type::default_length>, typename T::bit_type> {
    typedef pair<FixedVector<T, T::bit_type::default_length>, typename T::bit_type> super;
public:
    static const int MAX_SIZE = super::first_type::MAX_SIZE;

    dabitpack() {}

    dabitpack(const typename super::first_type& a, const typename super::second_type& b) : super(a, b) {
    }


    void push_a(const T& x)
    {
        this->first.push_back(x);
    }

    void input(ifstream& s)
    {
        char buffer[MAX_SIZE * T::size()];
        s.read(buffer, MAX_SIZE * T::size());
        for (int i = 0; i < MAX_SIZE; i++)
        {
            T x;
            x.assign(buffer + i * T::size());
            push_a(x);
        }
        size_t bsize = T::bit_type::part_type::size();
        char bbuffer[bsize];
        s.read(bbuffer, bsize);
        this->second.assign(bbuffer);
    }
};

template<class T>
using dabit_t = typename std::conditional<T::bit_type::tight_packed, dabitpack<T>, dabit<T>>::type;

template<class T>
dabitpack<T>& operator+=(dabitpack<T>& x, const dabitpack<T>& y)
{
    x.first += y.first;
    x.second ^= y.second;
    return x;
}



#endif /* PROTOCOLS_DABIT_H_ */
