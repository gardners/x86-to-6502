#include <iostream>
#include <vector>
#include <set>
#include <string>
#include <regex>
#include <cassert>
#include <map>
#include <cctype>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>

// 1 for MEGA65 CPU, 0 for normal 6502
int cpu_45gs02=0;

struct Operand
{
  enum class Type
  {
    empty,
    literal,
    reg /*ister*/
  };

  Type type = Type::empty;
  int reg_num = 0;
  std::string value;

  Operand() = default;

  bool operator==(const Operand &other) const {
    return type == other.type && reg_num == other.reg_num && value == other.value;
  }

  Operand(const Type t, std::string v)
    : type(t), value(std::move(v))
  {
    assert(type == Type::literal);
  }

  Operand(const Type t, const int num)
    : type(t), reg_num(num)
  {
    assert(type == Type::reg);
  }
};

struct ASMLine
{
  enum class Type
  {
    Label,
    Instruction,
    Directive
  };

  ASMLine(Type t, std::string te) : type(t), text(std::move(te)) {}

  Type type;
  std::string text;
};


struct mos6502 : ASMLine
{
  enum class OpCode 
  {
    unknown,
    lda,
      ldx,
    ldy,
    tay,
    tya,
      tax,
      txa,
      nop,
    cpy,
    eor,
    sta,
    sty,
    pha,
    pla,
    php,
    plp,
    lsr,
    ror,
    AND,
    inc,
    dec,
    ORA,
    cmp,
    bne,
    beq,
    bmi,
    bpl,
    bcc,
    bcs,
    jmp,
    adc,
    sbc,
    rts,
    clc,
    sec,
      bit,
    jsr,

    // 45GS02 / 4510 opcodes
      ldz,
      taz,
      tza,
      neg

      };

  static bool get_is_branch(const OpCode o) {
    switch (o) {
      case OpCode::beq:
      case OpCode::bne:
      case OpCode::bmi:
      case OpCode::bpl:
      case OpCode::bcc:
      case OpCode::bcs:
        return true;
      case OpCode::lda:
      case OpCode::ldx:
      case OpCode::ldy:
      case OpCode::tay:
      case OpCode::tya:
      case OpCode::tax:
      case OpCode::txa:
      case OpCode::nop:
      case OpCode::cpy:
      case OpCode::eor:
      case OpCode::sta:
      case OpCode::sty:
      case OpCode::pha:
      case OpCode::pla:
      case OpCode::php:
      case OpCode::plp:
      case OpCode::lsr:
      case OpCode::ror:
      case OpCode::AND:
      case OpCode::inc:
      case OpCode::dec:
      case OpCode::ORA:
      case OpCode::cmp:
      case OpCode::jmp:
      case OpCode::jsr:
      case OpCode::adc:
      case OpCode::sbc:
      case OpCode::rts:
      case OpCode::clc:
      case OpCode::sec:
      case OpCode::bit:

	// 45GS02 / 4510 opcodes
    case OpCode::ldz:
    case OpCode::taz:
    case OpCode::tza:
    case OpCode::neg:
	

    case OpCode::unknown:
        break;
    }
    return false;
  }

  static bool get_is_comparison(const OpCode o) {
    switch (o) {
      case OpCode::adc:
      case OpCode::sbc:
      case OpCode::cmp:
      case OpCode::cpy:
      case OpCode::bit:
        return true;
      case OpCode::lda:
      case OpCode::ldx:
      case OpCode::ldy:
      case OpCode::tay:
      case OpCode::tya:
      case OpCode::tax:
      case OpCode::txa:
      case OpCode::nop:
      case OpCode::eor:
      case OpCode::sta:
      case OpCode::sty:
      case OpCode::pha:
      case OpCode::pla:
      case OpCode::php:
      case OpCode::plp:
      case OpCode::lsr:
      case OpCode::ror:
      case OpCode::AND:
      case OpCode::inc:
      case OpCode::dec:
      case OpCode::ORA:
      case OpCode::jmp:
      case OpCode::jsr:
      case OpCode::bne:
      case OpCode::bmi:
      case OpCode::beq:
      case OpCode::bpl:
      case OpCode::bcc:
      case OpCode::bcs:
      case OpCode::rts:
      case OpCode::clc:
      case OpCode::sec:

	// 45GS02 / 4510 opcodes
    case OpCode::ldz:
    case OpCode::taz:
    case OpCode::tza:
    case OpCode::neg:
	
      case OpCode::unknown:
        break;
    }
    return false;
  }


  explicit mos6502(const OpCode o)
    : ASMLine(Type::Instruction, to_string(o)), opcode(o), is_branch(get_is_branch(o)), is_comparison(get_is_comparison(o))
  {
  }

  mos6502(const Type t, std::string s)
    : ASMLine(t, std::move(s))
  {
  }

  mos6502(const OpCode o, Operand t_o)
    : ASMLine(Type::Instruction, to_string(o)), opcode(o), op(std::move(t_o)), is_branch(get_is_branch(o)), is_comparison(get_is_comparison(o))
  {
  }

  static std::string to_string(const OpCode o)
  {
    switch (o) {
      case OpCode::lda:
        return "lda";
      case OpCode::ldx:
        return "ldx";
      case OpCode::ldy:
        return "ldy";
      case OpCode::ldz:
        return "ldz";
      case OpCode::tax:
        return "tax";
      case OpCode::txa:
        return "txa";
      case OpCode::tay:
        return "tay";
      case OpCode::tya:
        return "tya";
      case OpCode::taz:
        return "taz";
      case OpCode::tza:
        return "tza";
      case OpCode::cpy:
        return "cpy";
      case OpCode::eor:
        return "eor";
      case OpCode::sta:
        return "sta";
      case OpCode::sty:
        return "sty";
      case OpCode::pha:
        return "pha";
      case OpCode::pla:
        return "pla";
      case OpCode::php:
        return "php";
      case OpCode::plp:
        return "plp";
      case OpCode::lsr:
        return "lsr";
      case OpCode::ror:
        return "ror";
      case OpCode::AND:
        return "and";
      case OpCode::inc:
        return "inc";
      case OpCode::dec:
        return "dec";
      case OpCode::ORA:
        return "ora";
      case OpCode::cmp:
        return "cmp";
      case OpCode::bne:
        return "bne";
      case OpCode::bmi:
        return "bmi";
      case OpCode::bpl:
        return "bpl";
      case OpCode::bcc:
        return "bcc";
      case OpCode::bcs:
        return "bcs";
      case OpCode::beq:
        return "beq";
      case OpCode::jmp:
        return "jmp";
      case OpCode::jsr:
        return "jsr";
      case OpCode::adc:
        return "adc";
      case OpCode::sbc:
        return "sbc";
      case OpCode::rts:
        return "rts";
      case OpCode::clc:
        return "clc";
      case OpCode::sec:
        return "sec";
      case OpCode::bit:
        return "bit";
      case OpCode::neg:
        return "neg";
      case OpCode::nop:
        return "nop";
      case OpCode::unknown:
        return "";
    };

    return "";
  }

