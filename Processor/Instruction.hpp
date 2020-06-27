#ifndef PROCESSOR_INSTRUCTION_HPP_
#define PROCESSOR_INSTRUCTION_HPP_

#include "Processor/Instruction.h"
#include "Processor/Machine.h"
#include "Processor/Processor.h"
#include "Processor/IntInput.h"
#include "Processor/FixInput.h"
#include "Processor/FloatInput.h"
#include "Exceptions/Exceptions.h"
#include "Tools/time-func.h"
#include "Tools/parse.h"
#include "GC/Instruction.h"
#include "GC/instructions.h"

//#include "Processor/Processor.hpp"
#include "Processor/Binary_File_IO.hpp"
#include "Processor/PrivateOutput.hpp"
//#include "Processor/Input.hpp"
//#include "Processor/Beaver.hpp"
//#include "Protocols/Shamir.hpp"
//#include "Protocols/ShamirInput.hpp"
//#include "Protocols/Replicated.hpp"
//#include "Protocols/MaliciousRepMC.hpp"
//#include "Protocols/ShamirMC.hpp"
#include "Math/bigint.hpp"

#include <stdlib.h>
#include <algorithm>
#include <sstream>
#include <map>
#include <iomanip>

#include "Tools/callgrind.h"

inline
void BaseInstruction::parse(istream& s, int inst_pos)
{
  n=0; start.resize(0);
  r[0]=0; r[1]=0; r[2]=0; r[3]=0;

  int pos=s.tellg();
  opcode=get_int(s);
  size=unsigned(opcode)>>10;
  opcode&=0x3FF;
  
  if (size==0)
    size=1;

  parse_operands(s, inst_pos, pos);
}

inline
void BaseInstruction::parse_operands(istream& s, int pos, int file_pos)
{
  int num_var_args = 0;
  switch (opcode)
  {
      // instructions with 3 register operands
      case ADDC:
      case ADDCB:
      case ADDS:
      case ADDM:
      case SUBC:
      case SUBS:
      case SUBML:
      case SUBMR:
      case MULC:
      case MULM:
      case DIVC:
      case MODC:
      case TRIPLE:
      case ANDC:
      case XORC:
      case XORCB:
      case ORC:
      case SHLC:
      case SHRC:
      case GADDC:
      case GADDS:
      case GADDM:
      case GSUBC:
      case GSUBS:
      case GSUBML:
      case GSUBMR:
      case GMULC:
      case GMULM:
      case GDIVC:
      case GTRIPLE:
      case GBITTRIPLE:
      case GBITGF2NTRIPLE:
      case GANDC:
      case GXORC:
      case GORC:
      case GMULBITC:
      case GMULBITM:
      case LTC:
      case GTC:
      case EQC:
      case ADDINT:
      case SUBINT:
      case MULINT:
      case DIVINT:
      case CONDPRINTPLAIN:
        r[0]=get_int(s);
        r[1]=get_int(s);
        r[2]=get_int(s);
        break;
      // instructions with 2 register operands
      case LDMCI:
      case LDMSI:
      case STMCI:
      case STMSI:
      case LDMSBI:
      case STMSBI:
      case MOVC:
      case MOVS:
      case MOVSB:
      case MOVINT:
      case LDMINTI:
      case STMINTI:
      case LEGENDREC:
      case SQUARE:
      case INV:
      case GINV:
      case CONVINT:
      case GLDMCI:
      case GLDMSI:
      case GSTMCI:
      case GSTMSI:
      case GMOVC:
      case GMOVS:
      case GSQUARE:
      case GNOTC:
      case GCONVINT:
      case GCONVGF2N:
      case LTZC:
      case EQZC:
      case RAND:
      case PROTECTMEMS:
      case PROTECTMEMC:
      case GPROTECTMEMS:
      case GPROTECTMEMC:
      case PROTECTMEMINT:
      case DABIT:
      case SHUFFLE:
        r[0]=get_int(s);
        r[1]=get_int(s);
        break;
      // instructions with 1 register operand
      case BIT:
      case BITB:
      case PRINTMEM:
      case PRINTREGPLAIN:
      case PRINTREGPLAINB:
      case LDTN:
      case LDARG:
      case STARG:
      case JMPI:
      case GBIT:
      case GPRINTMEM:
      case GPRINTREGPLAIN:
      case JOIN_TAPE:
      case PUSHINT:
      case POPINT:
      case PUBINPUT:
      case RAWOUTPUT:
      case GRAWOUTPUT:
      case PRINTCHRINT:
      case PRINTSTRINT:
      case PRINTINT:
      case NPLAYERS:
      case THRESHOLD:
      case PLAYERID:
        r[0]=get_int(s);
        break;
      // instructions with 3 registers + 1 integer operand
        r[0]=get_int(s);
        r[1]=get_int(s);
        r[2]=get_int(s);
        n = get_int(s);
        break;
      // instructions with 2 registers + 1 integer operand
      case ADDCI:
      case ADDCBI:
      case ADDSI:
      case SUBCI:
      case SUBSI:
      case SUBCFI:
      case SUBSFI:
      case MULCI:
      case MULCBI:
      case MULSI:
      case DIVCI:
      case MODCI:
      case ANDCI:
      case XORCI:
      case XORCBI:
      case ORCI:
      case SHLCI:
      case SHRCI:
      case SHLCBI:
      case SHRCBI:
      case NOTC:
      case CONVMODP:
      case GADDCI:
      case GADDSI:
      case GSUBCI:
      case GSUBSI:
      case GSUBCFI:
      case GSUBSFI:
      case GMULCI:
      case GMULSI:
      case GDIVCI:
      case GANDCI:
      case GXORCI:
      case GORCI:
      case GSHLCI:
      case GSHRCI:
      case USE:
      case USE_INP:
      case USE_EDABIT:
      case RUN_TAPE:
      case STARTPRIVATEOUTPUT:
      case GSTARTPRIVATEOUTPUT:
      case STOPPRIVATEOUTPUT:
      case GSTOPPRIVATEOUTPUT:
      case DIGESTC:
        r[0]=get_int(s);
        r[1]=get_int(s);
        n = get_int(s);
        break;
      // instructions with 1 register + 1 integer operand
      case LDI:
      case LDSI:
      case LDMC:
      case LDMS:
      case STMC:
      case STMS:
      case LDMSB:
      case STMSB:
      case LDMCB:
      case STMCB:
      case LDMINT:
      case STMINT:
      case JMPNZ:
      case JMPEQZ:
      case GLDI:
      case GLDSI:
      case GLDMC:
      case GLDMS:
      case GSTMC:
      case GSTMS:
      case PRINTREG:
      case PRINTREGB:
      case GPRINTREG:
      case LDINT:
      case INPUTMASK:
      case GINPUTMASK:
      case ACCEPTCLIENTCONNECTION:
      case INV2M:
      case CONDPRINTSTR:
      case CONDPRINTSTRB:
        r[0]=get_int(s);
        n = get_int(s);
        break;
      // instructions with 1 integer operand
      case PRINTSTR:
      case PRINTCHR:
      case JMP:
      case START:
      case STOP:
      case LISTEN:
      case PRINTFLOATPREC:
        n = get_int(s);
        break;
      // instructions with no operand
      case TIME:
      case CRASH:
      case STARTGRIND:
      case STOPGRIND:
        break;
      // instructions with 5 register operands
      case PRINTFLOATPLAIN:
      case PRINTFLOATPLAINB:
        get_vector(5, start, s);
        break;
      case INCINT:
        r[0]=get_int(s);
        r[1]=get_int(s);
        n = get_int(s);
        get_vector(2, start, s);
        break;
      // open instructions + read/write instructions with variable length args
      case WRITEFILESHARE:
      case OPEN:
      case GOPEN:
      case MULS:
      case GMULS:
      case MULRS:
      case GMULRS:
      case DOTPRODS:
      case GDOTPRODS:
      case INPUT:
      case GINPUT:
      case INPUTFIX:
      case INPUTFLOAT:
      case INPUTMIXED:
      case INPUTMIXEDREG:
      case RAWINPUT:
      case GRAWINPUT:
      case TRUNC_PR:
        num_var_args = get_int(s);
        get_vector(num_var_args, start, s);
        break;
      case MATMULS:
        get_ints(r, s, 3);
        get_vector(3, start, s);
        break;
      case MATMULSM:
        get_ints(r, s, 3);
        get_vector(9, start, s);
        break;
      case CONV2DS:
        get_ints(r, s, 3);
        get_vector(11, start, s);
        break;

      // read from file, input is opcode num_args, 
      //   start_file_posn (read), end_file_posn(write) var1, var2, ...
      case READFILESHARE:
        num_var_args = get_int(s) - 2;
        r[0] = get_int(s);
        r[1] = get_int(s);
        get_vector(num_var_args, start, s);
        break;

      // read from external client, input is : opcode num_args, client_id, var1, var2 ...
      case READSOCKETC:
      case READSOCKETS:
      case READSOCKETINT:
        num_var_args = get_int(s) - 1;
        r[0] = get_int(s);
        get_vector(num_var_args, start, s);
        break;

      // write to external client, input is : opcode num_args, client_id, message_type, var1, var2 ...
      case WRITESOCKETC:
      case WRITESOCKETS:
      case WRITESOCKETSHARE:
      case WRITESOCKETINT:
        num_var_args = get_int(s) - 2;
        r[0] = get_int(s);
        r[1] = get_int(s);
        get_vector(num_var_args, start, s);
        break;
      case CONNECTIPV4:
        throw runtime_error("parties as clients not supported any more");
      case READCLIENTPUBLICKEY:
      case INITSECURESOCKET:
      case RESPSECURESOCKET:
        throw runtime_error("VM-controlled encryption not supported any more");
      // raw input
      case STARTINPUT:
      case GSTARTINPUT:
      case STOPINPUT:
      case GSTOPINPUT:
        throw runtime_error("two-stage input not supported any more");
      case GBITDEC:
      case GBITCOM:
        num_var_args = get_int(s) - 2;
        r[0] = get_int(s);
        n = get_int(s);
        get_vector(num_var_args, start, s);
        break;
      case BITDECINT:
      case EDABIT:
      case SEDABIT:
          num_var_args = get_int(s) - 1;
          r[0] = get_int(s);
          get_vector(num_var_args, start, s);
          break;
      case PREP:
      case GPREP:
        // subtract extra argument
        num_var_args = get_int(s) - 1;
        s.read((char*)r, sizeof(r));
        start.resize(num_var_args);
        for (int i = 0; i < num_var_args; i++)
        { start[i] = get_int(s); }
        break;
      case USE_PREP:
      case GUSE_PREP:
        s.read((char*)r, sizeof(r));
        n = get_int(s);
        break;
      case REQBL:
        n = get_int(s);
        BaseMachine::s().reqbl(n);
        break;
      case GREQBL:
        n = get_int(s);
        if (n > 0 && gf2n::degree() < int(n))
          {
            stringstream ss;
            ss << "Tape requires prime of bit length " << n << endl;
            throw Processor_Error(ss.str());
          }
        break;
      case XORM:
      case ANDM:
        n = get_int(s);
        get_ints(r, s, 3);
        break;
      case LDBITS:
        get_ints(r, s, 2);
        n = get_int(s);
        break;
      case BITDECS:
      case BITCOMS:
      case BITDECC:
      case CONVCINTVEC:
        num_var_args = get_int(s) - 1;
        get_ints(r, s, 1);
        get_vector(num_var_args, start, s);
        break;
      case CONVCINT:
      case CONVCBIT:
        get_ints(r, s, 2);
        break;
      case CONVSINT:
      case CONVCBITVEC:
      case CONVCBIT2S:
      case NOTS:
        n = get_int(s);
        get_ints(r, s, 2);
        break;
      case LDMSDI:
      case STMSDI:
      case LDMSD:
      case STMSD:
      case STMSDCI:
      case XORS:
      case ANDRS:
      case ANDS:
      case INPUTB:
      case REVEAL:
        get_vector(get_int(s), start, s);
        break;
      case PRINTREGSIGNED:
        n = get_int(s);
        get_ints(r, s, 1);
        break;
      case TRANS:
        num_var_args = get_int(s) - 1;
        n = get_int(s);
        get_vector(num_var_args, start, s);
        break;
      case SPLIT:
        num_var_args = get_int(s) - 2;
        n = get_int(s);
        get_ints(r, s, 1);
        get_vector(num_var_args, start, s);
        break;
      default:
        ostringstream os;
        os << "Invalid instruction " << showbase << hex << opcode << " at " << dec
            << pos << "/" << hex << file_pos << dec << endl;
        throw Invalid_Instruction(os.str());
  }
}

inline
bool Instruction::get_offline_data_usage(DataPositions& usage)
{
  switch (opcode)
  {
    case USE:
      if (r[0] >= N_DATA_FIELD_TYPE)
        throw invalid_program();
      if (r[1] >= N_DTYPE)
        throw invalid_program();
      usage.files[r[0]][r[1]] = n;
      return int(n) >= 0;
    case USE_INP:
      if (r[0] >= N_DATA_FIELD_TYPE)
        throw invalid_program();
      if ((unsigned)r[1] >= usage.inputs.size())
        throw Processor_Error("Player number too high");
      usage.inputs[r[1]][r[0]] = n;
      return int(n) >= 0;
    case USE_EDABIT:
      usage.edabits[{r[0], r[1]}] = n;
      return int(n) >= 0;
    case USE_PREP:
      usage.extended[DATA_INT][r] = n;
      return int(n) >= 0;
    case GUSE_PREP:
      usage.extended[gf2n::field_type()][r] = n;
      return int(n) >= 0;
    default:
      return true;
  }
}

inline
int BaseInstruction::get_reg_type() const
{
  switch (opcode & 0x2B0)
  {
    case SECRET_WRITE:
      return SBIT;
    case CLEAR_WRITE:
      return CBIT;
  }

  switch (opcode) {
    case LDMINT:
    case STMINT:
    case LDMINTI:
    case STMINTI:
    case PUSHINT:
    case POPINT:
    case MOVINT:
    case READSOCKETINT:
    case WRITESOCKETINT:
    case READCLIENTPUBLICKEY:
    case INITSECURESOCKET:
    case RESPSECURESOCKET:
    case LDARG:
    case LDINT:
    case INCINT:
    case SHUFFLE:
    case CONVMODP:
    case GCONVGF2N:
    case RAND:
    case NPLAYERS:
    case THRESHOLD:
    case PLAYERID:
    case CONVCBIT:
    case CONVCBITVEC:
      return INT;
    case PREP:
    case USE_PREP:
    case GUSE_PREP:
    case USE_EDABIT:
      // those use r[] not for registers
      return NONE;
    default:
      if (is_gf2n_instruction())
        return GF2N;
      else if (opcode >> 4 == 0x9)
        return INT;
      else
        return MODP;
  }
}

