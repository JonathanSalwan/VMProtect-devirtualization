; ModuleID = 'tritonModule'
source_filename = "tritonModule"

define i32 @__triton(i8 %SymVar_0, i8 %SymVar_1) {
entry:
  %0 = xor i8 %SymVar_0, 92
  %1 = and i8 %SymVar_0, 0
  %2 = zext i8 %1 to i32
  %3 = or i32 0, %2
  %4 = shl i32 %3, 8
  %5 = zext i8 %0 to i32
  %6 = or i32 %4, %5
  %7 = and i8 %SymVar_1, 0
  %8 = zext i8 %7 to i32
  %9 = or i32 0, %8
  %10 = shl i32 %9, 8
  %11 = zext i8 %SymVar_1 to i32
  %12 = or i32 %10, %11
  %13 = zext i8 %7 to i32
  %14 = or i32 0, %13
  %15 = shl i32 %14, 8
  %16 = zext i8 %SymVar_1 to i32
  %17 = or i32 %15, %16
  %18 = lshr i32 %17, 7
  %19 = xor i32 %18, -1
  %20 = add i32 1, %19
  %21 = shl i32 %20, 8
  %22 = add i32 %21, %12
  %23 = add i32 %22, %6
  ret i32 %23
}