  std::string to_string() const
  {
    switch (type) {
      case ASMLine::Type::Label:
        return text + ':';
      case ASMLine::Type::Directive:
      case ASMLine::Type::Instruction:
        {
          const std::string line = '\t' + text + ' ' + op.value;
          return line + std::string(static_cast<size_t>(std::max(15 - static_cast<int>(line.size()), 1)), ' ') + "; " + comment;
        }
    };
    throw std::runtime_error("Unable to render: " + text);
  }


  OpCode opcode = OpCode::unknown;
  Operand op;
  std::string comment;
  bool is_branch = false;
  bool is_comparison = false;
};


struct i386 : ASMLine
{
  enum class OpCode 
  {
    unknown,
    movzbl,
    movzwl,
    shrb,
    shrl,
    xorl,
    andl,
    andb,
    addb,
    ret,
    movb,
    call,
    cmpb,
    cmpl,
    movl,
    jmp,
    jne,
    je,
    jg,
    js,
    testb,
    incl,
    incb,      
    decl,
    decb,
    sarl,
    addl,
    subl,
    subb,
    sall,
    orl,
    orb,
    rep,
    pushl,
    sbbb,
    negb,
    notb,
    retl
  };

  static OpCode parse_opcode(Type t, const std::string &o);
  static Operand parse_operand(std::string o);

  i386(const int t_line_num, std::string t_line_text, Type t, std::string t_opcode, std::string o1="", std::string o2="");

  int line_num;
  std::string line_text;
  OpCode opcode;
  Operand operand1;
  Operand operand2;
};


int parse_8bit_literal(const std::string &s)
{
  return std::stoi(std::string(std::next(std::begin(s)), std::end(s)));
}



Operand get_register(const int reg_num_in, const int offset = 0) {

  const int reg_num = reg_num_in + offset;
  
  switch (reg_num) {
    // Each 386 register is really 4 bytes wide, so we need to have these
    // in groups of four, so that we can do clever efficient things
    // (especially for the MEGA65's GS4502 CPU that can do 32-bit accesses
    
    //  Usable ZP locations extracted from: http://sta.c64.org/cbm64mem.html
    
    // EAX
  case 0x00: return Operand(Operand::Type::literal, "$03"); // unused, fp->int routine pointer
  case 0x01: return Operand(Operand::Type::literal, "$04");
  case 0x02: return Operand(Operand::Type::literal, "$05"); // unused, int->fp routine pointer
  case 0x03: return Operand(Operand::Type::literal, "$06");
    
    // EBX
  case 0x04: return Operand(Operand::Type::literal, "$fb"); // unused 
  case 0x05: return Operand(Operand::Type::literal, "$fc"); // unused
  case 0x06: return Operand(Operand::Type::literal, "$fd"); // unused
  case 0x07: return Operand(Operand::Type::literal, "$fe"); // unused
    
    // ECX
  case 0x08: return Operand(Operand::Type::literal, "$19"); // String stack
  case 0x09: return Operand(Operand::Type::literal, "$1A"); // String stack
  case 0x0A: return Operand(Operand::Type::literal, "$1B"); // String stack
  case 0x0B: return Operand(Operand::Type::literal, "$1C"); // String stack

    // EDX
  case 0x0C: return Operand(Operand::Type::literal, "$1D"); // String stack
  case 0x0D: return Operand(Operand::Type::literal, "$1E"); // String stack
  case 0x0E: return Operand(Operand::Type::literal, "$1F"); // String stack
  case 0x0F: return Operand(Operand::Type::literal, "$20"); // String stack

    // ESI
  case 0x10: return Operand(Operand::Type::literal, "$39"); // Current BASIC line number
  case 0x11: return Operand(Operand::Type::literal, "$3A"); // Current BASIC line number
  case 0x12: return Operand(Operand::Type::literal, "$3B"); // Current BASIC line number for CONT
  case 0x13: return Operand(Operand::Type::literal, "$3C"); // Current BASIC line number for CONT

    // EDI
  case 0x14: return Operand(Operand::Type::literal, "$A7"); // RS232 things
  case 0x15: return Operand(Operand::Type::literal, "$A8"); // RS232 things
  case 0x16: return Operand(Operand::Type::literal, "$A9"); // RS232 things
  case 0x17: return Operand(Operand::Type::literal, "$AA"); // RS232 things

    
  };
  throw std::runtime_error("Unhandled register number: " + std::to_string(reg_num));
}

std::string fixup_8bit_literal(const std::string &s)
{
  if (!strcmp(s.c_str(),"%eax")) return get_register(i386::parse_operand(s).reg_num,0).value;
  if (!strcmp(s.c_str(),"(%eax)")) return "(" + get_register(0,0).value + ")";

  if (s[0] == '$')
  {
    return "#" + std::to_string(static_cast<uint8_t>(parse_8bit_literal(s)));
  } else {
    return s;
  }
}

i386::OpCode i386::parse_opcode(Type t, const std::string &o)
  {
    switch(t)
    {
      case Type::Label:
        return OpCode::unknown;
      case Type::Directive:
        return OpCode::unknown;
      case Type::Instruction:
        {
          if (o == "movzwl") return OpCode::movzwl;
          if (o == "movzbl") return OpCode::movzbl;
          if (o == "shrb") return OpCode::shrb;
          if (o == "shrl") return OpCode::shrl;
          if (o == "xorl") return OpCode::xorl;
          if (o == "andl") return OpCode::andl;
          if (o == "ret") return OpCode::ret;
          if (o == "movb") return OpCode::movb;
          if (o == "call") return OpCode::call;
          if (o == "cmpb") return OpCode::cmpb;
          if (o == "cmpl") return OpCode::cmpl;
          if (o == "movl") return OpCode::movl;
          if (o == "jmp") return OpCode::jmp;
          if (o == "testb") return OpCode::testb;
          if (o == "incl") return OpCode::incl;
          if (o == "sarl") return OpCode::sarl;
          if (o == "decl") return OpCode::decl;
          if (o == "jne") return OpCode::jne;
          if (o == "je") return OpCode::je;
          if (o == "jg") return OpCode::jg;
          if (o == "js") return OpCode::js;
          if (o == "subl") return OpCode::subl;
          if (o == "subb") return OpCode::subb;
          if (o == "addl") return OpCode::addl;
          if (o == "addb") return OpCode::addb;
          if (o == "sall") return OpCode::sall;
          if (o == "orl") return OpCode::orl;
          if (o == "andb") return OpCode::andb;
          if (o == "orb") return OpCode::orb;
          if (o == "decb") return OpCode::decb;
          if (o == "incb") return OpCode::incb;
          if (o == "rep") return OpCode::rep;
          if (o == "notb") return OpCode::notb;
          if (o == "negb") return OpCode::negb;
          if (o == "sbbb") return OpCode::sbbb;
          if (o == "pushl") return OpCode::pushl;
          if (o == "retl") return OpCode::retl;
        }
    }
    throw std::runtime_error("Unknown opcode: " + o);
  }

