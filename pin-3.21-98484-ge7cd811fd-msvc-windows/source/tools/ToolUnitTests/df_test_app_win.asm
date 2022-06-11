;
; Copyright (C) 2007-2012 Intel Corporation.
; SPDX-License-Identifier: MIT
;

PUBLIC main_asm


.686
.model flat, c

.code
main_asm PROC
    std
COMMENT    // in the next two insts, the tool analysis functions (in df_test_tool1.cpp, df_test_tool2.cpp and df_test_tool3.cpp)
COMMENT     // , which is called for every inst in this app, may find the DF set if there is a pin bug 
COMMENT    // that does not clear the DF before invoking the non-inlined analysis function, 
COMMENT     // or there is a register allocation bug in the processing of the inlined analysis
    mov eax, 0
    cld
    ret

main_asm ENDP

end