inline
unsigned BaseInstruction::get_max_reg(int reg_type) const
{
  int skip = 0;
  int offset = 0;
  int size_offset = 0;
  int size = this->size;

  if (opcode == DABIT)
  {
      if (reg_type == SBIT)
          return r[1] + size;
      else if (reg_type == MODP)
          return r[0] + size;
      else
          return 0;
  }
  else if (opcode == EDABIT or opcode == SEDABIT)
  {
      if (reg_type == SBIT)
          skip = 1;
      else if (reg_type == MODP)
          return r[0] + size;
      else
          return 0;
  }
  else if (get_reg_type() != reg_type)
  {
	  return 0;
  }

  switch (opcode)
  {
  case DOTPRODS:
  {
      int res = 0;
      auto it = start.begin();
      while (it != start.end())
      {
          assert(it < start.end());
          int n = *it;
          res = max(res, *it++);
          it += n - 1;
      }
      return res;
  }
  case MATMULS:
  case MATMULSM:
      return r[0] + start[0] * start[2];
  case CONV2DS:
      return r[0] + start[0] * start[1];
  case LDMSD:
  case LDMSDI:
      skip = 3;
      break;
  case STMSD:
  case STMSDI:
      skip = 2;
      break;
  case ANDRS:
  case XORS:
  case ANDS:
      skip = 4;
      offset = 1;
      size_offset = -1;
      break;
  case INPUTB:
      skip = 4;
      offset = 3;
      size_offset = -2;
      break;
  case ANDM:
  case NOTS:
      size = DIV_CEIL(n, 64);
      break;
  case CONVCBIT2S:
      size = DIV_CEIL(n, 64);
      break;
  case CONVCINTVEC:
      size = DIV_CEIL(size, 64);
      break;
  case CONVCBITVEC:
      size = n;
      break;
  case REVEAL:
      size = DIV_CEIL(n, 64);
      skip = 3;
      offset = 1;
      size_offset = -1;
      break;
  case SPLIT:
      size = DIV_CEIL(this->size, 64);
      skip = 1;
      break;
  }

  if (skip > 0)
  {
      unsigned m = 0;
      for (size_t i = offset; i < start.size(); i += skip)
      {
          if (size_offset != 0)
              size = DIV_CEIL(start[i + size_offset], 64);
          m = max(m, (unsigned)start[i] + size);
      }
      return m;
  }

  unsigned res = 0;
  for (auto x : start)
    res = max(res, (unsigned)x);
  for (auto x : r)
	res = max(res, (unsigned)x);
  return res + size;
}

inline
unsigned BaseInstruction::get_mem(RegType reg_type, SecrecyType sec_type) const
{
  if (get_reg_type() == reg_type and is_direct_memory_access(sec_type))
    return n + size;
  else
    return 0;
}

inline
bool BaseInstruction::is_direct_memory_access(SecrecyType sec_type) const
{
  switch (opcode)
  {
  case LDMS:
  case STMS:
  case GLDMS:
  case GSTMS:
    return sec_type == SECRET;
  case LDMC:
  case STMC:
  case GLDMC:
  case GSTMC:
    return sec_type == CLEAR;
  case LDMINT:
  case STMINT:
  case LDMSB:
  case STMSB:
  case LDMCB:
  case STMCB:
    return true;
  default:
    return false;
  }
}


inline
ostream& operator<<(ostream& s,const Instruction& instr)
{
  s << instr.opcode << " : ";
  for (int i=0; i<3; i++)
    { s << instr.r[i] << " "; }
  s << " : " << instr.n;
  if (instr.start.size()!=0)
    { s << " : " << instr.start.size() << " : ";
      for (unsigned int i=0; i<instr.start.size(); i++)
	{ s << instr.start[i] << " "; }
    }
  return s;
} 


