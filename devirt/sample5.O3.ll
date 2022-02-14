; ModuleID = './sample5.ll'
source_filename = "tritonModule"

; Function Attrs: mustprogress nofree norecurse nosync nounwind readnone willreturn
define i32 @__triton(i32 %SymVar_0, i32 %SymVar_1) local_unnamed_addr #0 {
entry:
  %0 = add i32 %SymVar_0, 1
  %1 = add i32 %SymVar_1, -1
  %2 = add i32 %SymVar_1, %SymVar_0
  %.not = icmp eq i32 %2, 1001
  %3 = select i1 %.not, i32 %0, i32 %1
  ret i32 %3
}

attributes #0 = { mustprogress nofree norecurse nosync nounwind readnone willreturn }
