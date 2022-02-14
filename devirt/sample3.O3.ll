; ModuleID = 'devirt/sample3.ll'
source_filename = "tritonModule"

; Function Attrs: mustprogress nofree norecurse nosync nounwind readnone willreturn
define i32 @__triton(i8 %SymVar_0, i8 %SymVar_1) local_unnamed_addr #0 {
entry:
  %0 = xor i8 %SymVar_0, 92
  %1 = zext i8 %0 to i32
  %2 = zext i8 %SymVar_1 to i32
  %3 = shl nuw nsw i32 %2, 1
  %4 = and i32 %3, 256
  %5 = add nuw nsw i32 %1, %2
  %6 = sub nsw i32 %5, %4
  ret i32 %6
}

attributes #0 = { mustprogress nofree norecurse nosync nounwind readnone willreturn }
