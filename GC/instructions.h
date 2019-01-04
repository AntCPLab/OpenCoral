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

#define R0 instruction.get_r(0)
#define R1 instruction.get_r(1)
#define R2 instruction.get_r(2)

#define S0 processor.S[instruction.get_r(0)]
#define PS1 processor.S[instruction.get_r(1)]
#define PS2 processor.S[instruction.get_r(2)]

#define C0 processor.C[instruction.get_r(0)]
#define C1 processor.C[instruction.get_r(1)]
#define C2 processor.C[instruction.get_r(2)]

#define I0 processor.I[instruction.get_r(0)]
#define I1 processor.I[instruction.get_r(1)]
#define I2 processor.I[instruction.get_r(2)]

#define IMM instruction.get_n()
#define EXTRA instruction.get_start()

#define MSD MACH.MS[IMM]
#define MMC MACH.MC[IMM]
#define MID MACH.MI[IMM]

#define MSI MACH.MS[I1.get()]
#define MII MACH.MI[I1.get()]

#define INSTRUCTIONS \
    X(XORS, PROC.xors(EXTRA)) \
    X(XORC, C0.xor_(C1, C2)) \
    X(XORCI, C0.xor_(C1, IMM)) \
    X(ANDRS, T::andrs(PROC, EXTRA)) \
    X(ANDS, T::ands(PROC, EXTRA)) \
    X(INPUTB, T::inputb(PROC, EXTRA)) \
    X(ADDC, C0 = C1 + C2) \
    X(ADDCI, C0 = C1 + IMM) \
    X(MULCI, C0 = C1 * IMM) \
    X(BITDECS, PROC.bitdecs(EXTRA, S0)) \
    X(BITCOMS, PROC.bitcoms(S0, EXTRA)) \
    X(BITDECC, PROC.bitdecc(EXTRA, C0)) \
    X(BITDECINT, PROC.bitdecint(EXTRA, I0)) \
    X(SHRCI, C0 = C1 >> IMM) \
    X(SHLCI, C0 = C1 << IMM) \
    X(LDBITS, S0.load(R1, IMM)) \
    X(LDMS, S0 = MSD) \
    X(STMS, MSD = S0) \
    X(LDMSI, S0 = MSI) \
    X(STMSI, MSI = S0) \
    X(LDMC, C0 = MMC) \
    X(STMC, MMC = C0) \
    X(LDMSD, PROC.load_dynamic_direct(EXTRA)) \
    X(STMSD, PROC.store_dynamic_direct(EXTRA)) \
    X(LDMSDI, PROC.load_dynamic_indirect(EXTRA)) \
    X(STMSDI, PROC.store_dynamic_indirect(EXTRA)) \
    X(STMSDCI, PROC.store_clear_in_dynamic(EXTRA)) \
    X(CONVSINT, S0.load(IMM, I1)) \
    X(CONVCINT, C0 = I1) \
    X(MOVS, S0 = PS1) \
    X(TRANS, T::trans(PROC, IMM, EXTRA)) \
    X(BIT, PROC.random_bit(S0)) \
    X(REVEAL, PS1.reveal(C0)) \
    X(PRINTREG, PROC.print_reg(R0, IMM)) \
    X(PRINTREGPLAIN, PROC.print_reg_plain(C0)) \
    X(PRINTREGSIGNED, PROC.print_reg_signed(IMM, C0)) \
    X(PRINTCHR, PROC.print_chr(IMM)) \
    X(PRINTSTR, PROC.print_str(IMM)) \
    X(PRINTFLOATPLAIN, PROC.print_float(EXTRA)) \
    X(PRINTFLOATPREC, PROC.print_float_prec(IMM)) \
    X(CONDPRINTSTR, if(C0.get()) PROC.print_str(IMM)) \
    X(LDINT, I0 = int(IMM)) \
    X(ADDINT, I0 = I1 + I2) \
    X(SUBINT, I0 = I1 - I2) \
    X(MULINT, I0 = I1 * I2) \
    X(DIVINT, I0 = I1 / I2) \
    X(JMP, PROC.PC += IMM) \
    X(JMPNZ, if (I0 != 0) PROC.PC += IMM) \
    X(JMPEQZ, if (I0 == 0) PROC.PC += IMM) \
    X(EQZC, I0 = I1 == 0) \
    X(LTZC, I0 = I1 < 0) \
    X(LTC, I0 = I1 < I2) \
    X(GTC, I0 = I1 > I2) \
    X(EQC, I0 = I1 == I2) \
    X(JMPI, PROC.PC += I0) \
    X(LDMINT, I0 = MID) \
    X(STMINT, MID = I0) \
    X(LDMINTI, I0 = MII) \
    X(STMINTI, MII = I0) \
    X(PUSHINT, PROC.pushi(I0.get())) \
    X(POPINT, long x; PROC.popi(x); I0 = x) \
    X(MOVINT, I0 = I1) \
    X(LDARG, I0 = PROC.get_arg()) \
    X(STARG, PROC.set_arg(I0.get())) \
    X(TIME, MACH.time()) \
    X(START, MACH.start(IMM)) \
    X(STOP, MACH.stop(IMM)) \
    X(GLDMS, ) \
    X(GLDMC, ) \
    X(PRINTINT, S0.out << I0) \
    X(STARTGRIND, CALLGRIND_START_INSTRUMENTATION) \
    X(STOPGRIND, CALLGRIND_STOP_INSTRUMENTATION) \
    X(RUN_TAPE, MACH.run_tape(R0, IMM, R1)) \
    X(JOIN_TAPE, MACH.join_tape(R0)) \

#endif /* GC_INSTRUCTIONS_H_ */
