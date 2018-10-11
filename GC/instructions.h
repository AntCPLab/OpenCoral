// (C) 2018 University of Bristol, Bar-Ilan University. See License.txt

/*
 * instructions.h
 *
 */

#ifndef GC_INSTRUCTIONS_H_
#define GC_INSTRUCTIONS_H_

#include <valgrind/callgrind.h>

#define P processor
#define INST instruction
#define M processor.machine

#define R0 instruction.get_r(0)
#define R1 instruction.get_r(1)
#define R2 instruction.get_r(2)

#define S0 processor.S[instruction.get_r(0)]
#define S1 processor.S[instruction.get_r(1)]
#define S2 processor.S[instruction.get_r(2)]

#define C0 processor.C[instruction.get_r(0)]
#define C1 processor.C[instruction.get_r(1)]
#define C2 processor.C[instruction.get_r(2)]

#define I0 processor.I[instruction.get_r(0)]
#define I1 processor.I[instruction.get_r(1)]
#define I2 processor.I[instruction.get_r(2)]

#define N instruction.get_n()
#define EXTRA instruction.get_start()

#define MSD M.MS[N]
#define MC M.MC[N]
#define MID M.MI[N]

#define MSI M.MS[I1.get()]
#define MII M.MI[I1.get()]

#define MD M.MD

#define INSTRUCTIONS \
    X(XORS, P.xors(EXTRA)) \
    X(XORC, C0.xor_(C1, C2)) \
    X(XORCI, C0.xor_(C1, N)) \
    X(ANDRS, T::andrs(P, EXTRA)) \
    X(INPUTB, T::inputb(P, EXTRA)) \
    X(ADDC, C0 = C1 + C2) \
    X(ADDCI, C0 = C1 + N) \
    X(MULCI, C0 = C1 * N) \
    X(BITDECS, P.bitdecs(EXTRA, S0)) \
    X(BITCOMS, P.bitcoms(S0, EXTRA)) \
    X(BITDECC, P.bitdecc(EXTRA, C0)) \
    X(BITDECINT, P.bitdecint(EXTRA, I0)) \
    X(SHRCI, C0 = C1 >> N) \
    X(LDBITS, S0.load(R1, N)) \
    X(LDMS, S0 = MSD) \
    X(STMS, MSD = S0) \
    X(LDMSI, S0 = MSI) \
    X(STMSI, MSI = S0) \
    X(LDMC, C0 = MC) \
    X(STMC, MC = C0) \
    X(LDMSD, P.load_dynamic_direct(EXTRA)) \
    X(STMSD, P.store_dynamic_direct(EXTRA)) \
    X(LDMSDI, P.load_dynamic_indirect(EXTRA)) \
    X(STMSDI, P.store_dynamic_indirect(EXTRA)) \
    X(STMSDCI, P.store_clear_in_dynamic(EXTRA)) \
    X(CONVSINT, S0.load(N, I1)) \
    X(CONVCINT, C0 = I1) \
    X(MOVS, S0 = S1) \
    X(BIT, P.random_bit(S0)) \
    X(REVEAL, S1.reveal(C0)) \
    X(PRINTREG, P.print_reg(R0, N)) \
    X(PRINTREGPLAIN, P.print_reg_plain(C0)) \
    X(PRINTREGSIGNED, P.print_reg_signed(N, C0)) \
    X(PRINTCHR, P.print_chr(N)) \
    X(PRINTSTR, P.print_str(N)) \
    X(LDINT, I0 = int(N)) \
    X(ADDINT, I0 = I1 + I2) \
    X(SUBINT, I0 = I1 - I2) \
    X(MULINT, I0 = I1 * I2) \
    X(DIVINT, I0 = I1 / I2) \
    X(JMP, P.PC += N) \
    X(JMPNZ, if (I0 != 0) P.PC += N) \
    X(JMPEQZ, if (I0 == 0) P.PC += N) \
    X(EQZC, I0 = I1 == 0) \
    X(LTZC, I0 = I1 < 0) \
    X(LTC, I0 = I1 < I2) \
    X(GTC, I0 = I1 > I2) \
    X(EQC, I0 = I1 == I2) \
    X(JMPI, P.PC += I0) \
    X(LDMINT, I0 = MID) \
    X(STMINT, MID = I0) \
    X(LDMINTI, I0 = MII) \
    X(STMINTI, MII = I0) \
    X(PUSHINT, P.pushi(I0.get())) \
    X(POPINT, long x; P.popi(x); I0 = x) \
    X(MOVINT, I0 = I1) \
    X(LDARG, I0 = P.get_arg()) \
    X(STARG, P.set_arg(I0.get())) \
    X(TIME, M.time()) \
    X(START, M.start(N)) \
    X(STOP, M.stop(N)) \
    X(GLDMS, ) \
    X(GLDMC, ) \
    X(PRINTINT, S0.out << I0) \
    X(STARTGRIND, CALLGRIND_START_INSTRUMENTATION) \
    X(STOPGRIND, CALLGRIND_STOP_INSTRUMENTATION) \

#endif /* GC_INSTRUCTIONS_H_ */
