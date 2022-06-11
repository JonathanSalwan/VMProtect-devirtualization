/*
 * Copyright (C) 2021-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#ifdef TARGET_WINDOWS
#include <windows.h>
#else
#include <sys/mman.h>
#endif

// Get page size from OS
static int GetPageSize();

// Allocate memory by OS
static uint8_t* AllocateMemory();

// Notify on successful instrumentation
static void PrintSuccess();

// The tool is expected to instrument this method and receive the relevant information to be
// able to instrument the indirect call read operand.
static void ArgsHook(void* tested_instruction, void* displacemen_address);

static uint8_t call_print_success_code[] = {
    0x55,                                           // 0x0 push rbp
    0x48, 0x89, 0xe5,                               // 0x1 mov rbp, rsp
    0x67, 0xff, 0x14, 0x25, 0x00, 0x00, 0x00, 0x00, // 0x4 addr32 call [00000000]
    0xc9,                                           // 0xB leave
    0xc3,                                           // 0xC ret
};

// Byte offset of the instruction "addr32 call [00000000]"
#define INSTRUCTION_OFFSET 0X4

// An offset to an arbitrary address located after the "call_print_success_code" code block.
// This address will hold the address of the PrintSuccess method, and it is the tool's job
// to places this address in the operand of addr32 call [00000000].
#define DISPLACEMENT_OFFSET 0x100

union Memory
{
    struct
    {
        uint8_t pad[INSTRUCTION_OFFSET];
        uint8_t data;
    } inst;

    struct
    {
        uint8_t pad[DISPLACEMENT_OFFSET];
        void (*data)();
    } displacement;
};

#ifdef TARGET_LINUX
static int GetPageSize() // Linux
{
    return getpagesize();
}
#else
static int GetPageSize() // Windows
{
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    return si.dwPageSize;
}
#endif

#ifdef TARGET_LINUX
static uint8_t* AllocateMemory() // Linux
{
    // allocate a page in the low 32 bits address space
    void *memory = mmap(0, GetPageSize(), PROT_READ | PROT_WRITE | PROT_EXEC,
                               MAP_PRIVATE | MAP_32BIT | MAP_ANONYMOUS, -1, 0);

    if (memory == MAP_FAILED) {
        fprintf(stderr, "mmap failed");
        exit(1);
    }

    if (((uintptr_t)memory) >= ((1LLU) << 32))
    {
        fprintf(stderr, "Address not in lower 2^32 range : 0x%p", memory);
        exit(1);
    }

    return memory;
}
#else
static uint8_t* AllocateMemory() // Windows
{
    int page_size   = GetPageSize();
    uint8_t* memory = NULL;
    uintptr_t ptr   = 1 << 16; // Start from 64K.
    for (; ptr < (1LLU << 32); ptr += page_size)
    {
        memory = VirtualAlloc(0, page_size, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
        if (memory) break;
    }
    if (!memory)
    {
        fprintf(stderr, "Couldn't allocate memory in lower 2^32 addresses");
        exit(1);
    }
    return memory;
}
#endif

static void PrintSuccess() {
    printf("success\n");
}

// The tool is expected to instrument this method and recieve the relevant information.
static void ArgsHook(void* tested_instruction, void* displacemen_address){}

int main() {

    // allocate a page in the low 32 bits address space
    union Memory *memory = (union Memory *) AllocateMemory();

    // copy the address of foo to memory+0x100
    memory->displacement.data = PrintSuccess;

    // copy the code stub to the allocated memory
    memcpy(memory, call_print_success_code, sizeof(call_print_success_code));

    // Notify pin on relevant arguments values
    ArgsHook(&memory->inst.data, &memory->displacement.data);

    // call the code stab
    ((void (*)())memory)();

    return 0;
}
