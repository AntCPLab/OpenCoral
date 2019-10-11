/*
 * RandomPrep.h
 *
 */

#ifndef PROTOCOLS_RANDOMPREP_H_
#define PROTOCOLS_RANDOMPREP_H_

template<class T>
class RandomPrep
{
public:
    virtual ~RandomPrep() {}

    virtual T get_random() = 0;
};

#endif /* PROTOCOLS_RANDOMPREP_H_ */