Operand i386::parse_operand(std::string o)
  {
    if (o.empty()) {
      return Operand();
    }

    if (o[0] == '%') {
      if (o == "%al" || o == "%ax" || o == "%eax") {
        return Operand(Operand::Type::reg, 0x00);
      } else if (o == "%ah") {
        return Operand(Operand::Type::reg, 0x01);
      } else if (o == "%bl" || o == "%bx" || o == "%ebx") {
        return Operand(Operand::Type::reg, 0x04);
      } else if (o == "%bh") {
        return Operand(Operand::Type::reg, 0x05);
      } else if (o == "%cl" || o == "%cx" || o == "%ecx") {
        return Operand(Operand::Type::reg, 0x08);
      } else if (o == "%ch") {
        return Operand(Operand::Type::reg, 0x09);
      } else if (o == "%dl" || o == "%dx" || o == "%edx") {
        return Operand(Operand::Type::reg, 0x0c);
      } else if (o == "%dh") {
        return Operand(Operand::Type::reg, 0x0d);
      } else if (o == "%sil" || o == "%si" || o == "%esi") {
        return Operand(Operand::Type::reg, 0x10);
      } else if (o == "%dil" || o == "%di" || o == "%edi") {
        return Operand(Operand::Type::reg, 0x14);
      } else {
        throw std::runtime_error("Unknown register operand: '" + o + "'");
      }
    } else {
      return Operand(Operand::Type::literal, std::move(o));
    }
  }

i386::i386(const int t_line_num, std::string t_line_text, Type t, std::string t_opcode, std::string o1, std::string o2)
    : ASMLine(t, t_opcode), line_num(t_line_num), line_text(std::move(t_line_text)), 
      opcode(parse_opcode(t, t_opcode)), operand1(parse_operand(o1)), operand2(parse_operand(o2))
  {
  }


void append_fetchb(std::vector<mos6502> &instructions,const Operand &o1)
{
        if (o1.value[0]=='(') {
          // Indirect argument 
          if (o1.value[1]=='%') {
            // Indirect fetch via register.
            // Only %eXX and %xX are allowed. (%xB) (writing using a point of 8-bit size) is not supported.
            // Worse, for 6502 we have only 64K address space, so we need to check if the upper half of
            // a 32-bit register is non-zero. if non-zero, don't write, so that we don't wrap the upper
            // part of the address space back onto the bottom 64KB, which would probably corrupt things.
            if (o1.value[2]=='e') {
              // 32-bit register, so use 32-bit ZP indirect as access mode.
	      if (cpu_45gs02) {	  
		instructions.emplace_back(mos6502::OpCode::nop);
		instructions.emplace_back(mos6502::OpCode::lda,Operand(o1.type,fixup_8bit_literal(o1.value)));
	      } else {
		// XXX - We can check that upper 16 bits are zero, and if so, we can do a normal 16-bit ZP indirect
		// on a 6502. We should probably implement that. Trouble is that requires run-time check, unless
		// we can prove from static analysis.
		throw std::runtime_error("Indirect fetch through 32-bit register requires 45GS02");
	      }
		       
            } else {
              throw std::runtime_error("Cannot translate movb instruction");
            }
          } else {
            throw std::runtime_error("Cannot translate movb instruction");
          }
        } else {
          instructions.emplace_back(mos6502::OpCode::lda, Operand(o1.type, fixup_8bit_literal(o1.value)));
        }
}

void append_storeb(std::vector<mos6502> &instructions,const Operand &o1)
{
        if (o1.value[0]=='(') {
          // Indirect argument 
          if (o1.value[1]=='%') {
            // Indirect store via register.
            // Only %eXX and %xX are allowed. (%xB) (writing using a point of 8-bit size) is not supported.
            // Worse, for 6502 we have only 64K address space, so we need to check if the upper half of
            // a 32-bit register is non-zero. if non-zero, don't write, so that we don't wrap the upper
            // part of the address space back onto the bottom 64KB, which would probably corrupt things.
            if (o1.value[2]=='e') {
              // 32-bit register, so use 32-bit ZP indirect as access mode.
	      if (cpu_45gs02) {
		instructions.emplace_back(mos6502::OpCode::nop);
		instructions.emplace_back(mos6502::OpCode::sta,Operand(o1.type,fixup_8bit_literal(o1.value)));
	      } else {
		throw std::runtime_error("Indirect store through 32-bit register requires 45GS02");
	      }
            } else {
              throw std::runtime_error("Cannot translate movb instruction");
            }
          } else {
            throw std::runtime_error("Cannot translate movb instruction");
          }
        } else {
          instructions.emplace_back(mos6502::OpCode::sta, Operand(o1.type, fixup_8bit_literal(o1.value)));
        }
}

