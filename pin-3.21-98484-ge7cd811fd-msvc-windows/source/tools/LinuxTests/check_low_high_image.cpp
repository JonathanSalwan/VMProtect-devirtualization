/*
 * Copyright (C) 2019-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
  Test pin calculation for the IMG size as obtained from IMG_LowAddress and 
  IMG_HighAddress APIs and compare the size to the value computed from the ELF file.
  This test verify the fix for Mantis #4735.
 */

#include <stdio.h>
#include <sys/fcntl.h>
#include <elf.h>
#include "pin.H"

using namespace std;

/* ===================================================================== */

/* ===================================================================== */

int ValidEhdr(Elf64_Ehdr* ehdr)
{
    if (sizeof(Elf64_Ehdr) != ehdr->e_ehsize) return 0;

    if ((ehdr->e_ident[EI_MAG0] != ELFMAG0) || (ehdr->e_ident[EI_MAG1] != ELFMAG1) || (ehdr->e_ident[EI_MAG2] != ELFMAG2) ||
        (ehdr->e_ident[EI_MAG3] != ELFMAG3))
        return 0;

    // supports only 64 bits
    return (ehdr->e_machine == EM_X86_64);
}

INT64 CalcSize(const char* imgname, BOOL text_only)
{
    ADDRINT baseAddr = 0;
    BOOL set         = false;
    INT64 memSize    = 0;
    Elf64_Ehdr ehdr;
    Elf64_Phdr* phdr;
    int res, i, fd;

    fd = open(imgname, O_RDONLY, 0);
    if (fd < 0) return 0;

    res = read(fd, &ehdr, sizeof(ehdr));
    if (res != sizeof(ehdr) || !ValidEhdr(&ehdr))
    {
        // The file is not a legal ELF image
        close(fd);
        return 0;
    }

    // Read program headers
    phdr = (Elf64_Phdr*)malloc(sizeof(Elf64_Phdr) * ehdr.e_phnum);
    lseek(fd, ehdr.e_phoff, SEEK_SET);
    read(fd, phdr, ehdr.e_phnum * sizeof(phdr[0]));

    // find the lowest vaddr of loadable segmenta
    for (i = 0; i < ehdr.e_phnum; i++)
    {
        if (phdr[i].p_type == PT_LOAD)
        {
            if (text_only && (phdr[i].p_flags & PF_W) != 0) continue;
            if (!set || phdr[i].p_vaddr < baseAddr)
            {
                set      = true;
                baseAddr = phdr[i].p_vaddr;
            }
        }
    }

    // find the size based on distance from base address and mem size
    for (i = 0; i < ehdr.e_phnum; i++)
    {
        if (phdr[i].p_type == PT_LOAD)
        {
            ADDRINT endAddr = phdr[i].p_vaddr + phdr[i].p_memsz;
            if (text_only && (phdr[i].p_flags & PF_W) != 0) continue;

            if (memSize < (INT64)(endAddr - baseAddr))
            {
                memSize = endAddr - baseAddr;
            }
        }
    }

    free(phdr);
    close(fd);
    return memSize - 1;
}

VOID ImageLoad(IMG img, VOID* v)
{
    ADDRINT imgLow  = IMG_LowAddress(img);
    ADDRINT imgHigh = IMG_HighAddress(img);
    string imgName  = IMG_Name(img);
    INT64 calc_size = 0;

    if (imgName.find("vdso") != string::npos) return;

    if (IMG_NumRegions(img) == 1)
    {
        calc_size = CalcSize(imgName.c_str(), false);
        //        fprintf(stderr, "Img: %-30s: 1 Region: %p %p expected size: %lld got %lld\n",
        //                imgName.c_str(), (void*)imgLow, (void*)imgHigh, (long long)calc_size, (long long)(imgHigh-imgLow));
    }
    else
    {
        calc_size = CalcSize(imgName.c_str(), true);
        //        fprintf(stderr, "Img: %-30s: %d Region: %p %p expected size: %lld got %lld\n", imgName.c_str(),
        //                IMG_NumRegions(img), (void*)imgLow, (void*)imgHigh, (long long)calc_size, (long long)(imgHigh-imgLow));
    }

    if (calc_size != (INT64)(imgHigh - imgLow))
    {
        fprintf(stderr, "ERROR: Img: %-30s: expected size: %lld got %lld\n", imgName.c_str(), (long long)calc_size,
                (long long)(imgHigh - imgLow));
    }
}

/* ===================================================================== */

int main(int argc, CHAR* argv[])
{
    PIN_InitSymbols();
    PIN_Init(argc, argv);

    IMG_AddInstrumentFunction(ImageLoad, NULL);

    PIN_StartProgramProbed();

    return 1;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */
