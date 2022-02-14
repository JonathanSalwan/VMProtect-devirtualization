; ModuleID = 'tritonModule'
source_filename = "tritonModule"

define i32 @__triton(i32 %SymVar_1, i32 %SymVar_0) {
entry:
  %0 = add i32 %SymVar_1, %SymVar_0
  ret i32 %0
}
