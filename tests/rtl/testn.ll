; ModuleID = 'testn.cpp'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%union.pthread_attr_t = type { i64, [48 x i8] }

@g_x = global i32 0, align 4
@g_y = global i32 0, align 4
@g_z = global i32 0, align 4
@.str = private unnamed_addr constant [9 x i8] c"failed C\00", align 1
@.str.1 = private unnamed_addr constant [13 x i8] c"\0D\0A\0D\0A%d  %d\0D\0A\00", align 1
@llvm.global_ctors = appending global [1 x { i32, void ()*, i8* }] [{ i32, void ()*, i8* } { i32 0, void ()* @tsan.module_ctor, i8* null }]

; Function Attrs: nounwind sanitize_thread uwtable
define i8* @_Z2t1Pv(i8* %pr) #0 {
entry:
  %0 = call i8* @llvm.returnaddress(i32 0)
  call void @__tsan_func_entry(i8* %0)
  %pr.addr = alloca i8*, align 8
  store i8* %pr, i8** %pr.addr, align 8
  call void @__tsan_read4(i8* bitcast (i32* @g_x to i8*))
  %1 = load i32, i32* @g_x, align 4
  call void @__tsan_read4(i8* bitcast (i32* @g_z to i8*))
  %2 = load i32, i32* @g_z, align 4
  %mul = mul nsw i32 %1, %2
  %3 = load i32, i32* @g_y, align 4
  %add = add nsw i32 %3, %mul
  call void @__tsan_write4(i8* bitcast (i32* @g_y to i8*))
  store i32 %add, i32* @g_y, align 4
  call void @__tsan_func_exit()
  ret i8* null
}

; Function Attrs: norecurse sanitize_thread uwtable
define i32 @main() #1 {
entry:
  %0 = call i8* @llvm.returnaddress(i32 0)
  call void @__tsan_func_entry(i8* %0)
  %retval = alloca i32, align 4
  %pt1 = alloca i64, align 8
  %ret = alloca i32, align 4
  store i32 0, i32* %retval, align 4
  call void @__tsan_write4(i8* bitcast (i32* @g_x to i8*))
  store i32 0, i32* @g_x, align 4
  call void @__tsan_write4(i8* bitcast (i32* @g_y to i8*))
  store i32 0, i32* @g_y, align 4
  %call = call i8* @_Z2t1Pv(i8* null)
  %call1 = call i32 @pthread_create(i64* %pt1, %union.pthread_attr_t* null, i8* (i8*)* @_Z2t1Pv, i8* null) #5
  store i32 %call1, i32* %ret, align 4
  %1 = load i32, i32* %ret, align 4
  %cmp = icmp ne i32 %1, 0
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %call2 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str, i32 0, i32 0))
  br label %if.end

if.end:                                           ; preds = %if.then, %entry
  %call3 = call i8* @_Z2t1Pv(i8* null)
  call void @__tsan_write4(i8* bitcast (i32* @g_x to i8*))
  store i32 1, i32* @g_x, align 4
  call void @__tsan_write4(i8* bitcast (i32* @g_y to i8*))
  store i32 2, i32* @g_y, align 4
  %2 = bitcast i64* %pt1 to i8*
  call void @__tsan_read8(i8* %2)
  %3 = load i64, i64* %pt1, align 8
  %call4 = call i32 @pthread_join(i64 %3, i8** null)
  call void @__tsan_read4(i8* bitcast (i32* @g_x to i8*))
  %4 = load i32, i32* @g_x, align 4
  call void @__tsan_read4(i8* bitcast (i32* @g_y to i8*))
  %5 = load i32, i32* @g_y, align 4
  %call5 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([13 x i8], [13 x i8]* @.str.1, i32 0, i32 0), i32 %4, i32 %5)
  call void @__tsan_func_exit()
  ret i32 0
}

; Function Attrs: nounwind
declare i32 @pthread_create(i64*, %union.pthread_attr_t*, i8* (i8*)*, i8*) #2

declare i32 @printf(i8*, ...) #3

declare i32 @pthread_join(i64, i8**) #3

define internal void @tsan.module_ctor() {
  call void @__tsan_init()
  ret void
}

declare void @__tsan_init()

declare void @__tsan_func_entry(i8*)

declare void @__tsan_func_exit()

declare void @__tsan_read1(i8*)

declare void @__tsan_write1(i8*)

declare void @__tsan_unaligned_read1(i8*)

declare void @__tsan_unaligned_write1(i8*)

declare i8 @__tsan_atomic8_load(i8*, i32)

declare void @__tsan_atomic8_store(i8*, i8, i32)

declare i8 @__tsan_atomic8_exchange(i8*, i8, i32)

declare i8 @__tsan_atomic8_fetch_add(i8*, i8, i32)

declare i8 @__tsan_atomic8_fetch_sub(i8*, i8, i32)

declare i8 @__tsan_atomic8_fetch_and(i8*, i8, i32)

declare i8 @__tsan_atomic8_fetch_nand(i8*, i8, i32)

