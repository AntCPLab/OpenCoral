/*
 * instructions.h
 *
 */

#ifndef PROCESSOR_INSTRUCTIONS_H_
#define PROCESSOR_INSTRUCTIONS_H_

#define ARITHMETIC_INSTRUCTIONS \
    X(LDI, auto dest = &Procp.get_C()[r[0]]; typename sint::clear tmp = int(n), \
            *dest++ = tmp) \
    X(LDMS, auto dest = &Procp.get_S()[r[0]]; auto source = &Proc.machine.Mp.MS[n], \
            *dest++ = *source++) \
    X(STMS, auto source = &Procp.get_S()[r[0]]; auto dest = &Proc.machine.Mp.MS[n], \
            *dest++ = *source++) \
    X(LDMSI, auto dest = &Procp.get_S()[r[0]]; auto source = &Proc.get_Ci()[r[1]], \
            *dest++ = Proc.machine.Mp.read_S(*source++)) \
    X(STMSI, auto source = &Procp.get_S()[r[0]]; auto dest = &Proc.get_Ci()[r[1]], \
            Proc.machine.Mp.write_S(*dest++, *source++)) \
    X(MOVS, auto dest = &Procp.get_S()[r[0]]; auto source = &Procp.get_S()[r[1]], \
            *dest++ = *source++) \
    X(ADDS, auto dest = &Procp.get_S()[r[0]]; auto op1 = &Procp.get_S()[r[1]]; \
            auto op2 = &Procp.get_S()[r[2]], \
            *dest++ = *op1++ + *op2++) \
    X(ADDM, auto dest = &Procp.get_S()[r[0]]; auto op1 = &Procp.get_S()[r[1]]; \
            auto op2 = &Procp.get_C()[r[2]], \
            *dest++ = *op1++ + sint::constant(*op2++, Proc.P.my_num(), Procp.MC.get_alphai())) \
    X(ADDC, auto dest = &Procp.get_C()[r[0]]; auto op1 = &Procp.get_C()[r[1]]; \
            auto op2 = &Procp.get_C()[r[2]], \
            *dest++ = *op1++ + *op2++) \
    X(ADDCI, auto dest = &Procp.get_C()[r[0]]; auto op1 = &Procp.get_C()[r[1]]; \
            typename sint::clear op2 = int(n), \
            *dest++ = *op1++ + op2) \
    X(SUBS, auto dest = &Procp.get_S()[r[0]]; auto op1 = &Procp.get_S()[r[1]]; \
            auto op2 = &Procp.get_S()[r[2]], \
            *dest++ = *op1++ - *op2++) \
    X(SUBSFI, auto dest = &Procp.get_S()[r[0]]; auto op1 = &Procp.get_S()[r[1]]; \
            auto op2 = sint::constant(int(n), Proc.P.my_num(), Procp.MC.get_alphai()), \
            *dest++ = op2 - *op1++) \
    X(SUBML, auto dest = &Procp.get_S()[r[0]]; auto op1 = &Procp.get_S()[r[1]]; \
            auto op2 = &Procp.get_C()[r[2]], \
            *dest++ = *op1++ - sint::constant(*op2++, Proc.P.my_num(), Procp.MC.get_alphai())) \
    X(SUBMR, auto dest = &Procp.get_S()[r[0]]; auto op1 = &Procp.get_C()[r[1]]; \
            auto op2 = &Procp.get_S()[r[2]], \
            *dest++ = sint::constant(*op1++, Proc.P.my_num(), Procp.MC.get_alphai()) - *op2++) \
    X(SUBC, auto dest = &Procp.get_C()[r[0]]; auto op1 = &Procp.get_C()[r[1]]; \
            auto op2 = &Procp.get_C()[r[2]], \
            *dest++ = *op1++ - *op2++) \
    X(MULM, auto dest = &Procp.get_S()[r[0]]; auto op1 = &Procp.get_S()[r[1]]; \
            auto op2 = &Procp.get_C()[r[2]], \
            *dest++ = *op1++ * *op2++) \
    X(MULC, auto dest = &Procp.get_C()[r[0]]; auto op1 = &Procp.get_C()[r[1]]; \
            auto op2 = &Procp.get_C()[r[2]], \
            *dest++ = *op1++ * *op2++) \
    X(MULCI, auto dest = &Procp.get_C()[r[0]]; auto op1 = &Procp.get_C()[r[1]]; \
            typename sint::clear op2 = int(n), \
            *dest++ = *op1++ * op2) \
    X(MULSI, auto dest = &Procp.get_S()[r[0]]; auto op1 = &Procp.get_S()[r[1]]; \
            typename sint::clear op2 = int(n), \
            *dest++ = *op1++ * op2) \
    X(SHRCI, auto dest = &Procp.get_C()[r[0]]; auto op1 = &Procp.get_C()[r[1]], \
            *dest++ = *op1++ >> n) \
    X(TRIPLE, auto a = &Procp.get_S()[r[0]]; auto b = &Procp.get_S()[r[1]]; \
            auto c = &Procp.get_S()[r[2]], \
            Procp.DataF.get_three(DATA_TRIPLE, *a++, *b++, *c++)) \
    X(BIT, auto dest = &Procp.get_S()[r[0]], \
            Procp.DataF.get_one(DATA_BIT, *dest++)) \
    X(LDINT, auto dest = &Proc.get_Ci()[r[0]], \
            *dest++ = int(n)) \
    X(ADDINT, auto dest = &Proc.get_Ci()[r[0]]; auto op1 = &Proc.get_Ci()[r[1]]; \
            auto op2 = &Proc.get_Ci()[r[2]], \
            *dest++ = *op1++ + *op2++) \
    X(SUBINT, auto dest = &Proc.get_Ci()[r[0]]; auto op1 = &Proc.get_Ci()[r[1]]; \
            auto op2 = &Proc.get_Ci()[r[2]], \
            *dest++ = *op1++ - *op2++) \
    X(MULINT, auto dest = &Proc.get_Ci()[r[0]]; auto op1 = &Proc.get_Ci()[r[1]]; \
            auto op2 = &Proc.get_Ci()[r[2]], \
            *dest++ = *op1++ * *op2++) \
    X(DIVINT, auto dest = &Proc.get_Ci()[r[0]]; auto op1 = &Proc.get_Ci()[r[1]]; \
            auto op2 = &Proc.get_Ci()[r[2]], \
            *dest++ = *op1++ / *op2++) \
    X(INCINT, auto dest = &Proc.get_Ci()[r[0]]; auto base = Proc.get_Ci()[r[1]], \
            int inc = (i / start[0]) % start[1]; *dest++ = base + inc * int(n)) \
    X(CONVINT, auto dest = &Procp.get_C()[r[0]]; auto source = &Proc.get_Ci()[r[1]], \
            *dest++ = *source++) \


#endif /* PROCESSOR_INSTRUCTIONS_H_ */
