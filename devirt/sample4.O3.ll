; ModuleID = './sample4.ll'
source_filename = "tritonModule"

; Function Attrs: mustprogress nofree norecurse nosync nounwind readnone willreturn
define i32 @__triton(i32 %SymVar_0, i32 %SymVar_1) local_unnamed_addr #0 {
entry:
  %0 = xor i32 %SymVar_0, %SymVar_1
  %1 = xor i32 %0, -1
  %2 = xor i32 %SymVar_0, -1
  %3 = sub i32 %SymVar_0, %SymVar_1
  %4 = and i32 %3, %1
  %5 = and i32 %2, %SymVar_1
  %6 = xor i32 %4, %5
  %7 = icmp slt i32 %6, 0
  %8 = select i1 %7, i32 %SymVar_1, i32 %SymVar_0
  ret i32 %8
}

attributes #0 = { mustprogress nofree norecurse nosync nounwind readnone willreturn }
