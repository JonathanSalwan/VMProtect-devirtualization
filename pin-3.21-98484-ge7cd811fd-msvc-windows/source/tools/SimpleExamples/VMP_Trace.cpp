#include "pin.H"
#include <fstream>
#include <iostream>
#include <list>

std::ostream* out = &std::cerr;
bool start = true;

static KNOB<UINT32> KnobStart(KNOB_MODE_WRITEONCE, "pintool", "start", "0", "Start the tracing at this address");
static KNOB<UINT32> KnobEnd(KNOB_MODE_WRITEONCE, "pintool", "end", "0", "Stop the tracing at this address");

VOID cb_inst(CONTEXT* ctx, const unsigned char* addr, UINT32 size) {
  std::ios_base::fmtflags f(out->flags());
  UINT64 buffer = 0;

  std::list<LEVEL_BASE::REG> regs;
  regs.push_back(LEVEL_BASE::REG_RAX);
  regs.push_back(LEVEL_BASE::REG_RBX);
  regs.push_back(LEVEL_BASE::REG_RCX);
  regs.push_back(LEVEL_BASE::REG_RDX);
  regs.push_back(LEVEL_BASE::REG_RDI);
  regs.push_back(LEVEL_BASE::REG_RSI);
  regs.push_back(LEVEL_BASE::REG_RBP);
  regs.push_back(LEVEL_BASE::REG_RSP);
  regs.push_back(LEVEL_BASE::REG_R8);
  regs.push_back(LEVEL_BASE::REG_R9);
  regs.push_back(LEVEL_BASE::REG_R10);
  regs.push_back(LEVEL_BASE::REG_R11);
  regs.push_back(LEVEL_BASE::REG_R12);
  regs.push_back(LEVEL_BASE::REG_R13);
  regs.push_back(LEVEL_BASE::REG_R14);
  regs.push_back(LEVEL_BASE::REG_R15);

  // Registers
  *out << "r";
  for (const auto& reg : regs) {
    PIN_GetContextRegval(ctx, reg, reinterpret_cast<unsigned char*>(&buffer));
    *out << ":" << std::hex << "0x" << buffer << std::dec;
  }
  *out << std::endl;

  // Instruction
  *out << "i:" << std::hex << "0x" << reinterpret_cast<unsigned long>(addr) << std::dec << ":" << size << ":";
  for (size_t i = 0; i < size; ++i)
    *out << std::uppercase << std::hex << std::setfill('0') << std::setw(2) << (((int)addr[i]) & 0xFF);
  *out << std::endl;

  out->flags(f);
}

VOID cb_memread(UINT64 addr, UINT32 size) {
  std::ios_base::fmtflags f(out->flags());
  *out << "mr:" << std::hex << "0x" << addr << std::dec << ":" << size << ":";
  switch (size) {
    case 1: *out << std::hex << "0x" << static_cast<const int>(*reinterpret_cast<const unsigned char*>(addr)) << std::dec; break;
    case 2: *out << std::hex << "0x" << *reinterpret_cast<const short*>(addr) << std::dec; break;
    case 4: *out << std::hex << "0x" << *reinterpret_cast<const UINT32*>(addr) << std::dec; break;
    case 8: *out << std::hex << "0x" << *reinterpret_cast<const UINT64*>(addr) << std::dec; break;
  }
  *out << std::endl;
  out->flags(f);
}

VOID Trace(TRACE trace, VOID* v) {
  for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl)) {
    for (INS ins = BBL_InsHead(bbl); INS_Valid(ins); ins = INS_Next(ins)) {
      if (start && INS_IsMemoryRead(ins)) {
        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)cb_memread,
          IARG_MEMORYREAD_EA,
          IARG_MEMORYREAD_SIZE,
          IARG_END);
      }

      if (start && INS_HasMemoryRead2(ins)) {
        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)cb_memread,
          IARG_MEMORYREAD2_EA,
          IARG_MEMORYREAD_SIZE,
          IARG_END);
      }

      if (start) {
        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)cb_inst, IARG_CONTEXT, IARG_INST_PTR, IARG_UINT32, INS_Size(ins), IARG_END);
      }
    }
  }
}


int usage(void) {
  std::cerr << "Usage: ./pin -t VMP_Trace.dll -start <start addr> -end <end addr> -- <vmp_binary> <vmp_binary_arg>" << std::endl;
  return -1;
}


int main(int argc, char* argv[]) {
  if (PIN_Init(argc, argv)) {
    return usage();
  }

  if (!KnobStart || !KnobEnd) {
    return usage();
  }

  std::cerr << "KnobStart: " << KnobStart << std::endl;
  std::cerr << "KnobEnd: " << KnobEnd <<std::endl;

  start = true;

  TRACE_AddInstrumentFunction(Trace, 0);
  PIN_StartProgram();

  return 0;
}