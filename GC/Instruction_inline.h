// (C) 2018 University of Bristol, Bar-Ilan University. See License.txt

/*
 * Instruction_inline.h
 *
 */

#ifndef GC_INSTRUCTION_INLINE_H_
#define GC_INSTRUCTION_INLINE_H_

#ifdef MAX_INLINE
#define MAYBE_INLINE inline
#else
#define MAYBE_INLINE
#endif

namespace GC {

#include "instructions.h"

template <class T>
inline bool fallback_code(const Instruction<T>& instruction, Processor<T>& processor)
{
    (void)processor;
    cout << "Undefined instruction " << showbase << hex
            << instruction.get_opcode() << endl << dec;
    return true;
}

template <class T>
MAYBE_INLINE bool Instruction<T>::execute(Processor<T>& processor) const
{
#ifdef DEBUG_OPS
    cout << typeid(T).name() << " ";
    cout << "pc " << processor.PC << " op " << hex << showbase << opcode << " "
            << dec << noshowbase << r[0];
    if (CommonParty::singleton)
        cout << ", " << CommonParty::s().get_reg_size() << " regs ";
    if (ProgramParty::singleton)
        ProgramParty::s().print_input_size<T>();
    cout << endl;
#endif
    const Instruction& instruction = *this;
    switch (opcode)
    {
#define X(NAME, CODE) case NAME: CODE; return true;
    INSTRUCTIONS
#undef X
    default:
        return fallback_code(*this, processor);
    }
}

} /* namespace GC */

#endif /* GC_INSTRUCTION_INLINE_H_ */
