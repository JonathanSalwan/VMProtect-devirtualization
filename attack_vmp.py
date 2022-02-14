#!/usr/bin/env python
## -*- coding: utf-8 -*-
##
## Working with Triton from commit 05b05cfbe8697a4a93d6ba674062f97465270412
##

import argparse
import sys

from triton import *


V_JMP = list()


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


def detecting_vjmp(execid, ctx, inst, vbraddr, vbrflag):
    global V_JMP
    ast = ctx.getAstContext()

    if execid == 2 and vbraddr and vbrflag:
        if inst.isSymbolized() and inst.getAddress() == vbraddr:
            flag = ctx.getRegisterAst(ctx.getRegister(vbrflag))
            if len(ast.search(flag, AST_NODE.VARIABLE)) == 2:
                model, status, _ = ctx.getModel(flag == flag.evaluate(), status=True)
                V_JMP.append(flag == flag.evaluate())
                return

    elif execid == 1:
        # Virtual jmp marker 1
        if inst.isSymbolized() and inst.getType() == OPCODE.X86.POPFQ:
            cf = ctx.getRegisterAst(ctx.registers.cf)
            if len(ast.search(cf, AST_NODE.VARIABLE)) == 2:
                model, status, _ = ctx.getModel(cf != cf.evaluate(), status=True)
                if status == SOLVER_STATE.SAT:
                    print(f'[+] A potential symbolic jump found on CF flag: {inst} - Model: {model}')

        # Virtual jmp marker 2
        if inst.isSymbolized() and inst.getType() == OPCODE.X86.CMP:
            op1 = inst.getOperands()[0]
            op2 = inst.getOperands()[1]
            if op1.getType() == OPERAND.REG and op2.getType() == OPERAND.REG:
                af  = ctx.getRegisterAst(ctx.registers.af)
                if len(ast.search(af, AST_NODE.VARIABLE)) == 2:
                    model, status, _ = ctx.getModel(af != af.evaluate(), status=True)
                    if status == SOLVER_STATE.SAT:
                        print(f'[+] A potential symbolic jump found of AF flag: {inst} - Model: {model}')

    return


def update_sym_var(ctx):
    ast = ctx.getAstContext()

    # Get concrete value of registers
    x_val = ctx.getConcreteRegisterValue(ctx.registers.rdi)
    y_val = ctx.getConcreteRegisterValue(ctx.registers.rsi)

    # Get symbolic variable
    sym_x = ctx.getSymbolicVariable(0)
    sym_y = ctx.getSymbolicVariable(1)

    # Set concrete value to symbolic variables
    ctx.setConcreteVariableValue(sym_x, x_val)
    ctx.setConcreteVariableValue(sym_y, y_val)

    # Create AST variables
    x = ast.zx(64 - sym_x.getBitSize(), ast.variable(sym_x))
    y = ast.zx(64 - sym_y.getBitSize(), ast.variable(sym_y))

    # Assign ASTs to symbolic reigsters
    ctx.assignSymbolicExpressionToRegister(ctx.newSymbolicExpression(x), ctx.registers.rdi)
    ctx.assignSymbolicExpressionToRegister(ctx.newSymbolicExpression(y), ctx.registers.rsi)
    return


def exec_instruction(execid, ctx, symsize, args, fuse, vbraddr, vbrflag):
    _, addr, size, data = args

    # This fuse is burned after the first instruction
    if fuse:
        print('[+] Symbolize inputs')
        map_size = {
            1: (ctx.registers.dil, ctx.registers.sil),
            2: (ctx.registers.di, ctx.registers.si),
            4: (ctx.registers.edi, ctx.registers.esi),
            8: (ctx.registers.rdi, ctx.registers.rsi),
        }
        vars = ctx.getSymbolicVariables()
        # If symbolic variables do not exist, create them
        if len(vars) == 0:
            ctx.symbolizeRegister(map_size[symsize][0], 'x')
            ctx.symbolizeRegister(map_size[symsize][1], 'y')
        else:
            # If symbolic variables already exist, assign them to registers
            update_sym_var(ctx)

    inst = Instruction(int(addr, 16), bytes.fromhex(data))
    ctx.processing(inst)
    detecting_vjmp(execid, ctx, inst, vbraddr, vbrflag)

    return False