void translate_instruction(std::vector<mos6502> &instructions, const i386::OpCode op, const Operand &o1, const Operand &o2)
{
  switch(op)
  {
    case i386::OpCode::ret:
      instructions.emplace_back(mos6502::OpCode::rts);
      break;
    case i386::OpCode::retl:
      /// \todo I don't know if this is completely correct for retl translation
      instructions.emplace_back(mos6502::OpCode::rts);
      break;
    case i386::OpCode::movl:
      if (o1.type == Operand::Type::reg && o2.type == Operand::Type::reg) {
	if (cpu_45gs02) {
	  // LDA32 source
	  instructions.emplace_back(mos6502::OpCode::neg);
	  instructions.emplace_back(mos6502::OpCode::neg);
	  instructions.emplace_back(mos6502::OpCode::lda, get_register(o1.reg_num));
	  // STA32 destination
	  instructions.emplace_back(mos6502::OpCode::neg);
	  instructions.emplace_back(mos6502::OpCode::neg);
	  instructions.emplace_back(mos6502::OpCode::sta, get_register(o2.reg_num));	  

	} else {
	  instructions.emplace_back(mos6502::OpCode::lda, get_register(o1.reg_num));
	  instructions.emplace_back(mos6502::OpCode::sta, get_register(o2.reg_num));
	  instructions.emplace_back(mos6502::OpCode::lda, get_register(o1.reg_num, 1));
	  instructions.emplace_back(mos6502::OpCode::sta, get_register(o2.reg_num, 1));
	  instructions.emplace_back(mos6502::OpCode::lda, get_register(o1.reg_num, 2));
	  instructions.emplace_back(mos6502::OpCode::sta, get_register(o2.reg_num, 2));
	  instructions.emplace_back(mos6502::OpCode::lda, get_register(o1.reg_num, 3));
	  instructions.emplace_back(mos6502::OpCode::sta, get_register(o2.reg_num, 3));
	}
      } else if (o1.type == Operand::Type::literal && o2.type == Operand::Type::reg) {
	if (cpu_45gs02) {
	  // Prepare the constant
	  instructions.emplace_back(mos6502::OpCode::lda, Operand(o1.type, "#<[" + o1.value+" & $FFFF]"));
	  instructions.emplace_back(mos6502::OpCode::ldx, Operand(o1.type, "#>[" + o1.value+" & $FFFF]"));
	  instructions.emplace_back(mos6502::OpCode::ldy, Operand(o1.type, "#<[" + o1.value+" >> 16]"));
	  instructions.emplace_back(mos6502::OpCode::ldz, Operand(o1.type, "#>[" + o1.value+" >> 16]"));
	  // Then issue STA32
	  instructions.emplace_back(mos6502::OpCode::neg);
	  instructions.emplace_back(mos6502::OpCode::neg);
	  instructions.emplace_back(mos6502::OpCode::sta, get_register(o2.reg_num));
	} else {
	  instructions.emplace_back(mos6502::OpCode::lda, Operand(o1.type, "#<[" + o1.value+" & $FFFF]"));
	  instructions.emplace_back(mos6502::OpCode::sta, get_register(o2.reg_num));
	  instructions.emplace_back(mos6502::OpCode::lda, Operand(o1.type, "#>[" + o1.value+" & $FFFF]"));
	  instructions.emplace_back(mos6502::OpCode::sta, get_register(o2.reg_num, 1));
	  instructions.emplace_back(mos6502::OpCode::lda, Operand(o1.type, "#<[" + o1.value+" >> 16]"));
	  instructions.emplace_back(mos6502::OpCode::sta, get_register(o2.reg_num, 2));
	  instructions.emplace_back(mos6502::OpCode::lda, Operand(o1.type, "#>[" + o1.value+" >> 16]"));
	  instructions.emplace_back(mos6502::OpCode::sta, get_register(o2.reg_num, 3));
	}
      } else {
        throw std::runtime_error("Cannot translate movl instruction");
      }
      break;
    case i386::OpCode::xorl:
      if (o1.type == Operand::Type::reg && o2.type == Operand::Type::reg
          && o1.reg_num == o2.reg_num) {
	// XOR same register clears the register contents

	if (cpu_45gs02) {
	  // For 32-bit clear, we need to set A,X,Y and Z to $00
	  instructions.emplace_back(mos6502::OpCode::lda, Operand(Operand::Type::literal, "#$00"));
	  instructions.emplace_back(mos6502::OpCode::tax);
	  instructions.emplace_back(mos6502::OpCode::tay);
	  instructions.emplace_back(mos6502::OpCode::taz);
	  // Now construct STA32
	  instructions.emplace_back(mos6502::OpCode::neg);
	  instructions.emplace_back(mos6502::OpCode::neg);
	  instructions.emplace_back(mos6502::OpCode::sta, get_register(o2.reg_num));
	} else {
	  instructions.emplace_back(mos6502::OpCode::lda, Operand(Operand::Type::literal, "#$00"));
	  instructions.emplace_back(mos6502::OpCode::sta, get_register(o2.reg_num));
	  instructions.emplace_back(mos6502::OpCode::sta, get_register(o2.reg_num, 1));
	  instructions.emplace_back(mos6502::OpCode::sta, get_register(o2.reg_num, 2));
	  instructions.emplace_back(mos6502::OpCode::sta, get_register(o2.reg_num, 3));
	}
      } else {
        throw std::runtime_error("Cannot translate xorl instruction");
      }
      break;
#if 0
    case i386::OpCode::cmpl:
      if (o1.type == Operand::Type::reg) {
	if (cpu_45gs02) {
	  instructions.emplace_back(mos6502::OpCode::neg);
	  instructions.emplace_back(mos6502::OpCode::neg);
	  instructions.emplace_back(mos6502::OpCode::cmp, get_register(o1.reg_num));
	} else {
	  static int cmpl_count=0;
	  char new_label_name[32];
	  snprintf(new_label_name,31,"__cmppl_%d",cmpl_count++);
	  instructions.emplace_back(mos6502::OpCode::cmp, get_register(o1.reg_num,3));
	  instructions.emplace_back(mos6502::OpCode::bne, Operand(Operand::Type::literal,new_label_name));
	  instructions.emplace_back(mos6502::OpCode::cmp, get_register(o1.reg_num,2));
	  instructions.emplace_back(mos6502::OpCode::bne, Operand(Operand::Type::literal,new_label_name));
	  instructions.emplace_back(mos6502::OpCode::cmp, get_register(o1.reg_num,1));
	  instructions.emplace_back(mos6502::OpCode::bne, Operand(Operand::Type::literal,new_label_name));
	  instructions.emplace_back(mos6502::OpCode::cmp, get_register(o1.reg_num,0));
	  instructions.emplace_back(ASMLine::Type::Label, new_label_name);
	}
      } else {
	if (cpu_45gs02) {
	  instructions.emplace_back(mos6502::OpCode::neg);
	  instructions.emplace_back(mos6502::OpCode::neg);
	  instructions.emplace_back(mos6502::OpCode::cmp, o1);
	} else
	  throw std::runtime_error("cmpl opcode requires 45GS02");
      }
      break;
#endif
    case i386::OpCode::movb:
      // There are lots of addressing modes for the source and destination.  We need to support
      // them all.  We use the 6502 accumulator as the intermediate storage in all cases.
      append_fetchb(instructions,o1);
      append_storeb(instructions,o2);
#if 0
      if (o1.type == Operand::Type::literal) {
        instructions.emplace_back(mos6502::OpCode::lda, Operand(o1.type, fixup_8bit_literal(o1.value)));
      } else if (o1.type == Operand::Type::reg) {
      } else {
        throw std::runtime_error("Cannot translate movb instruction");
      }
      if (o2.type == Operand::Type::literal) {
        instructions.emplace_back(mos6502::OpCode::sta, o2);
      } else if (o2.type == Operand::Type::reg) {
        instructions.emplace_back(mos6502::OpCode::lda, get_register(o1.reg_num));
        instructions.emplace_back(mos6502::OpCode::sta, get_register(o2.reg_num));
      } else {
        throw std::runtime_error("Cannot translate movb instruction");
      }
#endif
      break;
    case i386::OpCode::orb:
      if (o1.type == Operand::Type::literal && o2.type == Operand::Type::literal) {
        instructions.emplace_back(mos6502::OpCode::lda, Operand(o1.type, fixup_8bit_literal(o1.value)));
        instructions.emplace_back(mos6502::OpCode::ORA, o2);
        instructions.emplace_back(mos6502::OpCode::sta, o2);
      } else if (o1.type == Operand::Type::reg && o2.type == Operand::Type::reg) {
        instructions.emplace_back(mos6502::OpCode::lda, get_register(o1.reg_num));
        instructions.emplace_back(mos6502::OpCode::ORA, get_register(o2.reg_num));
        instructions.emplace_back(mos6502::OpCode::sta, get_register(o2.reg_num));
      } else if (o1.type == Operand::Type::literal && o2.type == Operand::Type::reg) {
        instructions.emplace_back(mos6502::OpCode::lda, Operand(o1.type, fixup_8bit_literal(o1.value)));
        instructions.emplace_back(mos6502::OpCode::ORA, get_register(o2.reg_num));
        instructions.emplace_back(mos6502::OpCode::sta, get_register(o2.reg_num));
      } else {
        throw std::runtime_error("Cannot translate orb instruction");
      }
      break;

    case i386::OpCode::movzbl:
      if (o1.type == Operand::Type::literal && o2.type == Operand::Type::reg) {
        instructions.emplace_back(mos6502::OpCode::lda, o1);
        instructions.emplace_back(mos6502::OpCode::sta, get_register(o2.reg_num));
      } else {
        throw std::runtime_error("Cannot translate movzbl instruction");
      }
      break;
    case i386::OpCode::shrb:
      if (o1.type == Operand::Type::reg || o2.type == Operand::Type::reg) {
        const auto do_shift = [&instructions](const int reg_num) {
          instructions.emplace_back(mos6502::OpCode::lsr, get_register(reg_num));
        };

        if (o1.type == Operand::Type::literal) {
          const auto count = parse_8bit_literal(o1.value);
          for (int i = 0; i < count; ++i) {
            do_shift(o2.reg_num);
          }
        } else {
          do_shift(o1.reg_num);
        }
      } else {
        throw std::runtime_error("Cannot translate shrb instruction");
      }
      break;
    case i386::OpCode::shrl:
      if (o1.type == Operand::Type::reg || o2.type == Operand::Type::reg) {
        const auto do_shift = [&instructions](const int reg_num) {
          instructions.emplace_back(mos6502::OpCode::lsr, get_register(reg_num, 1));
          instructions.emplace_back(mos6502::OpCode::ror, get_register(reg_num));
        };

        if (o1.type == Operand::Type::literal) {
          const auto count = parse_8bit_literal(o1.value);
          for (int i = 0; i < count; ++i) {
            do_shift(o2.reg_num);
          }
        } else {
          do_shift(o1.reg_num);
        }
      } else {
        throw std::runtime_error("Cannot translate shrl instruction");
      }
      break;
    case i386::OpCode::testb:
      if (o1.type == Operand::Type::reg && o2.type == Operand::Type::reg && o1.reg_num == o2.reg_num) {
        // this just tests the register for 0
        instructions.emplace_back(mos6502::OpCode::lda, get_register(o1.reg_num));
//        instructions.emplace_back(mos6502::OpCode::bit, Operand(Operand::Type::literal, "#$00"));
      } else if (o1.type == Operand::Type::reg && o2.type == Operand::Type::reg) {
        // ands the values
        instructions.emplace_back(mos6502::OpCode::lda, get_register(o1.reg_num));
        instructions.emplace_back(mos6502::OpCode::bit, get_register(o2.reg_num));
      } else if (o1.type == Operand::Type::literal && o2.type == Operand::Type::reg) {
        // ands the values
        instructions.emplace_back(mos6502::OpCode::lda, Operand(o1.type, fixup_8bit_literal(o1.value)));
        instructions.emplace_back(mos6502::OpCode::bit, get_register(o2.reg_num));
      } else if (o1.type == Operand::Type::literal && o2.type == Operand::Type::literal) {
        // ands the values
        instructions.emplace_back(mos6502::OpCode::lda, Operand(o1.type, fixup_8bit_literal(o1.value)));
        instructions.emplace_back(mos6502::OpCode::bit, o2);
      } else {
        throw std::runtime_error("Cannot translate testb instruction");
      }
      break;
    case i386::OpCode::decb:
      if (o1.type == Operand::Type::reg) {
        instructions.emplace_back(mos6502::OpCode::dec, get_register(o1.reg_num));
      } else {
        instructions.emplace_back(mos6502::OpCode::dec, o1);
      }
      break;
    case i386::OpCode::incb:
      if (o1.type == Operand::Type::reg) {
        instructions.emplace_back(mos6502::OpCode::inc, get_register(o1.reg_num));
      } else {
        instructions.emplace_back(mos6502::OpCode::inc, o1);
      }
      break;
    case i386::OpCode::incl:
      if (o1.type == Operand::Type::reg) {
	if (cpu_45gs02) {
	  instructions.emplace_back(mos6502::OpCode::neg);
	  instructions.emplace_back(mos6502::OpCode::neg);
	  instructions.emplace_back(mos6502::OpCode::inc, get_register(o1.reg_num));
	} else {
	  static int incl_count=0;
	  char new_label_name[32];
	  snprintf(new_label_name,31,"__incl_%d",incl_count++);
	  instructions.emplace_back(mos6502::OpCode::inc, get_register(o1.reg_num));
	  instructions.emplace_back(mos6502::OpCode::bne, Operand(Operand::Type::literal,new_label_name));
	  instructions.emplace_back(mos6502::OpCode::inc, get_register(o1.reg_num,1));
	  instructions.emplace_back(mos6502::OpCode::bne, Operand(Operand::Type::literal,new_label_name));
	  instructions.emplace_back(mos6502::OpCode::inc, get_register(o1.reg_num,2));
	  instructions.emplace_back(mos6502::OpCode::bne, Operand(Operand::Type::literal,new_label_name));
	  instructions.emplace_back(mos6502::OpCode::inc, get_register(o1.reg_num,3));
	  instructions.emplace_back(ASMLine::Type::Label, new_label_name);
	}
      } else {
	if (cpu_45gs02) {
	  instructions.emplace_back(mos6502::OpCode::neg);
	  instructions.emplace_back(mos6502::OpCode::neg);
	  instructions.emplace_back(mos6502::OpCode::inc, o1);
	} else
	  throw std::runtime_error("incl opcode requires 45GS02");
      }
      break;
    case i386::OpCode::jne:
      instructions.emplace_back(mos6502::OpCode::bne, o1);
      break;
    case i386::OpCode::je:
      instructions.emplace_back(mos6502::OpCode::beq, o1);
      break;
    case i386::OpCode::jg:
      instructions.emplace_back(mos6502::OpCode::bcs, o1);
      break;
    case i386::OpCode::js:
      instructions.emplace_back(mos6502::OpCode::bmi, o1);
      break;
    case i386::OpCode::jmp:
      instructions.emplace_back(mos6502::OpCode::jmp, o1);
      break;
    case i386::OpCode::call:
      instructions.emplace_back(mos6502::OpCode::jsr, o1);
      break;
    case i386::OpCode::addb:
      if (o1.type == Operand::Type::literal && o2.type == Operand::Type::reg) {
        instructions.emplace_back(mos6502::OpCode::lda, get_register(o2.reg_num));
        instructions.emplace_back(mos6502::OpCode::clc);
        instructions.emplace_back(mos6502::OpCode::adc, Operand(o1.type, fixup_8bit_literal(o1.value)));
        instructions.emplace_back(mos6502::OpCode::sta, get_register(o2.reg_num));
      } else if (o1.type == Operand::Type::literal && o2.type == Operand::Type::literal) {
        instructions.emplace_back(mos6502::OpCode::lda, Operand(o1.type, fixup_8bit_literal(o1.value)));
        instructions.emplace_back(mos6502::OpCode::clc);
        instructions.emplace_back(mos6502::OpCode::adc, o2);
        instructions.emplace_back(mos6502::OpCode::sta, o2);
      } else if (o1.type == Operand::Type::reg && o2.type == Operand::Type::literal) {
        instructions.emplace_back(mos6502::OpCode::lda, get_register(o1.reg_num));
        instructions.emplace_back(mos6502::OpCode::clc);
        instructions.emplace_back(mos6502::OpCode::adc, o2);
        instructions.emplace_back(mos6502::OpCode::sta, o2);
      } else {
        throw std::runtime_error("Cannot translate addb instruction");
      }
      break;
    case i386::OpCode::cmpb:
      if (o1.type == Operand::Type::literal && o2.type == Operand::Type::literal) {
        instructions.emplace_back(mos6502::OpCode::lda, o2);
        instructions.emplace_back(mos6502::OpCode::cmp, Operand(o1.type, fixup_8bit_literal(o1.value)));
      } else if (o1.type == Operand::Type::literal && o2.type == Operand::Type::reg) {
        instructions.emplace_back(mos6502::OpCode::lda, get_register(o2.reg_num));
        instructions.emplace_back(mos6502::OpCode::cmp, Operand(o1.type, fixup_8bit_literal(o1.value)));
      } else {
        throw std::runtime_error("Cannot translate cmpb instruction");
      }
      break;
    case i386::OpCode::cmpl:
      if (cpu_45gs02) {
	if (o1.type == Operand::Type::literal && o2.type == Operand::Type::literal) {
	  // Er... we are comparing #$nn with #$mm ???
	  instructions.emplace_back(mos6502::OpCode::neg);
	  instructions.emplace_back(mos6502::OpCode::neg);
	  instructions.emplace_back(mos6502::OpCode::lda, o2);
	  instructions.emplace_back(mos6502::OpCode::neg);
	  instructions.emplace_back(mos6502::OpCode::neg);
	  instructions.emplace_back(mos6502::OpCode::cmp, o1);
	} else if (o1.type == Operand::Type::literal && o2.type == Operand::Type::reg) {
	  // Compare register to literal
	  // Subtract second from first, but don't store result
	  // Prepare the constant
	  instructions.emplace_back(mos6502::OpCode::lda, Operand(o1.type, "#<[" + o1.value+" & $FFFF]"));
	  instructions.emplace_back(mos6502::OpCode::ldx, Operand(o1.type, "#>[" + o1.value+" & $FFFF]"));
	  instructions.emplace_back(mos6502::OpCode::ldy, Operand(o1.type, "#<[" + o1.value+" >> 16]"));
	  instructions.emplace_back(mos6502::OpCode::ldz, Operand(o1.type, "#>[" + o1.value+" >> 16]"));

	  instructions.emplace_back(mos6502::OpCode::neg);
	  instructions.emplace_back(mos6502::OpCode::neg);
	  instructions.emplace_back(mos6502::OpCode::sbc, get_register(o2.reg_num));
	} else {
	throw std::runtime_error("Unsupported operand combination for cmpl");	  
	}
      } else
	throw std::runtime_error("cmpl opcode requires 45GS02");
      break;
    case i386::OpCode::andb:
      if (o1.type == Operand::Type::literal && o2.type == Operand::Type::reg)  {
        const auto reg = get_register(o2.reg_num);
        instructions.emplace_back(mos6502::OpCode::lda, reg);
        instructions.emplace_back(mos6502::OpCode::AND, Operand(o1.type, fixup_8bit_literal(o1.value)));
        instructions.emplace_back(mos6502::OpCode::sta, reg);
      } else if (o1.type == Operand::Type::literal && o2.type == Operand::Type::literal)  {
        const auto reg = get_register(o2.reg_num);
        instructions.emplace_back(mos6502::OpCode::lda, o2);
        instructions.emplace_back(mos6502::OpCode::AND, Operand(o1.type, fixup_8bit_literal(o1.value)));
        instructions.emplace_back(mos6502::OpCode::sta, o2);
      } else {
        throw std::runtime_error("Cannot translate andb instruction");
      }
      break;
    case i386::OpCode::negb:
      if (o1.type == Operand::Type::reg) {
        // perform a two's complement of the register location
        instructions.emplace_back(mos6502::OpCode::lda, get_register(o1.reg_num));
        instructions.emplace_back(mos6502::OpCode::eor, Operand(Operand::Type::literal, "#$ff"));
        instructions.emplace_back(mos6502::OpCode::sta, get_register(o1.reg_num));
        instructions.emplace_back(mos6502::OpCode::inc, get_register(o1.reg_num));
      } else {
        throw std::runtime_error("Cannot translate negb instruction");
      }
      break;
    case i386::OpCode::notb:
      if (o1.type == Operand::Type::reg) {
        // exclusive or against 0xff to perform a logical not
        instructions.emplace_back(mos6502::OpCode::lda, get_register(o1.reg_num));
        instructions.emplace_back(mos6502::OpCode::eor, Operand(Operand::Type::literal, "#$ff"));
        instructions.emplace_back(mos6502::OpCode::sta, get_register(o1.reg_num));
      } else {
        throw std::runtime_error("Cannot translate notb instruction");
      }
      break;
    case i386::OpCode::subb:
      // DEST <- DEST - SRC
      // o2 = o2 - o1
      // Ensure that we set the carry flag before performing the subtraction
      if (o1.type == Operand::Type::reg && o2.type == Operand::Type::literal) {
        instructions.emplace_back(mos6502::OpCode::lda, o2);
        instructions.emplace_back(mos6502::OpCode::sec);
        instructions.emplace_back(mos6502::OpCode::sbc, get_register(o1.reg_num));
        instructions.emplace_back(mos6502::OpCode::sta, o2);
      } else {
        throw std::runtime_error("Cannot translate subb instruction");
      }
      break;
    case i386::OpCode::pushl:
      if (o1.type == Operand::Type::reg) {
        instructions.emplace_back(mos6502::OpCode::lda, get_register(o1.reg_num));
        instructions.emplace_back(mos6502::OpCode::pha);
        instructions.emplace_back(mos6502::OpCode::lda, get_register(o1.reg_num, 1));
        instructions.emplace_back(mos6502::OpCode::pha);
      } else {
        throw std::runtime_error("Cannot translate pushl instruction");
      }
      break;

    case i386::OpCode::sbbb:
      // DEST <- (DEST – (SRC + CF))
      // o2 <- (o2 - (o1 + cf))
      // if o1 and o2 are the same we get
      // o2 <- (o2 - (o2 + cf))
      // o2 <- -cf
      if (o1.type == Operand::Type::reg && o2.type == Operand::Type::reg
          && o1.reg_num == o2.reg_num) {
        instructions.emplace_back(mos6502::OpCode::lda, Operand(Operand::Type::literal, "#$00")); // reset a
        instructions.emplace_back(mos6502::OpCode::sbc, Operand(Operand::Type::literal, "#$00")); // subtract out the carry flag
        instructions.emplace_back(mos6502::OpCode::eor, Operand(Operand::Type::literal, "#$ff")); // invert the bits
        instructions.emplace_back(mos6502::OpCode::sta, get_register(o2.reg_num)); // place the value 
      } else {
        throw std::runtime_error("Cannot translate sbbb instruction");
      }
      break;

    default:
      throw std::runtime_error("Cannot translate unhandled instruction");

  };

}

