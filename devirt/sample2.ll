; ModuleID = 'tritonModule'
source_filename = "tritonModule"

define i32 @__triton(i32 %SymVar_0, i32 %SymVar_1) {
entry:
  %0 = xor i32 %SymVar_0, %SymVar_1
  ret i32 %0
}
