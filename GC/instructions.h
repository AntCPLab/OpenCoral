/*
 * instructions.h
 *
 */

#ifndef GC_INSTRUCTIONS_H_
#define GC_INSTRUCTIONS_H_

#include "Tools/callgrind.h"

#define PROC processor
#define INST instruction
#define MACH processor.machine
#define MD dynamic_memory

#define R0 instruction.get_r(0)
#define REG1 instruction.get_r(1)

#define S0 processor.S[instruction.get_r(0)]
#define PS1 processor.S[instruction.get_r(1)]
#define PS2 processor.S[instruction.get_r(2)]

#define C0 processor.C[instruction.get_r(0)]
#define PC1 processor.C[instruction.get_r(1)]
#define PC2 processor.C[instruction.get_r(2)]

#define I0 processor.I[instruction.get_r(0)]
#define PI1 processor.I[instruction.get_r(1)]
#define PI2 processor.I[instruction.get_r(2)]

#define IMM instruction.get_n()
#define EXTRA instruction.get_start()

#define MSD processor.memories.MS[IMM]
#define MMC processor.memories.MC[IMM]
#define MID MACH->MI[IMM]

#define MSI processor.memories.MS[PI1.get()]
#define MII MACH->MI[PI1.get()]

#define BIT_INSTRUCTIONS \
    X(XORS, PROC.xors(EXTRA)) \
    X(XORCB, C0.xor_(PC1, PC2)) \
    X(XORCBI, C0.xor_(PC1, IMM)) \
    X(ANDRS, T::andrs(PROC, EXTRA)) \
    X(ANDS, T::ands(PROC, EXTRA)) \
    X(INPUTB, T::inputb(PROC, EXTRA)) \
    X(ADDCB, C0 = PC1 + PC2) \
    X(ADDCBI, C0 = PC1 + IMM) \
    X(MULCBI, C0 = PC1 * IMM) \
    X(BITDECS, PROC.bitdecs(EXTRA, S0)) \
    X(BITCOMS, PROC.bitcoms(S0, EXTRA)) \
    X(BITDECC, PROC.bitdecc(EXTRA, C0)) \
    X(SHRCBI, C0 = PC1 >> IMM) \
    X(SHLCBI, C0 = PC1 << IMM) \
    X(LDBITS, S0.load_clear(REG1, IMM)) \
    X(LDMSB, S0 = MSD) \
    X(STMSB, MSD = S0) \
    X(LDMCB, C0 = MMC) \
    X(STMCB, MMC = C0) \
    X(MOVSB, S0 = PS1) \
    X(TRANS, T::trans(PROC, IMM, EXTRA)) \
    X(BITB, PROC.random_bit(S0)) \
    X(REVEAL, PS1.reveal(IMM, C0)) \
    X(PRINTREGSIGNED, PROC.print_reg_signed(IMM, C0)) \
    X(PRINTREGB, PROC.print_reg(R0, IMM)) \
    X(PRINTREGPLAINB, PROC.print_reg_plain(C0)) \
    X(PRINTFLOATPLAINB, PROC.print_float(EXTRA)) \
    X(CONDPRINTSTRB, if(C0.get()) PROC.print_str(IMM)) \

#define COMBI_INSTRUCTIONS BIT_INSTRUCTIONS \
    X(ANDM, S0 = PS1 & PC2) \
    X(LDMSBI, S0 = processor.memories.MS[Proc.read_Ci(REG1)]) \
    X(STMSBI, processor.memories.MS[Proc.read_Ci(REG1)] = S0) \
    X(CONVSINT, S0.load_clear(IMM, Proc.read_Ci(REG1))) \
    X(CONVCINT, C0 = Proc.read_Ci(REG1)) \
    X(CONVCBIT, Proc.write_Ci(R0, PC1.get())) \
    X(DABIT, Proc.dabit(INST)) \

#define GC_INSTRUCTIONS \
    X(LDMSBI, S0 = MSI) \
    X(STMSBI, MSI = S0) \
    X(LDMSD, PROC.load_dynamic_direct(EXTRA, MD)) \
    X(STMSD, PROC.store_dynamic_direct(EXTRA, MD)) \
    X(LDMSDI, PROC.load_dynamic_indirect(EXTRA, MD)) \
    X(STMSDI, PROC.store_dynamic_indirect(EXTRA, MD)) \
    X(STMSDCI, PROC.store_clear_in_dynamic(EXTRA, MD)) \
    X(CONVSINT, S0.load_clear(IMM, PI1)) \
    X(CONVCINT, C0 = PI1) \
    X(CONVCBIT, T::convcbit(I0, PC1)) \
    X(PRINTCHR, PROC.print_chr(IMM)) \
    X(PRINTSTR, PROC.print_str(IMM)) \
    X(PRINTFLOATPREC, PROC.print_float_prec(IMM)) \
    X(LDINT, I0 = int(IMM)) \
    X(ADDINT, I0 = PI1 + PI2) \
    X(SUBINT, I0 = PI1 - PI2) \
    X(MULINT, I0 = PI1 * PI2) \
    X(DIVINT, I0 = PI1 / PI2) \
    X(JMP, PROC.PC += IMM) \
    X(JMPNZ, if (I0 != 0) PROC.PC += IMM) \
    X(JMPEQZ, if (I0 == 0) PROC.PC += IMM) \
    X(EQZC, I0 = PI1 == 0) \
    X(LTZC, I0 = PI1 < 0) \
    X(LTC, I0 = PI1 < PI2) \
    X(GTC, I0 = PI1 > PI2) \
    X(EQC, I0 = PI1 == PI2) \
    X(JMPI, PROC.PC += I0) \
    X(LDMINT, I0 = MID) \
    X(STMINT, MID = I0) \
    X(LDMINTI, I0 = MII) \
    X(STMINTI, MII = I0) \
    X(PUSHINT, PROC.pushi(I0.get())) \
    X(POPINT, long x; PROC.popi(x); I0 = x) \
    X(MOVINT, I0 = PI1) \
    X(BITDECINT, PROC.bitdecint(EXTRA, I0)) \
    X(LDARG, I0 = PROC.get_arg()) \
    X(STARG, PROC.set_arg(I0.get())) \
    X(TIME, MACH->time()) \
    X(START, MACH->start(IMM)) \
    X(STOP, MACH->stop(IMM)) \
    X(GLDMS, ) \
    X(GLDMC, ) \
    X(LDMS, ) \
    X(LDMC, ) \
    X(PRINTINT, S0.out << I0) \
    X(STARTGRIND, CALLGRIND_START_INSTRUMENTATION) \
    X(STOPGRIND, CALLGRIND_STOP_INSTRUMENTATION) \
    X(RUN_TAPE, MACH->run_tape(R0, IMM, REG1)) \
    X(JOIN_TAPE, MACH->join_tape(R0)) \

#define INSTRUCTIONS BIT_INSTRUCTIONS GC_INSTRUCTIONS

#endif /* GC_INSTRUCTIONS_H_ */
