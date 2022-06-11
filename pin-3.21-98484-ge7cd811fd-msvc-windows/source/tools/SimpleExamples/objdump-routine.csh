#!/bin/csh -f
#


if ( $#argv != 2 ) then
    echo "ERROR: missing arguments"
    echo ""
    echo "Usage: objdump-routine.csh routine image"
    echo ""
    echo "disassemble 'routine' using objdump until the next symbol (as determined by readelf)" 
    exit -1
endif



set beg1 = `readelf -s $2 | sort -k 2 | grep -A 1 -w $1 | tail -2 | head -1`
set end1 = `readelf -s $2 | sort -k 2 | grep -A 1 -w $1 | tail -1`

if ($#beg1 == 0 || $#end1 == 0 ) then
    echo "could not find routine symbol in image"
    exit -1
endif



set beg = $beg1[2]
set end = $end1[2]

echo "# Disassemble $1 from $beg to $end"

objdump -d --start-address=0x$beg --stop-address=0x$end $2
 

#eof
