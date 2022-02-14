#!/usr/bin/env python
## -*- coding: utf-8 -*-
##
## Working with Triton from commit 05b05cfbe8697a4a93d6ba674062f97465270412
##

import argparse
import sys

from triton import *



def sync_reg(ctx, regs):
    mapping = {
        ctx.registers.rax : regs[0],
        ctx.registers.rbx : regs[1],
        ctx.registers.rcx : regs[2],
        ctx.registers.rdx : regs[3],
        ctx.registers.rdi : regs[4],
        ctx.registers.rsi : regs[5],
        ctx.registers.rbp : regs[6],
        ctx.registers.rsp : regs[7],
        ctx.registers.r8  : regs[8],
        ctx.registers.r9  : regs[9],
        ctx.registers.r10 : regs[10],
        ctx.registers.r11 : regs[11],
        ctx.registers.r12 : regs[12],
        ctx.registers.r13 : regs[13],
        ctx.registers.r14 : regs[14],
        ctx.registers.r15 : regs[15],
    }

    for tt_reg, pin_value in mapping.items():
        tt_value = ctx.getConcreteRegisterValue(tt_reg)
        if tt_value != int(pin_value, 16):
            ctx.setConcreteRegisterValue(tt_reg, int(pin_value, 16))
    return


def sync_memory(ctx, args):
    _, addr, size, data = args
    memory = MemoryAccess(int(addr, 16), int(size))
    synch  = ctx.getConcreteMemoryValue(memory)
    if synch != int(data, 16):
        ctx.setConcreteMemoryValue(memory, int(data, 16))
    return


def detecting_vjmp(ctx, inst):
    ast = ctx.getAstContext()

    if inst.isSymbolized() and inst.getType() == OPCODE.X86.POPFQ:
        cf = ctx.getRegisterAst(ctx.registers.cf)
        if len(ast.search(cf, AST_NODE.VARIABLE)) == 2:
            model, status, _ = ctx.getModel(cf != cf.evaluate(), status=True)
            if status == SOLVER_STATE.SAT:
                print(f'[+] A potential symbolic jump found on CF flag: {inst} - Model: {model}')

    if inst.isSymbolized() and inst.getType() == OPCODE.X86.CMP:
        af = ctx.getRegisterAst(ctx.registers.af)
        if len(ast.search(af, AST_NODE.VARIABLE)) == 2:
            model, status, _ = ctx.getModel(af != af.evaluate(), status=True)
            if status == SOLVER_STATE.SAT:
                print(f'[+] A potential symbolic jump found of AF flag: {inst} - Model: {model}')

    return


def exec_instruction(ctx, symsize, args, fuse):
    _, addr, size, data = args

    if fuse:
        print('[+] Symbolize inputs')
        map_size = {
            1: (ctx.registers.dil, ctx.registers.sil),
            2: (ctx.registers.di, ctx.registers.si),
            4: (ctx.registers.edi, ctx.registers.esi),
            8: (ctx.registers.rdi, ctx.registers.rsi),
        }
        ctx.symbolizeRegister(map_size[symsize][0], 'x')
        ctx.symbolizeRegister(map_size[symsize][1], 'y')

    inst = Instruction(int(addr, 16), bytes.fromhex(data))
    ctx.processing(inst)
    detecting_vjmp(ctx, inst)
    #if inst.isSymbolized():
    #    print(f'[T] {inst}')
    #else:
    #    print(f'[ ] {inst}')

    return False


def emulate(ctx, symsize, file):
    count = 0
    fuse = True

    with open(file, 'r') as fd:
      for line in fd:
        args = line.split(':')
        kind = args[0]

        # Synch memory read
        if kind == 'mr':
            sync_memory(ctx, args)

        # Synch registers
        if kind == 'r':
            sync_reg(ctx, args[1:])

        # Execute instruction
        if kind == 'i':
            fuse = exec_instruction(ctx, symsize, args, fuse)
            count += 1

    print(f'[+] Instruction executed: {count}')
    return


def setMode(ctx):
    # Define optimizations
    ctx.setMode(MODE.ALIGNED_MEMORY, True)
    ctx.setMode(MODE.AST_OPTIMIZATIONS, True)
    ctx.setMode(MODE.CONSTANT_FOLDING, True)
    return


def one_path(ctx, trace, symsize):
    ctx.concretizeAllRegister()
    ctx.concretizeAllMemory()
    print('[+] Replaying the VMP trace')
    emulate(ctx, symsize, trace)
    print('[+] Emulation done')
    eax = ctx.getRegisterAst(ctx.registers.eax)
    return eax


def analysis(argv):
    ctx = TritonContext(ARCH.X86_64)
    setMode(ctx)
    ret_expr = one_path(ctx, argv.trace, argv.symsize)
    return 0


def main():
    parser = argparse.ArgumentParser(formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--trace",   type=str, metavar="<trace>",  help="Specify the VMP trace")
    parser.add_argument("--symsize", type=int, metavar="<symsize>", help="Specify the size of symbolic variables")
    argv = parser.parse_args(sys.argv[1:])

    if argv.trace is None:
        print('[-] You must define a VMP trace')
        print('[!] Syntax: %s --trace <vmp trace> --symsize <sym size>' %(sys.argv[0]))
        return -1

    if argv.symsize not in [1, 2, 4, 8]:
        print('[-] Size of symbolic variables must be equal to: 1, 2, 4, or 8 bytes')
        print('[!] Syntax: %s --trace <vmp trace> --symsize <sym size>' %(sys.argv[0]))
        return -1

    return analysis(argv)


if __name__ == '__main__':
    sys.exit(main())
