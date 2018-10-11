/*
 * YaoGarbleMaster.h
 *
 */

#ifndef YAO_YAOGARBLEMASTER_H_
#define YAO_YAOGARBLEMASTER_H_

#include "GC/ThreadMaster.h"
#include "GC/Secret.h"
#include "YaoGarbleWire.h"

using namespace GC;

class YaoGarbleMaster : public GC::ThreadMaster<GC::Secret<YaoGarbleWire>>
{
public:
    bool continuous;
    int threshold;
    Key delta;

    YaoGarbleMaster(bool continuous, int threshold = 1024);

    Thread<Secret<YaoGarbleWire>>* new_thread(int i);
};

#endif /* YAO_YAOGARBLEMASTER_H_ */
