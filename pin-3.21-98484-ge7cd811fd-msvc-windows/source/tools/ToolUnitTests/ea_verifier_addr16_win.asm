;
; Copyright (C) 2008-2012 Intel Corporation.
; SPDX-License-Identifier: MIT
;

PUBLIC Test_addr16


.686
.model flat, c
COMMENT // use of segment register is not an ERROR
ASSUME NOTHING
.code
Test_addr16 PROC
      
         push       ebx
         push       esi
         push       ebp
         mov        ebx, 0ffff0018h
         mov        ebp, 0ffff0017h
         mov        esi, 0ffff0002h
         db         67h, 64h, 0a1h, 05h, 00h
         db         67h, 64h, 8bh, 16h, 05h, 00h 
         db         67h, 64h, 89h, 16h, 05h, 00h
         db         67h, 64h, 8bh, 57h, 00h 
         db         67h, 64h, 89h, 57h, 00h
         db         67h, 64h, 8bh, 57h, 05h 
         db         67h, 64h, 89h, 57h, 05h
         db         67h, 64h, 8bh, 50h, 00h 
         db         67h, 64h, 89h, 50h, 00h
         db         67h, 64h, 8bh, 50h, 05h 
         db         67h, 64h, 89h, 50h, 05h
         db         67h, 8dh, 50h, 05h
         db         67h, 64h, 8bh, 52h, 05h
         db         67h, 64h, 89h, 52h, 05h          
         db         67h, 89h, 50h, 05h             
         db         67h, 8bh, 50h, 05h
         db         67h, 89h, 52h, 05h
         db         67h, 8bh, 52h, 05h
         db         67h, 89h, 57h, 05h
         db         67h, 8bh, 57h, 05h
         db         67h, 89h, 56h, 05h
         db         67h, 8bh, 56h, 05h 
         pop        ebp
         pop        esi
         pop        ebx
  
    ret

Test_addr16 ENDP

end

COMMENT  Do NOT currently know how to create address16 override in ml
COMMENT  To create the db stuff - which is the encoding of the following addr16 instructions
COMMENT  copy the ea_verifier_addr16_lin_for_win.s to a linux32 system and use as to assemble
COMMENT  and objdump to see the encoding of the instructions 

COMMENT:   67 64 a1 05 00          addr16 mov %fs:5,%eax
COMMENT:   67 64 8b 16 05 00       addr16 mov %fs:5,%edx
COMMENT:   67 64 89 16 05 00       addr16 mov %edx,%fs:5
COMMENT:   67 64 8b 57 00          addr16 mov %fs:0(%bx),%edx
COMMENT:   67 64 89 57 00          addr16 mov %edx,%fs:0(%bx)
COMMENT:   67 64 8b 57 05          addr16 mov %fs:5(%bx),%edx
COMMENT:   67 64 89 57 05          addr16 mov %edx,%fs:5(%bx)
COMMENT:   67 64 8b 50 00          addr16 mov %fs:0(%bx,%si),%edx
COMMENT:   67 64 89 50 00          addr16 mov %edx,%fs:0(%bx,%si)
COMMENT:   67 64 8b 50 05          addr16 mov %fs:5(%bx,%si),%edx
COMMENT:   67 64 89 50 05          addr16 mov %edx,%fs:5(%bx,%si)
COMMENT:   67 8d 50 05             addr16 lea 5(%bx,%si),%edx
COMMENT:   67 64 8b 52 05          addr16 mov %fs:5(%bp,%si),%edx
COMMENT:   67 64 89 52 05          addr16 mov %edx,%fs:5(%bp,%si)
COMMENT:   67 89 50 05             addr16 mov %edx,5(%bx,%si)
COMMENT:   67 8b 50 05             addr16 mov 5(%bx,%si),%edx
COMMENT:   67 89 52 05             addr16 mov %edx,5(%bp,%si)
COMMENT:   67 8b 52 05             addr16 mov 5(%bp,%si),%edx
COMMENT:   67 89 57 05             addr16 mov %edx,5(%bx)
COMMENT:   67 8b 57 05             addr16 mov 5(%bx),%edx
COMMENT:   67 89 56 05             addr16 mov %edx,5(%bp)
COMMENT:   67 8b 56 05             addr16 mov 5(%bp),%edx