template<class sint, class sgf2n>
#ifndef __clang__
__attribute__((always_inline))
#endif
inline void Instruction::execute(Processor<sint, sgf2n>& Proc) const
{
  Proc.PC+=1;
  auto& Procp = Proc.Procp;
  auto& Proc2 = Proc.Proc2;

  // binary instructions
  typedef typename sint::bit_type T;
  auto& processor = Proc.Procb;
  auto& instruction = *this;
  auto& Ci = Proc.get_Ci();

  // optimize some instructions
  switch (opcode)
  {
    case GADDC:
      for (int i = 0; i < size; i++)
        Proc.get_C2_ref(r[0] + i).add(Proc.read_C2(r[1] + i),Proc.read_C2(r[2] + i));
      return;
    case GADDS:
      for (int i = 0; i < size; i++)
        Proc.get_S2_ref(r[0] + i).add(Proc.read_S2(r[1] + i),Proc.read_S2(r[2] + i));
      return;
    case GMOVC:
      for (int i = 0; i < size; i++)
        Proc.write_C2(r[0] + i, Proc.read_C2(r[1] + i));
      return;
    case GANDC:
      for (int i = 0; i < size; i++)
         Proc.get_C2_ref(r[0] + i).AND(Proc.read_C2(r[1] + i),Proc.read_C2(r[2] + i));
      return;
    case GSHLCI:
      for (int i = 0; i < size; i++)
         Proc.get_C2_ref(r[0] + i).SHL(Proc.read_C2(r[1] + i),n);
      return;
    case GSHRCI:
      for (int i = 0; i < size; i++)
        Proc.get_C2_ref(r[0] + i).SHR(Proc.read_C2(r[1] + i),n);
      return;
    case GMULM:
      for (int i = 0; i < size; i++)
         Proc.get_S2_ref(r[0] + i).mul(Proc.read_S2(r[1] + i),Proc.read_C2(r[2] + i));
      return;
    case LDI:
      Proc.temp.assign_ansp(n);
      for (int i = 0; i < size; i++)
        Proc.write_Cp(r[0] + i,Proc.temp.ansp);
      return;
    case LDMSI:
      for (int i = 0; i < size; i++)
        Proc.write_Sp(r[0] + i, Proc.machine.Mp.read_S(Proc.read_Ci(r[1] + i)));
      return;
    case LDMS:
      for (int i = 0; i < size; i++)
        Proc.write_Sp(r[0] + i, Proc.machine.Mp.read_S(n + i));
      return;
    case STMSI:
      for (int i = 0; i < size; i++)
        Proc.machine.Mp.write_S(Proc.read_Ci(r[1] + i), Proc.read_Sp(r[0] + i), Proc.PC);
      return;
    case STMS:
      for (int i = 0; i < size; i++)
        Proc.machine.Mp.write_S(n + i, Proc.read_Sp(r[0] + i), Proc.PC);
      return;
    case ADDC:
      for (int i = 0; i < size; i++)
        Proc.get_Cp_ref(r[0] + i).add(Proc.read_Cp(r[1] + i),Proc.read_Cp(r[2] + i));
      return;
    case ADDS:
      for (int i = 0; i < size; i++)
        Proc.get_Sp_ref(r[0] + i).add(Proc.read_Sp(r[1] + i),Proc.read_Sp(r[2] + i));
      return;
    case ADDM:
      for (int i = 0; i < size; i++)
        Proc.get_Sp_ref(r[0] + i).add(Proc.read_Sp(r[1] + i),Proc.read_Cp(r[2] + i),Proc.P.my_num(),Proc.MCp.get_alphai());
      return;
    case ADDCI:
      Proc.temp.assign_ansp(n);
      for (int i = 0; i < size; i++)
         Proc.get_Cp_ref(r[0] + i).add(Proc.temp.ansp,Proc.read_Cp(r[1] + i));
      return;
    case SUBS:
      for (int i = 0; i < size; i++)
        Proc.get_Sp_ref(r[0] + i).sub(Proc.read_Sp(r[1] + i),Proc.read_Sp(r[2] + i));
      return;
    case SUBSFI:
      Proc.temp.assign_ansp(n);
      for (int i = 0; i < size; i++)
        Proc.get_Sp_ref(r[0] + i).sub(Proc.temp.ansp,Proc.read_Sp(r[1] + i),Proc.P.my_num(),Proc.MCp.get_alphai());
      return;
    case SUBML:
      for (int i = 0; i < size; i++)
         Proc.get_Sp_ref(r[0] + i).sub(Proc.read_Sp(r[1] + i), Proc.read_Cp(r[2] + i),
             Proc.P.my_num(), Proc.MCp.get_alphai());
      return;
    case SUBMR:
      for (int i = 0; i < size; i++)
         Proc.get_Sp_ref(r[0] + i).sub(Proc.read_Cp(r[1] + i), Proc.read_Sp(r[2] + i),
             Proc.P.my_num(), Proc.MCp.get_alphai());
      return;
    case MULM:
      for (int i = 0; i < size; i++)
        Proc.get_Sp_ref(r[0] + i).mul(Proc.read_Sp(r[1] + i),Proc.read_Cp(r[2] + i));
      return;
    case MULC:
      for (int i = 0; i < size; i++)
        Proc.get_Cp_ref(r[0] + i).mul(Proc.read_Cp(r[1] + i),Proc.read_Cp(r[2] + i));
      return;
    case MULCI:
      Proc.temp.assign_ansp(n);
      for (int i = 0; i < size; i++)
        Proc.get_Cp_ref(r[0] + i).mul(Proc.temp.ansp,Proc.read_Cp(r[1] + i));
      return;
    case SHRCI:
      for (int i = 0; i < size; i++)
         Proc.get_Cp_ref(r[0] + i).SHR(Proc.read_Cp(r[1] + i), n);
      return;
    case TRIPLE:
      for (int i = 0; i < size; i++)
        Procp.DataF.get_three(DATA_TRIPLE, Proc.get_Sp_ref(r[0] + i),
            Proc.get_Sp_ref(r[1] + i), Proc.get_Sp_ref(r[2] + i));
      return;
    case BIT:
      for (int i = 0; i < size; i++)
        Procp.DataF.get_one(DATA_BIT, Proc.get_Sp_ref(r[0] + i));
      return;
    case LDINT:
      for (int i = 0; i < size; i++)
        Proc.write_Ci(r[0] + i, int(n));
      return;
    case INCINT:
      {
        for (int i = 0; i < size; i++)
          {
            int inc = (i / start[0]) % start[1];
            Proc.write_Ci(r[0] + i, Proc.read_Ci(r[1]) + inc * int(n));
          }
      }
      return;
    case SHUFFLE:
      for (int i = 0; i < size; i++)
        Proc.write_Ci(r[0] + i, Proc.read_Ci(r[1] + i));
      for (int i = 0; i < size; i++)
      {
        int j = Proc.shared_prng.get_uint(size - i);
        swap(Proc.get_Ci_ref(r[0] + i), Proc.get_Ci_ref(r[0] + i + j));
      }
      return;
    case ADDINT:
      for (int i = 0; i < size; i++)
        Proc.get_Ci_ref(r[0] + i) = Proc.read_Ci(r[1] + i) + Proc.read_Ci(r[2] + i);
      return;
    case SUBINT:
      for (int i = 0; i < size; i++)
        Proc.get_Ci_ref(r[0] + i) = Proc.read_Ci(r[1] + i) - Proc.read_Ci(r[2] + i);
      return;
    case MULINT:
      for (int i = 0; i < size; i++)
        Proc.get_Ci_ref(r[0] + i) = Proc.read_Ci(r[1] + i) * Proc.read_Ci(r[2] + i);
      return;
    case DIVINT:
      for (int i = 0; i < size; i++)
        Proc.get_Ci_ref(r[0] + i) = Proc.read_Ci(r[1] + i) / Proc.read_Ci(r[2] + i);
      return;
    case CONVINT:
      for (int i = 0; i < size; i++)
      {
        Proc.temp.assign_ansp(Proc.read_Ci(r[1] + i));
        Proc.get_Cp_ref(r[0] + i) = Proc.temp.ansp;
      }
      return;
    case CONVMODP:
      if (n == 0)
        {
          for (int i = 0; i < size; i++)
            Proc.write_Ci(r[0] + i,
                Integer::convert_unsigned(Proc.read_Cp(r[1] + i)).get());
        }
      else if (n <= 64)
        for (int i = 0; i < size; i++)
          Proc.write_Ci(r[0] + i, Integer(Proc.read_Cp(r[1] + i), n).get());
      else
        throw Processor_Error(to_string(n) + "-bit conversion impossible; "
            "integer registers only have 64 bits");
      return;
#define X(NAME, CODE) case NAME: CODE; return;
      COMBI_INSTRUCTIONS
#undef X
  }

  int r[3] = {this->r[0], this->r[1], this->r[2]};
  int n = this->n;
  for (int i = 0; i < size; i++) 
  { switch (opcode)
    { case LDI: 
        Proc.temp.assign_ansp(n);
        Proc.write_Cp(r[0],Proc.temp.ansp);
        break;
      case GLDI: 
        Proc.temp.ans2.assign(n);
        Proc.write_C2(r[0],Proc.temp.ans2);
        break;
      case LDSI:
        Proc.get_Sp_ref(r[0]).assign(n, Proc.P.my_num(), Proc.MCp.get_alphai());
        break;
      case GLDSI:
        Proc.get_S2_ref(r[0]).assign(n, Proc.P.my_num(), Proc.MC2.get_alphai());
        break;
      case LDMC:
        Proc.write_Cp(r[0],Proc.machine.Mp.read_C(n));
        n++;
        break;
      case GLDMC:
        Proc.write_C2(r[0],Proc.machine.M2.read_C(n));
        n++;
        break;
      case LDMS:
        Proc.write_Sp(r[0],Proc.machine.Mp.read_S(n));
        n++;
        break;
      case GLDMS:
        Proc.write_S2(r[0],Proc.machine.M2.read_S(n));
        n++;
        break;
      case LDMINT:
        Proc.write_Ci(r[0],Proc.machine.Mi.read_C(n).get());
        n++;
        break;
      case LDMCI:
        Proc.write_Cp(r[0], Proc.machine.Mp.read_C(Proc.read_Ci(r[1])));
        break;
      case GLDMCI:
        Proc.write_C2(r[0], Proc.machine.M2.read_C(Proc.read_Ci(r[1])));
        break;
      case LDMSI:
        Proc.write_Sp(r[0], Proc.machine.Mp.read_S(Proc.read_Ci(r[1])));
        break;
      case GLDMSI:
        Proc.write_S2(r[0], Proc.machine.M2.read_S(Proc.read_Ci(r[1])));
        break;
      case LDMINTI:
        Proc.write_Ci(r[0], Proc.machine.Mi.read_C(Proc.read_Ci(r[1])).get());
        break;
      case STMC:
        Proc.machine.Mp.write_C(n,Proc.read_Cp(r[0]),Proc.PC);
        n++;
        break;
      case GSTMC:
        Proc.machine.M2.write_C(n,Proc.read_C2(r[0]),Proc.PC);
        n++;
        break;
      case STMS:
        Proc.machine.Mp.write_S(n,Proc.read_Sp(r[0]),Proc.PC);
        n++;
        break;
      case GSTMS:
        Proc.machine.M2.write_S(n,Proc.read_S2(r[0]),Proc.PC);
        n++;
        break;
      case STMINT:
        Proc.machine.Mi.write_C(n,Integer(Proc.read_Ci(r[0])),Proc.PC);
        n++;
        break;
      case STMCI:
        Proc.machine.Mp.write_C(Proc.read_Ci(r[1]), Proc.read_Cp(r[0]),Proc.PC);
        break;
      case GSTMCI:
        Proc.machine.M2.write_C(Proc.read_Ci(r[1]), Proc.read_C2(r[0]),Proc.PC);
        break;
      case STMSI:
        Proc.machine.Mp.write_S(Proc.read_Ci(r[1]), Proc.read_Sp(r[0]),Proc.PC);
        break;
      case GSTMSI:
        Proc.machine.M2.write_S(Proc.read_Ci(r[1]), Proc.read_S2(r[0]),Proc.PC);
        break;
      case STMINTI:
        Proc.machine.Mi.write_C(Proc.read_Ci(r[1]), Integer(Proc.read_Ci(r[0])),Proc.PC);
        break;
      case MOVC:
        Proc.write_Cp(r[0],Proc.read_Cp(r[1]));
        break;
      case GMOVC:
        Proc.write_C2(r[0],Proc.read_C2(r[1]));
        break;
      case MOVS:
        Proc.write_Sp(r[0],Proc.read_Sp(r[1]));
        break;
      case GMOVS:
        Proc.write_S2(r[0],Proc.read_S2(r[1]));
        break;
      case MOVINT:
        Proc.write_Ci(r[0],Proc.read_Ci(r[1]));
        break;
      case PROTECTMEMS:
        Proc.machine.Mp.protect_s(Proc.read_Ci(r[0]), Proc.read_Ci(r[1]));
        break;
      case PROTECTMEMC:
        Proc.machine.Mp.protect_c(Proc.read_Ci(r[0]), Proc.read_Ci(r[1]));
        break;
      case GPROTECTMEMS:
        Proc.machine.M2.protect_s(Proc.read_Ci(r[0]), Proc.read_Ci(r[1]));
        break;
      case GPROTECTMEMC:
        Proc.machine.M2.protect_c(Proc.read_Ci(r[0]), Proc.read_Ci(r[1]));
        break;
      case PROTECTMEMINT:
        Proc.machine.Mi.protect_c(Proc.read_Ci(r[0]), Proc.read_Ci(r[1]));
        break;
      case PUSHINT:
        Proc.pushi(Proc.read_Ci(r[0]));
        break;
      case POPINT:
        Proc.popi(Proc.get_Ci_ref(r[0]));
        break;
      case LDTN:
        Proc.write_Ci(r[0],Proc.get_thread_num());
        break;
      case LDARG:
        Proc.write_Ci(r[0],Proc.get_arg());
        break;
      case STARG:
        Proc.set_arg(Proc.read_Ci(r[0]));
        break;
      case ADDC:
           Proc.get_Cp_ref(r[0]).add(Proc.read_Cp(r[1]),Proc.read_Cp(r[2]));
        break;
      case GADDC:
           Proc.get_C2_ref(r[0]).add(Proc.read_C2(r[1]),Proc.read_C2(r[2]));
        break;
      case ADDS:
           Proc.get_Sp_ref(r[0]).add(Proc.read_Sp(r[1]),Proc.read_Sp(r[2]));
        break;
      case GADDS:
           Proc.get_S2_ref(r[0]).add(Proc.read_S2(r[1]),Proc.read_S2(r[2]));
        break;
      case ADDM:
           Proc.get_Sp_ref(r[0]).add(Proc.read_Sp(r[1]),Proc.read_Cp(r[2]),Proc.P.my_num(),Proc.MCp.get_alphai());
        break;
      case GADDM:
           Proc.get_S2_ref(r[0]).add(Proc.read_S2(r[1]),Proc.read_C2(r[2]),Proc.P.my_num(),Proc.MC2.get_alphai());
        break;
      case SUBC:
          Proc.get_Cp_ref(r[0]).sub(Proc.read_Cp(r[1]),Proc.read_Cp(r[2]));
        break;
      case GSUBC:
          Proc.get_C2_ref(r[0]).sub(Proc.read_C2(r[1]),Proc.read_C2(r[2]));
        break;
      case SUBS:
           Proc.get_Sp_ref(r[0]).sub(Proc.read_Sp(r[1]),Proc.read_Sp(r[2]));
        break;
      case GSUBS:
           Proc.get_S2_ref(r[0]).sub(Proc.read_S2(r[1]),Proc.read_S2(r[2]));
        break;
      case SUBML:
           Proc.get_Sp_ref(r[0]).sub(Proc.read_Sp(r[1]),Proc.read_Cp(r[2]),Proc.P.my_num(),Proc.MCp.get_alphai());
        break;
      case GSUBML:
           Proc.get_S2_ref(r[0]).sub(Proc.read_S2(r[1]),Proc.read_C2(r[2]),Proc.P.my_num(),Proc.MC2.get_alphai());
        break;
      case SUBMR:
           Proc.get_Sp_ref(r[0]).sub(Proc.read_Cp(r[1]),Proc.read_Sp(r[2]),Proc.P.my_num(),Proc.MCp.get_alphai());
        break;
      case GSUBMR:
           Proc.get_S2_ref(r[0]).sub(Proc.read_C2(r[1]),Proc.read_S2(r[2]),Proc.P.my_num(),Proc.MC2.get_alphai());
        break;
      case MULC:
          Proc.get_Cp_ref(r[0]).mul(Proc.read_Cp(r[1]),Proc.read_Cp(r[2]));
        break;
      case GMULC:
          Proc.get_C2_ref(r[0]).mul(Proc.read_C2(r[1]),Proc.read_C2(r[2]));
        break;
      case MULM:
           Proc.get_Sp_ref(r[0]).mul(Proc.read_Sp(r[1]),Proc.read_Cp(r[2]));
        break;
      case GMULM:
           Proc.get_S2_ref(r[0]).mul(Proc.read_S2(r[1]),Proc.read_C2(r[2]));
        break;
      case DIVC:
        if (Proc.read_Cp(r[2]).is_zero())
          throw Processor_Error("Division by zero from register");
        Proc.temp.ansp.invert(Proc.read_Cp(r[2]));
        Proc.temp.ansp.mul(Proc.read_Cp(r[1]));
        Proc.write_Cp(r[0],Proc.temp.ansp);
        break;
      case GDIVC:
        if (Proc.read_C2(r[2]).is_zero())
          throw Processor_Error("Division by zero from register");
        Proc.temp.ans2.invert(Proc.read_C2(r[2]));
        Proc.temp.ans2.mul(Proc.read_C2(r[1]));
        Proc.write_C2(r[0],Proc.temp.ans2);
        break;
      case MODC:
        to_bigint(Proc.temp.aa, Proc.read_Cp(r[1]));
        to_bigint(Proc.temp.aa2, Proc.read_Cp(r[2]));
        mpz_fdiv_r(Proc.temp.aa.get_mpz_t(), Proc.temp.aa.get_mpz_t(), Proc.temp.aa2.get_mpz_t());
        Proc.temp.ansp.convert_destroy(Proc.temp.aa);
        Proc.write_Cp(r[0],Proc.temp.ansp);
        break;
      case LEGENDREC:
        to_bigint(Proc.temp.aa, Proc.read_Cp(r[1]));
        Proc.temp.aa = mpz_legendre(Proc.temp.aa.get_mpz_t(), sint::clear::pr().get_mpz_t());
        to_gfp(Proc.temp.ansp, Proc.temp.aa);
        Proc.write_Cp(r[0], Proc.temp.ansp);
        break;
      case DIGESTC:
      {
        octetStream o;
        to_bigint(Proc.temp.aa, Proc.read_Cp(r[1]));

        to_gfp(Proc.temp.ansp, Proc.temp.aa);
        Proc.temp.ansp.pack(o);
        // keep first n bytes
        to_gfp(Proc.temp.ansp, o.check_sum(n));
        Proc.write_Cp(r[0], Proc.temp.ansp);
      }
        break;
      case DIVCI:
        if (n == 0)
          throw Processor_Error("Division by immediate zero");
        bigint::tmp = n;
        to_gfp(Proc.temp.ansp, bigint::tmp);
        Proc.temp.ansp.invert();
        Proc.temp.ansp.mul(Proc.read_Cp(r[1]));
        Proc.write_Cp(r[0],Proc.temp.ansp);
        break;
      case GDIVCI:
        if (n == 0)
          throw Processor_Error("Division by immediate zero");
        Proc.temp.ans2.assign(n);
        Proc.temp.ans2.invert();
        Proc.temp.ans2.mul(Proc.read_C2(r[1]));
        Proc.write_C2(r[0],Proc.temp.ans2);
        break;
      case INV2M:
        if (Proc.inverses2m.find(n) == Proc.inverses2m.end())
          {
            to_gfp(Proc.inverses2m[n], bigint(1) << n);
            Proc.inverses2m[n].invert();
          }
        Proc.write_Cp(r[0], Proc.inverses2m[n]);
        break;
      case MODCI:
        to_bigint(Proc.temp.aa, Proc.read_Cp(r[1]));
        to_gfp(Proc.temp.ansp, Proc.temp.aa2 = mpz_fdiv_ui(Proc.temp.aa.get_mpz_t(), n));
        Proc.write_Cp(r[0],Proc.temp.ansp);
        break;
      case GMULBITC:
          Proc.get_C2_ref(r[0]).mul_by_bit(Proc.read_C2(r[1]),Proc.read_C2(r[2]));
        break;
      case GMULBITM:
          Proc.get_S2_ref(r[0]).mul_by_bit(Proc.read_S2(r[1]),Proc.read_C2(r[2]));
        break;
      case ADDCI:
        Proc.temp.assign_ansp(n);
           Proc.get_Cp_ref(r[0]).add(Proc.temp.ansp,Proc.read_Cp(r[1]));
        break;
      case GADDCI:
        Proc.temp.ans2.assign(n);
           Proc.get_C2_ref(r[0]).add(Proc.temp.ans2,Proc.read_C2(r[1]));
        break;
      case ADDSI:
        Proc.temp.assign_ansp(n);
           Proc.get_Sp_ref(r[0]).add(Proc.read_Sp(r[1]),Proc.temp.ansp,Proc.P.my_num(),Proc.MCp.get_alphai());
        break;
      case GADDSI:
        Proc.temp.ans2.assign(n);
           Proc.get_S2_ref(r[0]).add(Proc.read_S2(r[1]),Proc.temp.ans2,Proc.P.my_num(),Proc.MC2.get_alphai());
        break;
      case SUBCI:
        Proc.temp.assign_ansp(n);
           Proc.get_Cp_ref(r[0]).sub(Proc.read_Cp(r[1]),Proc.temp.ansp);
        break;
      case GSUBCI:
        Proc.temp.ans2.assign(n);
           Proc.get_C2_ref(r[0]).sub(Proc.read_C2(r[1]),Proc.temp.ans2);
        break;
      case SUBSI:
        Proc.temp.assign_ansp(n);
           Proc.get_Sp_ref(r[0]).sub(Proc.read_Sp(r[1]),Proc.temp.ansp,Proc.P.my_num(),Proc.MCp.get_alphai());
        break;
      case GSUBSI:
        Proc.temp.ans2.assign(n);
           Proc.get_S2_ref(r[0]).sub(Proc.read_S2(r[1]),Proc.temp.ans2,Proc.P.my_num(),Proc.MC2.get_alphai());
        break;
      case SUBCFI:
        Proc.temp.assign_ansp(n);
           Proc.get_Cp_ref(r[0]).sub(Proc.temp.ansp,Proc.read_Cp(r[1]));
        break;
      case GSUBCFI:
        Proc.temp.ans2.assign(n);
           Proc.get_C2_ref(r[0]).sub(Proc.temp.ans2,Proc.read_C2(r[1]));
        break;
      case SUBSFI:
        Proc.temp.assign_ansp(n);
           Proc.get_Sp_ref(r[0]).sub(Proc.temp.ansp,Proc.read_Sp(r[1]),Proc.P.my_num(),Proc.MCp.get_alphai());
        break;
      case GSUBSFI:
        Proc.temp.ans2.assign(n);
           Proc.get_S2_ref(r[0]).sub(Proc.temp.ans2,Proc.read_S2(r[1]),Proc.P.my_num(),Proc.MC2.get_alphai());
        break;
      case MULCI:
        Proc.temp.assign_ansp(n);
           Proc.get_Cp_ref(r[0]).mul(Proc.temp.ansp,Proc.read_Cp(r[1]));
        break;
      case GMULCI:
        Proc.temp.ans2.assign(n);
           Proc.get_C2_ref(r[0]).mul(Proc.temp.ans2,Proc.read_C2(r[1]));
        break;
      case MULSI:
        Proc.temp.assign_ansp(n);
           Proc.get_Sp_ref(r[0]).mul(Proc.read_Sp(r[1]),Proc.temp.ansp);
        break;
      case GMULSI:
        Proc.temp.ans2.assign(n);
           Proc.get_S2_ref(r[0]).mul(Proc.read_S2(r[1]),Proc.temp.ans2);
        break;
      case TRIPLE:
        Procp.DataF.get_three(DATA_TRIPLE, Proc.get_Sp_ref(r[0]),Proc.get_Sp_ref(r[1]),Proc.get_Sp_ref(r[2]));
        break;
      case GTRIPLE:
        Proc2.DataF.get_three(DATA_TRIPLE, Proc.get_S2_ref(r[0]),Proc.get_S2_ref(r[1]),Proc.get_S2_ref(r[2]));
        break;
      case GBITTRIPLE:
        Proc2.DataF.get_three(DATA_BITTRIPLE, Proc.get_S2_ref(r[0]),Proc.get_S2_ref(r[1]),Proc.get_S2_ref(r[2]));
        break;
      case GBITGF2NTRIPLE:
        Proc2.DataF.get_three(DATA_BITGF2NTRIPLE, Proc.get_S2_ref(r[0]),Proc.get_S2_ref(r[1]),Proc.get_S2_ref(r[2]));
        break;
      case SQUARE:
        Procp.DataF.get_two(DATA_SQUARE, Proc.get_Sp_ref(r[0]),Proc.get_Sp_ref(r[1]));
        break;
      case GSQUARE:
        Proc2.DataF.get_two(DATA_SQUARE, Proc.get_S2_ref(r[0]),Proc.get_S2_ref(r[1]));
        break;
      case BIT:
        Procp.DataF.get_one(DATA_BIT, Proc.get_Sp_ref(r[0]));
        break;
      case GBIT:
        Proc2.DataF.get_one(DATA_BIT, Proc.get_S2_ref(r[0]));
        break;
      case INV:
        Procp.DataF.get_two(DATA_INVERSE, Proc.get_Sp_ref(r[0]),Proc.get_Sp_ref(r[1]));
        break;
      case GINV:
        Proc2.DataF.get_two(DATA_INVERSE, Proc.get_S2_ref(r[0]),Proc.get_S2_ref(r[1]));
        break;
      case INPUTMASK:
        Procp.DataF.get_input(Proc.get_Sp_ref(r[0]), Proc.temp.rrp, n);
        if (n == Proc.P.my_num())
          Proc.temp.rrp.output(Proc.private_output, false);
        break;
      case GINPUTMASK:
        Proc2.DataF.get_input(Proc.get_S2_ref(r[0]), Proc.temp.ans2, n);
        if (n == Proc.P.my_num())
          Proc.temp.ans2.output(Proc.private_output, false);
        break;
      case INPUT:
        sint::Input::template input<IntInput<typename sint::clear>>(Proc.Procp, start, size);
        return;
      case GINPUT:
        sgf2n::Input::template input<IntInput<typename sgf2n::clear>>(Proc.Proc2, start, size);
        return;
      case INPUTFIX:
        sint::Input::template input<FixInput>(Proc.Procp, start, size);
        return;
      case INPUTFLOAT:
        sint::Input::template input<FloatInput>(Proc.Procp, start, size);
        return;
      case INPUTMIXED:
        sint::Input::input_mixed(Proc.Procp, start, size, false);
        return;
      case INPUTMIXEDREG:
        sint::Input::input_mixed(Proc.Procp, start, size, true);
        return;
      case RAWINPUT:
        Proc.Procp.input.raw_input(Proc.Procp, start, size);
        return;
      case GRAWINPUT:
        Proc.Proc2.input.raw_input(Proc.Proc2, start, size);
        return;
      case ANDC:
           Proc.get_Cp_ref(r[0]).AND(Proc.read_Cp(r[1]),Proc.read_Cp(r[2]));
        break;
      case GANDC:
           Proc.get_C2_ref(r[0]).AND(Proc.read_C2(r[1]),Proc.read_C2(r[2]));
        break;
      case XORC:
           Proc.get_Cp_ref(r[0]).XOR(Proc.read_Cp(r[1]),Proc.read_Cp(r[2]));
        break;
      case GXORC:
           Proc.get_C2_ref(r[0]).XOR(Proc.read_C2(r[1]),Proc.read_C2(r[2]));
        break;
      case ORC:
           Proc.get_Cp_ref(r[0]).OR(Proc.read_Cp(r[1]),Proc.read_Cp(r[2]));
        break;
      case GORC:
           Proc.get_C2_ref(r[0]).OR(Proc.read_C2(r[1]),Proc.read_C2(r[2]));
        break;
      case ANDCI:
        Proc.temp.aa=n;
           Proc.get_Cp_ref(r[0]).AND(Proc.read_Cp(r[1]),Proc.temp.aa);
        break;
      case GANDCI:
        Proc.temp.ans2.assign(n);
           Proc.get_C2_ref(r[0]).AND(Proc.temp.ans2,Proc.read_C2(r[1]));
        break;
      case XORCI:
        Proc.temp.aa=n;
           Proc.get_Cp_ref(r[0]).XOR(Proc.read_Cp(r[1]),Proc.temp.aa);
        break;
      case GXORCI:
        Proc.temp.ans2.assign(n);
           Proc.get_C2_ref(r[0]).XOR(Proc.temp.ans2,Proc.read_C2(r[1]));
        break;
      case ORCI:
        Proc.temp.aa=n;
	   Proc.get_Cp_ref(r[0]).OR(Proc.read_Cp(r[1]),Proc.temp.aa);
        break;
      case GORCI:
        Proc.temp.ans2.assign(n);
	   Proc.get_C2_ref(r[0]).OR(Proc.temp.ans2,Proc.read_C2(r[1]));
        break;
      // Note: Fp version has different semantics for NOTC than GNOTC
      case NOTC:
        to_bigint(Proc.temp.aa, Proc.read_Cp(r[1]));
        mpz_com(Proc.temp.aa.get_mpz_t(), Proc.temp.aa.get_mpz_t());
        Proc.temp.aa2 = 1;
        Proc.temp.aa2 <<= n;
        Proc.temp.aa += Proc.temp.aa2;
        Proc.temp.ansp.convert_destroy(Proc.temp.aa);
        Proc.write_Cp(r[0],Proc.temp.ansp);
        break;
      case GNOTC:
           Proc.get_C2_ref(r[0]).NOT(Proc.read_C2(r[1]));
        break;
      case SHLC:
        to_bigint(Proc.temp.aa,Proc.read_Cp(r[2]));
        if (Proc.temp.aa > 63)
          throw runtime_error("too much left shift");
           Proc.get_Cp_ref(r[0]).SHL(Proc.read_Cp(r[1]),Proc.temp.aa);
        break;
      case SHRC:
        to_bigint(Proc.temp.aa,Proc.read_Cp(r[2]));
        if (Proc.temp.aa > 63)
          throw runtime_error("too much right shift");
           Proc.get_Cp_ref(r[0]).SHR(Proc.read_Cp(r[1]),Proc.temp.aa);
        break;
      case SHLCI:
           Proc.get_Cp_ref(r[0]).SHL(Proc.read_Cp(r[1]),n);
        break;
      case GSHLCI:
           Proc.get_C2_ref(r[0]).SHL(Proc.read_C2(r[1]),n);
        break;
      case SHRCI:
           Proc.get_Cp_ref(r[0]).SHR(Proc.read_Cp(r[1]),n);
        break;
      case GSHRCI:
           Proc.get_C2_ref(r[0]).SHR(Proc.read_C2(r[1]),n);
        break;
      case GBITDEC:
        for (int j = 0; j < size; j++)
          {
            gf2n::internal_type a = Proc.read_C2(r[0] + j).get();
            for (unsigned int i = 0; i < start.size(); i++)
              {
                Proc.get_C2_ref(start[i] + j) = a & 1;
                a >>= n;
              }
          }
        return;
      case GBITCOM:
        for (int j = 0; j < size; j++)
          {
            gf2n::internal_type a = 0;
            for (unsigned int i = 0; i < start.size(); i++)
              {
                a ^= Proc.read_C2(start[i] + j).get() << (i * n);
              }
            Proc.get_C2_ref(r[0] + j) = a;
          }
        return;
      case BITDECINT:
        for (int j = 0; j < size; j++)
        {
          long a = Proc.read_Ci(r[0] + j);
          for (unsigned int i = 0; i < start.size(); i++)
            {
              Proc.get_Ci_ref(start[i] + j) = (a >> i) & 1;
            }
        }
        return;
      case OPEN:
        Proc.Procp.POpen(start, Proc.P, size);
        return;
      case GOPEN:
        Proc.Proc2.POpen(start, Proc.P, size);
        return;
      case MULS:
        Proc.Procp.muls(start, size);
        return;
      case GMULS:
        Proc.Proc2.protocol.muls(start, Proc.Proc2, Proc.MC2, size);
        return;
      case MULRS:
        Proc.Procp.mulrs(start);
        return;
      case GMULRS:
        Proc.Proc2.protocol.mulrs(start, Proc.Proc2);
        return;
      case DOTPRODS:
        Proc.Procp.dotprods(start, size);
        return;
      case GDOTPRODS:
        Proc.Proc2.dotprods(start, size);
        return;
      case MATMULS:
        Proc.Procp.matmuls(Proc.Procp.get_S(), *this, r[1], r[2]);
        return;
      case MATMULSM:
        Proc.Procp.matmulsm(Proc.machine.Mp.MS, *this, Proc.read_Ci(r[1]),
            Proc.read_Ci(r[2]));
        return;
      case CONV2DS:
        Proc.Procp.conv2ds(*this);
        return;
      case TRUNC_PR:
        Proc.Procp.protocol.trunc_pr(start, size, Proc.Procp);
        return;
      case JMP:
        Proc.PC += (signed int) n;
        break;
      case JMPI:
        Proc.PC += (signed int) Proc.read_Ci(r[0]);
        break;
      case JMPNZ:
        if (Proc.read_Ci(r[0]) != 0)
          { Proc.PC += (signed int) n; }
        break;
      case JMPEQZ:
        if (Proc.read_Ci(r[0]) == 0)
          { Proc.PC += (signed int) n; }
        break;
      case EQZC:
        if (Proc.read_Ci(r[1]) == 0)
          Proc.write_Ci(r[0], 1);
        else
          Proc.write_Ci(r[0], 0);
        break;
      case LTZC:
        if (Proc.read_Ci(r[1]) < 0)
          Proc.write_Ci(r[0], 1);
        else
          Proc.write_Ci(r[0], 0);
       break;
      case LTC:
        if (Proc.read_Ci(r[1]) < Proc.read_Ci(r[2]))
          Proc.write_Ci(r[0], 1);
        else
          Proc.write_Ci(r[0], 0);
        break;
      case GTC:
        if (Proc.read_Ci(r[1]) > Proc.read_Ci(r[2]))
          Proc.write_Ci(r[0], 1);
        else
          Proc.write_Ci(r[0], 0);
        break;
      case EQC:
        if (Proc.read_Ci(r[1]) == Proc.read_Ci(r[2]))
          Proc.write_Ci(r[0], 1);
        else
          Proc.write_Ci(r[0], 0);
        break;
      case LDINT:
        Proc.write_Ci(r[0], n);
        break;
      case ADDINT:
        Proc.get_Ci_ref(r[0]) = Proc.read_Ci(r[1]) + Proc.read_Ci(r[2]);
        break;
      case SUBINT:
        Proc.get_Ci_ref(r[0]) = Proc.read_Ci(r[1]) - Proc.read_Ci(r[2]);
        break;
      case MULINT:
        Proc.get_Ci_ref(r[0]) = Proc.read_Ci(r[1]) * Proc.read_Ci(r[2]);
        break;
      case DIVINT:
        Proc.get_Ci_ref(r[0]) = Proc.read_Ci(r[1]) / Proc.read_Ci(r[2]);
        break;
      case CONVINT:
        Proc.temp.assign_ansp(Proc.read_Ci(r[1]));
        Proc.get_Cp_ref(r[0]) = Proc.temp.ansp;
        break;
      case GCONVINT:
        Proc.get_C2_ref(r[0]).assign((word)Proc.read_Ci(r[1]));
        break;
      case CONVMODP:
        if (n == 0)
        {
          Proc.write_Ci(r[0],
            Integer::convert_unsigned(Proc.read_Cp(r[1])).get());
        }
        else
        {
          if (n > 64)
            throw Processor_Error(to_string(n) + "-bit conversion impossible; "
                "integer registers only have 64 bits");
          Proc.write_Ci(r[0], Integer(Proc.read_Cp(r[1]), n).get());
        }
        break;
      case GCONVGF2N:
        Proc.write_Ci(r[0], Proc.read_C2(r[1]).get_word());
        break;
      case PRINTMEM:
	  { Proc.out << "Mem[" <<  r[0] << "] = " << Proc.machine.Mp.read_C(r[0]) << endl; }
        break;
      case GPRINTMEM:
	  { Proc.out << "Mem[" <<  r[0] << "] = " << Proc.machine.M2.read_C(r[0]) << endl; }
        break;
      case PRINTREG:
           {
             Proc.out << "Reg[" << r[0] << "] = " << Proc.read_Cp(r[0])
              << " # " << string((char*)&n,sizeof(n)) << endl;
           }
        break;
      case GPRINTREG:
           {
             Proc.out << "Reg[" << r[0] << "] = " << Proc.read_C2(r[0])
              << " # " << string((char*)&n,sizeof(n)) << endl;
           }
        break;
      case PRINTREGPLAIN:
           {
             Proc.out << Proc.read_Cp(r[0]) << flush;
           }
        break;
      case CONDPRINTPLAIN:
        if (not Proc.read_Cp(r[0]).is_zero())
          {
            auto v = Proc.read_Cp(r[1]);
            auto p = Proc.read_Cp(r[2]);
            if (p.is_zero())
              Proc.out << v << flush;
            else
              Proc.out << bigint::get_float(v, p, {}, {}) << flush;
          }
        break;
      case GPRINTREGPLAIN:
           {
             Proc.out << Proc.read_C2(r[0]) << flush;
           }
        break;
      case PRINTINT:
           {
             Proc.out << Proc.read_Ci(r[0]) << flush;
           }
        break;
      case PRINTFLOATPLAIN:
          {
            auto nan = Proc.read_Cp(start[4]);
            typename sint::clear v = Proc.read_Cp(start[0]);
            typename sint::clear p = Proc.read_Cp(start[1]);
            typename sint::clear z = Proc.read_Cp(start[2]);
            typename sint::clear s = Proc.read_Cp(start[3]);
            bigint::output_float(Proc.out, bigint::get_float(v, p, z, s), nan);
          }
      break;
      case PRINTFLOATPREC:
        Proc.out << setprecision(n);
        break;
      case PRINTSTR:
           {
             Proc.out << string((char*)&n,sizeof(n)) << flush;
           }
        break;
      case CONDPRINTSTR:
          if (not Proc.read_Cp(r[0]).is_zero())
            {
              string str = {(char*)&n, sizeof(n)};
              size_t n = str.find('\0');
              if (n < 4)
                str.erase(n);
              Proc.out << str << flush;
            }
        break;
      case PRINTCHR:
           {
             Proc.out << string((char*)&n,1) << flush;
           }
        break;
      case PRINTCHRINT:
           {
             Proc.out << string((char*)&(Proc.read_Ci(r[0])),1) << flush;
           }
        break;
      case PRINTSTRINT:
           {
             Proc.out << string((char*)&(Proc.read_Ci(r[0])),sizeof(int)) << flush;
           }
        break;
      case RAND:
        Proc.write_Ci(r[0], Proc.shared_prng.get_uint() % (1 << Proc.read_Ci(r[1])));
        break;
      case REQBL:
      case GREQBL:
      case USE:
      case USE_INP:
      case USE_EDABIT:
      case USE_PREP:
      case GUSE_PREP:
        break;
      case TIME:
        Proc.machine.time();
	break;
      case START:
        Proc.machine.start(n);
        break;
      case STOP:
        Proc.machine.stop(n);
        break;
      case RUN_TAPE:
        Proc.DataF.skip(
            Proc.machine.run_tape(r[0], n, r[1], -1, &Proc.DataF.DataFp,
                &Proc.share_thread.DataF));
        break;
      case JOIN_TAPE:
        Proc.machine.join_tape(r[0]);
        break;
      case CRASH:
        throw crash_requested();
        break;
      case STARTGRIND:
        CALLGRIND_START_INSTRUMENTATION;
        break;
      case STOPGRIND:
        CALLGRIND_STOP_INSTRUMENTATION;
        break;
      case NPLAYERS:
        Proc.write_Ci(r[0], Proc.P.num_players());
        break;
      case THRESHOLD:
        Proc.write_Ci(r[0], sint::threshold(Proc.P.num_players()));
        break;
      case PLAYERID:
        Proc.write_Ci(r[0], Proc.P.my_num());
        break;
      // ***
      // TODO: read/write shared GF(2^n) data instructions
      // ***
      case LISTEN:
        // listen for connections at port number n
        Proc.external_clients.start_listening(n);
        break;
      case ACCEPTCLIENTCONNECTION:
      {
        // get client connection at port number n + my_num())
        int client_handle = Proc.external_clients.get_client_connection(n);
        if (client_handle == -1)
        {
          stringstream ss;
          ss << "No connection on port " << r[0] << endl;
          throw Processor_Error(ss.str());
        }
        if (Proc.P.my_num() == 0)
        {
          octetStream os;
          os.store(int(sint::open_type::type_char()));
          sint::open_type::specification(os);
          os.Send(Proc.external_clients.get_socket(client_handle));
        }
        Proc.write_Ci(r[0], client_handle);
        break;
      }
      case READSOCKETINT:
        Proc.read_socket_ints(Proc.read_Ci(r[0]), start);
        break;
      case READSOCKETC:
        Proc.read_socket_vector(Proc.read_Ci(r[0]), start);
        break;
      case READSOCKETS:
        // read shares and MAC shares
        Proc.read_socket_private(Proc.read_Ci(r[0]), start, true);
        break;
      case GREADSOCKETS:
        //Proc.get_S2_ref(r[0]).get_share().pack(socket_octetstream);
        //Proc.get_S2_ref(r[0]).get_mac().pack(socket_octetstream);
        break;
      case WRITESOCKETINT:
        Proc.write_socket(INT, CLEAR, false, Proc.read_Ci(r[0]), r[1], start);
        break;
      case WRITESOCKETC:
        Proc.write_socket(MODP, CLEAR, false, Proc.read_Ci(r[0]), r[1], start);
        break;
      case WRITESOCKETS:
        // Send shares + MACs
        Proc.write_socket(MODP, SECRET, true, Proc.read_Ci(r[0]), r[1], start);
        break;
      case WRITESOCKETSHARE:
        // Send only shares, no MACs
        // N.B. doesn't make sense to have a corresponding read instruction for this
        Proc.write_socket(MODP, SECRET, false, Proc.read_Ci(r[0]), r[1], start);
        break;
      /*case GWRITESOCKETS:
        Proc.get_S2_ref(r[0]).get_share().pack(socket_octetstream);
        Proc.get_S2_ref(r[0]).get_mac().pack(socket_octetstream);
        break;*/
      case WRITEFILESHARE:
        // Write shares to file system
        Proc.write_shares_to_file(start);
        break;
      case READFILESHARE:
        // Read shares from file system
        Proc.read_shares_from_file(Proc.read_Ci(r[0]), r[1], start);
        break;        
      case PUBINPUT:
        Proc.public_input >> Proc.get_Ci_ref(r[0]);
        break;
      case RAWOUTPUT:
        Proc.read_Cp(r[0]).output(Proc.public_output, false);
        break;
      case GRAWOUTPUT:
        Proc.read_C2(r[0]).output(Proc.public_output, false);
        break;
      case STARTPRIVATEOUTPUT:
        Proc.privateOutputp.start(n,r[0],r[1]);
        break;
      case GSTARTPRIVATEOUTPUT:
        Proc.privateOutput2.start(n,r[0],r[1]);
        break;
      case STOPPRIVATEOUTPUT:
        Proc.privateOutputp.stop(n,r[0],r[1]);
        break;
      case GSTOPPRIVATEOUTPUT:
        Proc.privateOutput2.stop(n,r[0],r[1]);
        break;
      case PREP:
        Procp.DataF.get(Proc.Procp.get_S(), r, start, size);
        return;
      case GPREP:
        Proc2.DataF.get(Proc.Proc2.get_S(), r, start, size);
        return;
      default:
        printf("Case of opcode=0x%x not implemented yet\n",opcode);
        throw runtime_error("invalid opcode: " + to_string(opcode));
        break;
#define X(NAME, CODE) case NAME: throw runtime_error("wrong case statement"); return;
        COMBI_INSTRUCTIONS
#undef X
    }
  if (size > 1)
    {
      r[0]++; r[1]++; r[2]++;
    }
  }
}

template<class sint, class sgf2n>
void Program::execute(Processor<sint, sgf2n>& Proc) const
{
  unsigned int size = p.size();
  Proc.PC=0;
  while (Proc.PC<size)
    { p[Proc.PC].execute(Proc); }
}

#endif
