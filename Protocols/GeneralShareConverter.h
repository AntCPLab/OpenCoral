#ifndef PROTOCOLS_GENERALSHARECONVERTER_H_
#define PROTOCOLS_GENERALSHARECONVERTER_H_

#include <memory>
#include "Protocols/ProtocolGlobalInit.h"

template<class SrcType, class DstType>
class GeneralShareConverter {
    typedef typename SrcType::MAC_Check SrcMC;
    typedef typename SrcType::LivePrep SrcLivePrep;
    typedef typename DstType::MAC_Check DstMC;

public:
    SrcMC* src_mc;
    SrcLivePrep* src_prep;

    /**
     * Assuming ShareThread singleton has been set, we take out the MC and prep from there.
    */
    GeneralShareConverter(Player& P) {
        GC::ShareThread<SrcType>& thread = GC::ShareThread<SrcType>::s();
        this->src_mc = thread.MC;
        this->src_prep = dynamic_cast<SrcLivePrep*>(&thread.DataF);
    }

    ~GeneralShareConverter() {
    }

    SrcLivePrep* get_src_prep() const {
        return src_prep;
    }

    SrcMC* get_src_mc() const {
        return src_mc;
    }

    void convert(std::vector<DstType>& dst_shares, const std::vector<SrcType>& src_shares);
};

#endif