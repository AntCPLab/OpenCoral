/*
 * ValueInterface.h
 *
 */

#ifndef MATH_VALUEINTERFACE_H_
#define MATH_VALUEINTERFACE_H_

class ValueInterface
{
public:
    static int t() { return 0; }

    template<class T>
    static void init(bool mont = true) { (void) mont; }
    static void init_default(int l) { (void) l; }

    static void read_setup(int nparties, int lg2p, int gf2ndegree);

    void normalize() {}
};

#endif /* MATH_VALUEINTERFACE_H_ */