enum class LogLevel
{
  Warning,
  Error
};

std::string to_string(const LogLevel ll)
{
  switch (ll)
  {
    case LogLevel::Warning:
      return "warning";
    case LogLevel::Error:
      return "error";
  }
  return "unknown";
}

void log(LogLevel ll, const i386 &i, const std::string &message)
{
  std::cerr << to_string(ll) << ": " << i.line_num << ": " << message << ": `" << i.line_text << "`\n";
}

void log(LogLevel ll, const int line_no, const std::string &line, const std::string &message)
{
  std::cerr << to_string(ll) << ": "  << line_no << ": " << message << ": `" << line << "`\n";
}

void to_mos6502(const i386 &i, std::vector<mos6502> &instructions)
{
  try {
    switch(i.type)
    {
      case ASMLine::Type::Label:
        instructions.emplace_back(i.type, i.text);
        return;
      case ASMLine::Type::Directive:
        instructions.emplace_back(i.type, i.text);
        return;
      case ASMLine::Type::Instruction:
//        instructions.emplace_back(ASMLine::Type::Directive, "; " + i.line_text);

        const auto head = instructions.size();
        translate_instruction(instructions, i.opcode, i.operand1, i.operand2);

        auto text = i.line_text;
        if (text[0] == '\t') {
          text.erase(0, 1);
        }
        for_each(std::next(instructions.begin(), head), instructions.end(), 
            [ text ](auto &ins){ 
              
              ins.comment = text; 
            }
          );
        return;
    }
  } catch (const std::exception &e) {
    log(LogLevel::Error, i, e.what());
  }
}

