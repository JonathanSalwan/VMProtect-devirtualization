; ModuleID = 'tritonModule'
source_filename = "tritonModule"

define i32 @__triton(i32 %SymVar_0, i32 %SymVar_1) {
entry:
  %0 = add i32 %SymVar_0, 1
  %1 = add i32 %SymVar_1, -1
  %2 = add i32 %SymVar_1, %SymVar_0
  %3 = xor i32 %2, -1
  %4 = xor i32 %2, -1
  %5 = and i32 %4, %3
  %6 = xor i32 %5, 1001
  %7 = add i32 %5, 1001
  %8 = xor i32 %5, 1001
  %9 = xor i32 %8, %7
  %10 = and i32 %9, %6
  %11 = and i32 %5, 1001
  %12 = xor i32 %11, %10
  %13 = lshr i32 %12, 31
  %14 = trunc i32 %13 to i1
  %15 = trunc i32 %7 to i8
  %16 = lshr i8 %15, 7
  %17 = trunc i8 %16 to i1
  %18 = trunc i32 %7 to i8
  %19 = lshr i8 %18, 6
  %20 = trunc i8 %19 to i1
  %21 = trunc i32 %7 to i8
  %22 = lshr i8 %21, 5
  %23 = trunc i8 %22 to i1
  %24 = trunc i32 %7 to i8
  %25 = lshr i8 %24, 4
  %26 = trunc i8 %25 to i1
  %27 = trunc i32 %7 to i8
  %28 = lshr i8 %27, 3
  %29 = trunc i8 %28 to i1
  %30 = trunc i32 %7 to i8
  %31 = lshr i8 %30, 2
  %32 = trunc i8 %31 to i1
  %33 = trunc i32 %7 to i8
  %34 = lshr i8 %33, 1
  %35 = trunc i8 %34 to i1
  %36 = trunc i32 %7 to i8
  %37 = trunc i8 %36 to i1
  %38 = xor i1 true, %37
  %39 = xor i1 %38, %35
  %40 = xor i1 %39, %32
  %41 = xor i1 %40, %29
  %42 = xor i1 %41, %26
  %43 = xor i1 %42, %23
  %44 = xor i1 %43, %20
  %45 = xor i1 %44, %17
  %46 = xor i32 %5, 1001
  %47 = xor i32 %7, %46
  %48 = and i32 16, %47
  %49 = icmp eq i32 16, %48
  %50 = select i1 %49, i1 true, i1 false
  %51 = icmp eq i32 %7, 0
  %52 = select i1 %51, i1 true, i1 false
  %53 = zext i32 %7 to i64
  %54 = lshr i64 %53, 31
  %55 = trunc i64 %54 to i1
  %56 = xor i32 %5, %7
  %57 = xor i32 %5, -1002
  %58 = and i32 %57, %56
  %59 = lshr i32 %58, 31
  %60 = trunc i32 %59 to i1
  %61 = zext i1 %60 to i22
  %62 = or i22 0, %61
  %63 = shl i22 %62, 1
  %64 = shl i22 %63, 1
  %65 = or i22 %64, 1
  %66 = shl i22 %65, 1
  %67 = shl i22 %66, 1
  %68 = zext i1 %55 to i22
  %69 = or i22 %67, %68
  %70 = shl i22 %69, 1
  %71 = zext i1 %52 to i22
  %72 = or i22 %70, %71
  %73 = shl i22 %72, 1
  %74 = shl i22 %73, 1
  %75 = zext i1 %50 to i22
  %76 = or i22 %74, %75
  %77 = shl i22 %76, 1
  %78 = shl i22 %77, 1
  %79 = zext i1 %45 to i22
  %80 = or i22 %78, %79
  %81 = shl i22 %80, 1
  %82 = or i22 %81, 1
  %83 = shl i22 %82, 1
  %84 = zext i1 %14 to i22
  %85 = or i22 %83, %84
  %86 = zext i22 %85 to i64
  %87 = xor i64 %86, -1
  %88 = zext i22 %85 to i64
  %89 = xor i64 %88, -1
  %90 = and i64 %89, %87
  %91 = xor i64 %90, -1
  %92 = and i64 2069, %91
  %93 = xor i32 %7, -1
  %94 = xor i32 %7, -1
  %95 = and i32 %94, %93
  %96 = trunc i32 %95 to i8
  %97 = lshr i8 %96, 7
  %98 = trunc i8 %97 to i1
  %99 = trunc i32 %95 to i8
  %100 = lshr i8 %99, 6
  %101 = trunc i8 %100 to i1
  %102 = trunc i32 %95 to i8
  %103 = lshr i8 %102, 5
  %104 = trunc i8 %103 to i1
  %105 = trunc i32 %95 to i8
  %106 = lshr i8 %105, 4
  %107 = trunc i8 %106 to i1
  %108 = trunc i32 %95 to i8
  %109 = lshr i8 %108, 3
  %110 = trunc i8 %109 to i1
  %111 = trunc i32 %95 to i8
  %112 = lshr i8 %111, 2
  %113 = trunc i8 %112 to i1
  %114 = trunc i32 %95 to i8
  %115 = lshr i8 %114, 1
  %116 = trunc i8 %115 to i1
  %117 = trunc i32 %95 to i8
  %118 = trunc i8 %117 to i1
  %119 = xor i1 true, %118
  %120 = xor i1 %119, %116
  %121 = xor i1 %120, %113
  %122 = xor i1 %121, %110
  %123 = xor i1 %122, %107
  %124 = xor i1 %123, %104
  %125 = xor i1 %124, %101
  %126 = xor i1 %125, %98
  %127 = icmp eq i32 %95, 0
  %128 = select i1 %127, i1 true, i1 false
  %129 = zext i32 %95 to i64
  %130 = lshr i64 %129, 31
  %131 = trunc i64 %130 to i1
  %132 = zext i1 %131 to i22
  %133 = or i22 4, %132
  %134 = shl i22 %133, 1
  %135 = zext i1 %128 to i22
  %136 = or i22 %134, %135
  %137 = shl i22 %136, 1
  %138 = shl i22 %137, 1
  %139 = shl i22 %138, 1
  %140 = shl i22 %139, 1
  %141 = zext i1 %126 to i22
  %142 = or i22 %140, %141
  %143 = shl i22 %142, 1
  %144 = or i22 %143, 1
  %145 = shl i22 %144, 1
  %146 = zext i22 %145 to i64
  %147 = xor i64 %146, -1
  %148 = or i64 2069, %147
  %149 = xor i64 %148, -1
  %150 = or i64 2069, %147
  %151 = xor i64 %150, -1
  %152 = and i64 %151, %149
  %153 = add i64 %152, %92
  %154 = xor i64 %153, -1
  %155 = and i64 64, %154
  %156 = lshr i64 %155, 3
  %157 = add i64 %156, 140737488347280
  %158 = trunc i64 %157 to i8
  %159 = xor i8 80, %158
  %160 = sub i8 80, %158
  %161 = xor i8 %160, %159
  %162 = and i8 16, %161
  %163 = icmp eq i8 16, %162
  %164 = select i1 %163, i1 true, i1 false
  %165 = icmp eq i1 %164, true
  %166 = select i1 %165, i32 %1, i32 %0
  ret i32 %166
}
