/*
 * NoShare.h
 *
 */

#ifndef GC_NOSHARE_H_
#define GC_NOSHARE_H_

#include "BMR/Register.h"
#include "Processor/DummyProtocol.h"

namespace GC
{

class NoValue
{
public:
    const static int n_bits = 0;

    static bool allows(Dtype)
    {
        return false;
    }

    static int size()
    {
        return 0;
    }

    static void fail()
    {
        throw runtime_error("VM does not support binary circuits");
    }

    void assign(const char*) { fail(); }

    int get() const { fail(); return 0; }
};

class NoShare : public Phase
{
public:
    typedef DummyMC<NoShare> MC;
    typedef DummyProtocol Protocol;
    typedef NotImplementedInput<NoShare> Input;
    typedef DummyLivePrep<NoShare> LivePrep;
    typedef DummyMC<NoShare> MAC_Check;

    typedef NoValue open_type;
    typedef NoValue clear;
    typedef NoValue mac_key_type;

    typedef NoShare bit_type;
    typedef NoShare part_type;

    static const bool needs_ot = false;

    static MC* new_mc(mac_key_type)
    {
        return new MC;
    }

    template<class T>
    static void generate_mac_key(mac_key_type, T)
    {
    }

    static DataFieldType field_type()
    {
        throw not_implemented();
    }

    static string type_short()
    {
        return "";
    }

    static string type_string()
    {
        return "no";
    }

    static int size()
    {
        return 0;
    }

    static void fail()
    {
        NoValue::fail();
    }

    static void inputb(Processor<NoShare>&, const vector<int>&) { fail(); }

    static void input(Processor<NoShare>&, InputArgs&) { fail(); }
    static void trans(Processor<NoShare>&, Integer, const vector<int>&) { fail(); }

    NoShare() {}

    NoShare(int) { fail(); }

    void load_clear(Integer, Integer) { fail(); }
    void random_bit() { fail(); }
    void and_(int, NoShare&, NoShare&, bool) { fail(); }
    void xor_(int, NoShare&, NoShare&) { fail(); }
    void bitdec(vector<NoShare>&, const vector<int>&) const { fail(); }
    void bitcom(vector<NoShare>&, const vector<int>&) const { fail(); }
    void reveal(Integer, Integer) { fail(); }

    void assign(const char*) { fail(); }

    NoShare operator&(const Clear&) const { fail(); return {}; }

    NoShare operator<<(int) const { fail(); return {}; }
    void operator^=(NoShare) { fail(); }

    NoShare operator+(const NoShare&) const { fail(); return {}; }
};

} /* namespace GC */

#endif /* GC_NOSHARE_H_ */