bool optimize(std::vector<mos6502> &instructions)
{
  if (instructions.size() < 2) {
    return false;
  }

  const auto next_instruction = [&instructions](auto i) {
    do {
      ++i;
    } while (i < instructions.size() && instructions[i].type == ASMLine::Type::Directive);
    return i;
  };

  for (size_t op = 0; op < instructions.size() - 1; ++op)
  {
    // look for a transfer of Y -> A immediately followed by A -> Y
    if (instructions[op].opcode == mos6502::OpCode::tya)
    {
      next_instruction(op);
      if (instructions[op].opcode == mos6502::OpCode::tay) {
        instructions.erase(std::next(std::begin(instructions), op), std::next(std::begin(instructions), op+1));
        return true;
      }
    }
  }

  for (size_t op = 0; op < instructions.size() - 1; ++op)
  {
    // look for a store A -> loc immediately followed by loc -> A
    if (instructions[op].opcode == mos6502::OpCode::sta)
    {
      const auto next = next_instruction(op);
      if (instructions[next].opcode == mos6502::OpCode::lda
          && instructions[next].op == instructions[op].op)
      {
        instructions.erase(std::next(std::begin(instructions), next), std::next(std::begin(instructions), next+1));
        return true;
      }
    }
  }

  for (size_t op = 0; op < instructions.size() - 1; ++op)
  {
    if (instructions[op].opcode == mos6502::OpCode::lda
        && instructions[op].op.type == Operand::Type::literal)
    {
      const auto operand = instructions[op].op;
      auto op2 = op+1;
      // look for multiple stores of the same value
      while (op2 < instructions.size() && (instructions[op2].opcode == mos6502::OpCode::sta 
          || instructions[op2].type == ASMLine::Type::Directive)) {
        ++op2;
      }
      if (instructions[op2].opcode == mos6502::OpCode::lda
          && operand == instructions[op2].op)
      {
        instructions.erase(std::next(std::begin(instructions), op2), std::next(std::begin(instructions), op2+1));
        return true;
      }

    }
  }


  return false;
}