def emulate(execid, ctx, symsize, file, vbraddr, vbrflag):
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
            fuse = exec_instruction(execid, ctx, symsize, args, fuse, vbraddr, vbrflag)
            count += 1

    print(f'[+] Instruction executed: {count}')
    return


def setMode(ctx):
    # Define optimizations
    ctx.setMode(MODE.ALIGNED_MEMORY, True)
    ctx.setMode(MODE.AST_OPTIMIZATIONS, True)
    ctx.setMode(MODE.CONSTANT_FOLDING, True)
    return


def one_path(execid, ctx, trace, symsize, vbraddr, vbrflag):
    print('[+] Replaying the VMP trace')
    emulate(execid, ctx, symsize, trace, vbraddr, vbrflag)
    print('[+] Emulation done')
    eax = ctx.getRegisterAst(ctx.registers.eax)
    return eax


def result(ctx, ret_expr):
    ast    = ctx.getAstContext()
    unro   = ast.unroll(ret_expr)
    synth  = ctx.synthesize(ret_expr)
    ppast1 = (str(unro) if len(str(unro)) < 100 else 'In: %s ...' %(str(unro)[0:100]))
    ppast2 = (str(synth) if len(str(synth)) < 100 else 'In: %s ...' %(str(unro)[0:100]))

    print(f'[+] Return value: {hex(ret_expr.evaluate())}')
    print(f'[+] Devirt expr: {ppast1}')
    print(f'[+] Synth expr: {ppast2}\n')
    print(f'[+] LLVM IR ==============================\n')
    print(ctx.liftToLLVM(synth if synth else ret_expr))
    print(f'[+] EOF LLVM IR ============================== ')
    return 0


def analysis(argv):
    ctx = TritonContext(ARCH.X86_64)
    setMode(ctx)

    ret_expr1 = one_path(1, ctx, argv.trace1, argv.symsize, argv.vbraddr, argv.vbrflag)
    if argv.trace2:
        print(f'[+] A second trace has been provided')
        ret_expr2 = one_path(2, ctx, argv.trace2, argv.symsize, argv.vbraddr, argv.vbrflag)
        ast = ctx.getAstContext()
        print(f'[+] Merging expressions from trace1 and trace2')
        e1 = V_JMP[0]
        ret_expr2 = ast.ite(e1, ret_expr2, ret_expr1)
        result(ctx, ret_expr2)
    else:
        result(ctx, ret_expr1)
    return 0


def main():
    parser = argparse.ArgumentParser(formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--trace1",  type=str,                  metavar="<trace1>",  help="Specify the VMP trace1")
    parser.add_argument("--trace2",  type=str,                  metavar="<trace2>",  help="Specify the VMP trace2. The second trace is used if you want merging paths")
    parser.add_argument("--symsize", type=int,                  metavar="<symsize>", help="Specify the size of symbolic variables")
    parser.add_argument("--vbraddr", type=lambda x: int(x,0),   metavar="<vbraddr>", help="Virtual branch address")
    parser.add_argument("--vbrflag", type=str,                  metavar="<vbrflag>", help="Virtual branch flag")
    argv = parser.parse_args(sys.argv[1:])

    if argv.trace1 is None:
        print('[-] You must define a VMP trace')
        print('[!] Syntax: %s --trace1 <vmp trace> --symsize <sym size>' %(sys.argv[0]))
        return -1

    if argv.symsize not in [1, 2, 4, 8]:
        print('[-] Size of symbolic variables must be equal to: 1, 2, 4, or 8 bytes')
        print('[!] Syntax: %s --trace1 <vmp trace> --symsize <sym size>' %(sys.argv[0]))
        return -1

    if argv.trace2 is not None and argv.vbrflag is None:
        print('[-] If you define a second trace, you have to define the virtual branch flag (e.g: cf, af, zf etc.')
        print('[!] Syntax: %s --trace1 <vmp trace> --trace2 <vmp trace> --symsize <sym size> --vbraddr <vbraddr> --vbrflag <vbrflag>' %(sys.argv[0]))
        return -1

    return analysis(argv)


if __name__ == '__main__':
    sys.exit(main())