declare i8 @__tsan_atomic8_fetch_or(i8*, i8, i32)

declare i8 @__tsan_atomic8_fetch_xor(i8*, i8, i32)

declare i8 @__tsan_atomic8_compare_exchange_val(i8*, i8, i8, i32, i32)

declare void @__tsan_read2(i8*)

declare void @__tsan_write2(i8*)

declare void @__tsan_unaligned_read2(i8*)

declare void @__tsan_unaligned_write2(i8*)

declare i16 @__tsan_atomic16_load(i16*, i32)

declare void @__tsan_atomic16_store(i16*, i16, i32)

declare i16 @__tsan_atomic16_exchange(i16*, i16, i32)

declare i16 @__tsan_atomic16_fetch_add(i16*, i16, i32)

declare i16 @__tsan_atomic16_fetch_sub(i16*, i16, i32)

declare i16 @__tsan_atomic16_fetch_and(i16*, i16, i32)

declare i16 @__tsan_atomic16_fetch_nand(i16*, i16, i32)

declare i16 @__tsan_atomic16_fetch_or(i16*, i16, i32)

declare i16 @__tsan_atomic16_fetch_xor(i16*, i16, i32)

declare i16 @__tsan_atomic16_compare_exchange_val(i16*, i16, i16, i32, i32)

declare void @__tsan_read4(i8*)

declare void @__tsan_write4(i8*)

declare void @__tsan_unaligned_read4(i8*)

declare void @__tsan_unaligned_write4(i8*)

declare i32 @__tsan_atomic32_load(i32*, i32)

declare void @__tsan_atomic32_store(i32*, i32, i32)

declare i32 @__tsan_atomic32_exchange(i32*, i32, i32)

declare i32 @__tsan_atomic32_fetch_add(i32*, i32, i32)

declare i32 @__tsan_atomic32_fetch_sub(i32*, i32, i32)

declare i32 @__tsan_atomic32_fetch_and(i32*, i32, i32)

declare i32 @__tsan_atomic32_fetch_nand(i32*, i32, i32)

declare i32 @__tsan_atomic32_fetch_or(i32*, i32, i32)

declare i32 @__tsan_atomic32_fetch_xor(i32*, i32, i32)

declare i32 @__tsan_atomic32_compare_exchange_val(i32*, i32, i32, i32, i32)

declare void @__tsan_read8(i8*)

declare void @__tsan_write8(i8*)

declare void @__tsan_unaligned_read8(i8*)

declare void @__tsan_unaligned_write8(i8*)

declare i64 @__tsan_atomic64_load(i64*, i32)

declare void @__tsan_atomic64_store(i64*, i64, i32)

declare i64 @__tsan_atomic64_exchange(i64*, i64, i32)

declare i64 @__tsan_atomic64_fetch_add(i64*, i64, i32)

declare i64 @__tsan_atomic64_fetch_sub(i64*, i64, i32)

declare i64 @__tsan_atomic64_fetch_and(i64*, i64, i32)

declare i64 @__tsan_atomic64_fetch_nand(i64*, i64, i32)

declare i64 @__tsan_atomic64_fetch_or(i64*, i64, i32)

declare i64 @__tsan_atomic64_fetch_xor(i64*, i64, i32)

declare i64 @__tsan_atomic64_compare_exchange_val(i64*, i64, i64, i32, i32)

declare void @__tsan_read16(i8*)

declare void @__tsan_write16(i8*)

declare void @__tsan_unaligned_read16(i8*)

declare void @__tsan_unaligned_write16(i8*)

declare i128 @__tsan_atomic128_load(i128*, i32)

declare void @__tsan_atomic128_store(i128*, i128, i32)

declare i128 @__tsan_atomic128_exchange(i128*, i128, i32)

declare i128 @__tsan_atomic128_fetch_add(i128*, i128, i32)

declare i128 @__tsan_atomic128_fetch_sub(i128*, i128, i32)

declare i128 @__tsan_atomic128_fetch_and(i128*, i128, i32)

declare i128 @__tsan_atomic128_fetch_nand(i128*, i128, i32)

declare i128 @__tsan_atomic128_fetch_or(i128*, i128, i32)

declare i128 @__tsan_atomic128_fetch_xor(i128*, i128, i32)

declare i128 @__tsan_atomic128_compare_exchange_val(i128*, i128, i128, i32, i32)

declare void @__tsan_vptr_update(i8*, i8*)

declare void @__tsan_vptr_read(i8*)

declare void @__tsan_atomic_thread_fence(i32)

declare void @__tsan_atomic_signal_fence(i32)

declare i8* @memmove(i8*, i8*, i64)

declare i8* @memcpy(i8*, i8*, i64)

declare i8* @memset(i8*, i32, i64)

; Function Attrs: nounwind readnone
declare i8* @llvm.returnaddress(i32) #4

attributes #0 = { nounwind sanitize_thread uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { norecurse sanitize_thread uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { nounwind readnone }
attributes #5 = { nounwind }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.8.0 (tags/RELEASE_380/final)"}
