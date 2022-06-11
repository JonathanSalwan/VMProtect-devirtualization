handle SIGILL pass nostop noprint
break loadYmm0Breakpoint
break loadZmm0Breakpoint
break loadK0Breakpoint
break HandleSigill
cont
set width 4096
p $k0
cont
p/x $ymm0
cont
p/x $zmm0
quit
