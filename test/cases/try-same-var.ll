; ModuleID = '<stdin>'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [3 x i8] c"%d\00", align 1

; Function Attrs: nounwind uwtable
define i8* @klock_getT2() #0 {
entry:
  %call = call i32 (i32, ...)* bitcast (i32 (...)* @malloc to i32 (i32, ...)*)(i32 4) #2, !dbg !18
  %conv = sext i32 %call to i64, !dbg !18
  %0 = inttoptr i64 %conv to i8*, !dbg !18
  ret i8* %0, !dbg !18
}

declare i32 @malloc(...) #1

; Function Attrs: nounwind uwtable
define i32 @getTIssue(i32 %x) #0 {
entry:
  %call = call i8* @klock_getT2() #2, !dbg !19
  %0 = bitcast i8* %call to i32*, !dbg !19
  br label %while.cond, !dbg !21

while.cond:                                       ; preds = %while.body, %entry
  %a.0 = phi i32* [ %0, %entry ], [ %1, %while.body ]
  %c.0 = phi i32 [ 30, %entry ], [ %sub, %while.body ]
  %cmp = icmp sgt i32 %c.0, 20, !dbg !21
  br i1 %cmp, label %while.body, label %while.end, !dbg !21

while.body:                                       ; preds = %while.cond
  %call1 = call i8* @klock_getT2() #2, !dbg !22
  %1 = bitcast i8* %call1 to i32*, !dbg !22
  %2 = load i32* %a.0, align 4, !dbg !24
  %3 = load i32* %1, align 4, !dbg !24
  %sub = sub nsw i32 %2, %3, !dbg !24
  br label %while.cond, !dbg !25

while.end:                                        ; preds = %while.cond
  ret i32 %c.0, !dbg !26
}

; Function Attrs: nounwind uwtable
define i32 @main() #0 {
entry:
  %a = alloca i32, align 4
  %call = call i32 (i8*, ...)* @__isoc99_scanf(i8* getelementptr inbounds ([3 x i8]* @.str, i32 0, i32 0), i32* %a) #2, !dbg !27
  %0 = load i32* %a, align 4, !dbg !28
  %call1 = call i32 @getTIssue(i32 %0) #2, !dbg !28
  store i32 %call1, i32* %a, align 4, !dbg !28
  %1 = load i32* %a, align 4, !dbg !29
  ret i32 %1, !dbg !29
}

declare i32 @__isoc99_scanf(i8*, ...) #1

attributes #0 = { nounwind uwtable "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf"="true" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf"="true" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nobuiltin }

!llvm.dbg.cu = !{!0}

!0 = metadata !{i32 786449, metadata !1, i32 12, metadata !"clang version 3.3 (http://llvm.org/git/clang.git 82a5430ee824bbe3a49360db41d53d316f9fd016) (ssh://git@sp11.ecn.purdue.edu/export3/git/repo/timeBugs/llvm.git f667db3652e1fd198ce4e3aec4cebf080a124552)", i1 false, metadata !"", i32 0, metadata !2, metadata !2, metadata !3, metadata !2, metadata !2, metadata !""} ; [ DW_TAG_compile_unit ] [/export2/jindal0/kint/test/cases/try-same-var.c] [DW_LANG_C99]
!1 = metadata !{metadata !"try-same-var.c", metadata !"/export2/jindal0/kint/test/cases"}
!2 = metadata !{i32 0}
!3 = metadata !{metadata !4, metadata !10, metadata !15}
!4 = metadata !{i32 786478, metadata !5, metadata !6, metadata !"klock_getT2", metadata !"klock_getT2", metadata !"", i32 7, metadata !7, i1 false, i1 true, i32 0, i32 0, null, i32 0, i1 false, i8* ()* @klock_getT2, null, null, metadata !2, i32 7} ; [ DW_TAG_subprogram ] [line 7] [def] [klock_getT2]
!5 = metadata !{metadata !"./compiler.h", metadata !"/export2/jindal0/kint/test/cases"}
!6 = metadata !{i32 786473, metadata !5}          ; [ DW_TAG_file_type ] [/export2/jindal0/kint/test/cases/./compiler.h]
!7 = metadata !{i32 786453, i32 0, i32 0, metadata !"", i32 0, i64 0, i64 0, i64 0, i32 0, null, metadata !8, i32 0, i32 0} ; [ DW_TAG_subroutine_type ] [line 0, size 0, align 0, offset 0] [from ]
!8 = metadata !{metadata !9}
!9 = metadata !{i32 786447, null, null, metadata !"", i32 0, i64 64, i64 64, i64 0, i32 0, null} ; [ DW_TAG_pointer_type ] [line 0, size 64, align 64, offset 0] [from ]
!10 = metadata !{i32 786478, metadata !1, metadata !11, metadata !"getTIssue", metadata !"getTIssue", metadata !"", i32 2, metadata !12, i1 false, i1 true, i32 0, i32 0, null, i32 256, i1 false, i32 (i32)* @getTIssue, null, null, metadata !2, i32 2} ; [ DW_TAG_subprogram ] [line 2] [def] [getTIssue]
!11 = metadata !{i32 786473, metadata !1}         ; [ DW_TAG_file_type ] [/export2/jindal0/kint/test/cases/try-same-var.c]
!12 = metadata !{i32 786453, i32 0, i32 0, metadata !"", i32 0, i64 0, i64 0, i64 0, i32 0, null, metadata !13, i32 0, i32 0} ; [ DW_TAG_subroutine_type ] [line 0, size 0, align 0, offset 0] [from ]
!13 = metadata !{metadata !14, metadata !14}
!14 = metadata !{i32 786468, null, null, metadata !"int", i32 0, i64 32, i64 32, i64 0, i32 0, i32 5} ; [ DW_TAG_base_type ] [int] [line 0, size 32, align 32, offset 0, enc DW_ATE_signed]
!15 = metadata !{i32 786478, metadata !1, metadata !11, metadata !"main", metadata !"main", metadata !"", i32 14, metadata !16, i1 false, i1 true, i32 0, i32 0, null, i32 0, i1 false, i32 ()* @main, null, null, metadata !2, i32 14} ; [ DW_TAG_subprogram ] [line 14] [def] [main]
!16 = metadata !{i32 786453, i32 0, i32 0, metadata !"", i32 0, i64 0, i64 0, i64 0, i32 0, null, metadata !17, i32 0, i32 0} ; [ DW_TAG_subroutine_type ] [line 0, size 0, align 0, offset 0] [from ]
!17 = metadata !{metadata !14}
!18 = metadata !{i32 8, i32 0, metadata !4, null}
!19 = metadata !{i32 5, i32 0, metadata !20, null}
!20 = metadata !{i32 786443, metadata !1, metadata !10} ; [ DW_TAG_lexical_block ] [/export2/jindal0/kint/test/cases/try-same-var.c]
!21 = metadata !{i32 7, i32 0, metadata !20, null}
!22 = metadata !{i32 8, i32 0, metadata !23, null}
!23 = metadata !{i32 786443, metadata !1, metadata !20, i32 7, i32 0, i32 0} ; [ DW_TAG_lexical_block ] [/export2/jindal0/kint/test/cases/try-same-var.c]
!24 = metadata !{i32 9, i32 0, metadata !23, null}
!25 = metadata !{i32 11, i32 0, metadata !23, null}
!26 = metadata !{i32 12, i32 0, metadata !20, null}
!27 = metadata !{i32 16, i32 0, metadata !15, null}
!28 = metadata !{i32 17, i32 0, metadata !15, null}
!29 = metadata !{i32 18, i32 0, metadata !15, null}
