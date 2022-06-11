/*
 * Copyright (C) 2014-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>

#if defined(TARGET_WINDOWS)
#define EXPORT_CSYM __declspec(dllexport)
#define NO_INLINE __declspec(noinline)
#else
#define EXPORT_CSYM
#define NO_INLINE
extern int DoReps();
#endif

EXPORT_CSYM NO_INLINE void RepAppAtIpointAfterStarted() { printf("RepAppAtIpointAfterStarted\n"); }

int main(int argc, char** argv)
{
#if defined(TARGET_WINDOWS)
    char stringOne[]    = "IAMHEREE";
    char stringTwo[]    = "IWASHERE";
    char stringThree[]  = "ABCDEF20";
    char stringFour[]   = "BCDEFG21";
    char stringBlanks[] = "          X";
    char stringSource[] = "12345678";
    char stringDest[9];
#define length 9
#define length2 8
#define length3 12
#define length4 8
    int retVal = 0;
    RepAppAtIpointAfterStarted();
    printf("Starting...\n");
    __asm {
        fnop                                      
	cld
        xor     ebx, ebx                      ; ebx holds test number (used as exit code on failure)
        
; Test different string comparison
        inc     ebx
	lea	esi, stringOne
	lea	edi, stringTwo
	mov     ecx, length
	repe cmpsb
        cmp     ecx,(length-2)                ; Should fail at second byte
        jne     l2

; Test same string comparison
        inc     ebx
	lea	esi, stringOne
	lea	edi, stringOne
	mov     ecx, length
	repe cmpsb 
        test    ecx,ecx                       ; Should run full length
        jne     l2

; Test same string comparison, but with no count...
        inc     ebx
	lea	esi, stringOne
	lea	edi, stringOne
        xor     ecx,ecx
	repe cmpsb 
        test    ecx,ecx                       ; Should still be zero
        jne     l2

; Test same string comparison limited by ecx
        inc     ebx
	lea	esi, stringOne
	lea	edi, stringOne
	mov     ecx, length-3
	repe cmpsb
        cmp     ecx,(0)                ; Should run til ecx becomes 0
        jne     l2

; Test different comparison repne
        inc     ebx
	lea	esi, stringThree
	lea	edi, stringFour
	mov     ecx, length2
	repne cmpsb
        cmp     ecx,(1)                ; Should fail at last byte
        jne     l2

; Test scasb
        inc     ebx
	mov	eax, 32
	lea	edi, stringBlanks
    mov     ecx, length3
	repe    scasb
	mov     eax,ecx
	cmp     ecx,(1)                   ; Should fail at last byte
    jne     l2


; Test rep movsb
        inc     ebx
	lea	esi, stringSource
	lea	edi, stringDest
	mov     ecx, length4
	rep movsb
        cmp     ecx,0                ; 
        jne     l2

; end

	jmp end
l2:
    lea	esi, retVal
    mov dword ptr[esi], 1
end:
        fnop
    }
#else
    int retVal;
    RepAppAtIpointAfterStarted();
    retVal = DoReps();
#endif
    printf("Ending... retVal %d\n", retVal);
    return retVal;
}