bool fix_long_branches(std::vector<mos6502> &instructions, int &branch_patch_count)
{
  std::map<std::string, size_t> labels;
  for (size_t op = 0; op < instructions.size(); ++op)
  {
    if (instructions[op].type == ASMLine::Type::Label) {
      labels[instructions[op].text] = op;
    }
  }

  for (size_t op = 0; op < instructions.size(); ++op)
  {
    if (instructions[op].is_branch && std::abs(static_cast<int>(labels[instructions[op].op.value]) - static_cast<int>(op)) * 3 > 255)
    {
      ++branch_patch_count;
      const auto going_to = instructions[op].op.value;
      const auto new_pos = "patch_" + std::to_string(branch_patch_count);
      // uh-oh too long of a branch, have to convert this to a jump...
      if (instructions[op].opcode == mos6502::OpCode::bne) {
        const auto comment = instructions[op].comment;
        instructions[op] = mos6502(mos6502::OpCode::beq, Operand(Operand::Type::literal, new_pos));
        instructions.insert(std::next(std::begin(instructions), op + 1), mos6502(mos6502::OpCode::jmp, Operand(Operand::Type::literal, going_to)));
        instructions.insert(std::next(std::begin(instructions), op + 2), mos6502(ASMLine::Type::Label, new_pos));
        instructions[op].comment = instructions[op+1].comment = instructions[op+2].comment = comment;
        return true;
      } else {
        throw std::runtime_error("Don't know how to reorg this branch");
      }
    }
  }
  return false;
}


bool fix_overwritten_flags(std::vector<mos6502> &instructions)
{
  if (instructions.size() < 3) {
    return false;
  }

  for (size_t op = 0; op < instructions.size(); ++op)
  {
    if (instructions[op].is_comparison) {
      auto op2 = op + 1;
      while (op2 < instructions.size() 
             && !instructions[op2].is_comparison
             && !instructions[op2].is_branch)
      {
        ++op2;
      }

      if (op2 < instructions.size() 
          && (op2 - op) > 1
          && instructions[op2-1].opcode != mos6502::OpCode::plp) {
        if (instructions[op2].is_comparison) {
          continue;
        }

        if (instructions[op2].is_branch) {
          // insert a pull of processor status before the branch
          instructions.insert(std::next(std::begin(instructions), op2), mos6502(mos6502::OpCode::plp));
          // insert a push of processor status after the comparison
          instructions.insert(std::next(std::begin(instructions), op+1), mos6502(mos6502::OpCode::php));

          return true;
        }

      }
    }
  }

  return false;
}


int main(int argc,char **argv)
{
  std::regex Comment(R"(\s*\#.*)");
  std::regex Label(R"(^(\S+):.*)");
  std::regex Directive(R"(^\t(\..+))");
  std::regex UnaryInstruction(R"(^\t(\S+)\s+(\S+))");
  std::regex BinaryInstruction(R"(^\t(\S+)\s+(\S+),\s+(\S+))");
  std::regex Instruction(R"(^\t(\S+))");

  int lineno = 0;

  std::vector<i386> instructions;

  int c=0;
  while ((c=getopt(argc,argv,"m:"))!=-1) {
    switch(c) {
    case 'm':
      {
	char *cpu_type=optarg;
	if (!(strcasecmp(cpu_type,"45gs02")&&strcasecmp(cpu_type,"mega65")))
	  cpu_45gs02=1;
	else if (!(strcasecmp(cpu_type,"6502")&&strcasecmp(cpu_type,"6510")))
	  cpu_45gs02=0;
	else
	  {
	    std::cout << "Illegal CPU type specified.  Valid values are 6502 and 45gs02\n";
	    exit(-1);
	  }
      }
      break;
    default:
      std::cout << "Illegal option\n"
		<< "usage: x86-to-6502 [-m <target>]\n"
		<< "\n"
		<< "Valid targets are: 6502 and 45gs02\n"
		<< "\n";
      exit(-1);
      break;
    }
  }
  
  while (std::cin.good())
  {
    std::string line;
    getline(std::cin, line);

    try {
      std::smatch match;
      if (std::regex_match(line, match, Label))
      {
        instructions.emplace_back(lineno, line, ASMLine::Type::Label, match[1]);
      } else if (std::regex_match(line, match, Comment)) {
        // don't care about comments
      } else if (std::regex_match(line, match, Directive)) {
        instructions.emplace_back(lineno, line, ASMLine::Type::Directive, match[1]);
      } else if (std::regex_match(line, match, UnaryInstruction)) {
        instructions.emplace_back(lineno, line, ASMLine::Type::Instruction, match[1], match[2]);
      } else if (std::regex_match(line, match, BinaryInstruction)) {
        instructions.emplace_back(lineno, line, ASMLine::Type::Instruction, match[1], match[2], match[3]);
      } else if (std::regex_match(line, match, Instruction)) {
        instructions.emplace_back(lineno, line, ASMLine::Type::Instruction, match[1]);
      } else if (line == "") {
        //std::cout << "EmptyLine\n";
      } else {
        throw std::runtime_error("Unparsed Input, Line: " + std::to_string(lineno));
      }
    } catch (const std::exception &e) {
      log(LogLevel::Error, lineno, line, e.what());
    }

    ++lineno;
  }

  std::set<std::string> labels;

  for (const auto i : instructions)
  {
    if (i.type == ASMLine::Type::Label) {
      labels.insert(i.text);
    }
  }

  std::set<std::string> used_labels{"main"};

  for (const auto i : instructions)
  {
    if (i.type == ASMLine::Type::Instruction)
    {
      if (labels.count(i.operand1.value) != 0) {
        used_labels.insert(i.operand1.value);
        used_labels.insert(i.operand2.value);
      }
    }
  }

  // remove all labels and directives that we don't need
  instructions.erase(
    std::remove_if(std::begin(instructions), std::end(instructions),
        [&used_labels](const auto &i){
          if (i.type == ASMLine::Type::Label) {
            if (used_labels.count(i.text) == 0) {
              // remove all unused labels that aren't 'main'
              return true;
            }
          }
          return false;
        }
        ),
    std::end(instructions)
  );



  const auto new_labels = 
    [&used_labels](){
      std::map<std::string, std::string> result;
      for (const auto &l : used_labels) {
        auto newl = l;
        std::transform(newl.begin(), newl.end(), newl.begin(), [](const auto c) { return std::tolower(c); });
        newl.erase(std::remove_if(newl.begin(), newl.end(), [](const auto c){ return !std::isalnum(c); }), std::end(newl));
        result.emplace(std::make_pair(l, newl));
      }
      return result;
    }();



  for (auto &i : instructions)
  {
    if (i.type == ASMLine::Type::Label)
    {
      i.text = new_labels.at(i.text);
    }

    const auto itr1 = new_labels.find(i.operand1.value);
    if (itr1 != new_labels.end()) {
      i.operand1.value = itr1->second;
    }

    const auto itr2 = new_labels.find(i.operand2.value);
    if (itr2 != new_labels.end()) {
      i.operand2.value = itr2->second;
    }
  }


  std::vector<mos6502> new_instructions;

  for (const auto &i : instructions)
  {
    to_mos6502(i, new_instructions);
  }

  while (fix_overwritten_flags(new_instructions))
  {
    // do it however many times it takes
  }

  while (optimize(new_instructions))
  {
    // do it however many times it takes
  }

  int branch_patch_count = 0;
  while (fix_long_branches(new_instructions, branch_patch_count))
  {
    // do it however many times it takes
  }


  for (const auto i : new_instructions)
  {
    std::cout << i.to_string() << '\n';
  }
}
