; ModuleID = '<stdin>'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [3 x i8] c"%d\00", align 1

; Function Attrs: nounwind uwtable
define i8* @klock_getT2() #0 {
entry:
  %call = call i32 (i32, ...)* bitcast (i32 (...)* @malloc to i32 (i32, ...)*)(i32 4) #2, !dbg !19
  %conv = sext i32 %call to i64, !dbg !19
  %0 = inttoptr i64 %conv to i8*, !dbg !19
  ret i8* %0, !dbg !19
}

declare i32 @malloc(...) #1

; Function Attrs: nounwind uwtable
define i32 @getTIssue2(i32 %y) #0 {
entry:
  %call = call i8* @klock_getT2() #2, !dbg !20
  %0 = bitcast i8* %call to i32*, !dbg !20
  %1 = load i32* %0, align 4, !dbg !22
  %sub = sub nsw i32 %1, %y, !dbg !22
  ret i32 %sub, !dbg !22
}

; Function Attrs: nounwind uwtable
define i32 @getTIssue1(i32 %x) #0 {
entry:
  %call = call i8* @klock_getT2() #2, !dbg !23
  %0 = bitcast i8* %call to i32*, !dbg !23
  %1 = load i32* %0, align 4, !dbg !24
  %add = add nsw i32 %1, %x, !dbg !24
  ret i32 %add, !dbg !24
}

; Function Attrs: nounwind uwtable
define i32 @main() #0 {
entry:
  %a = alloca i32, align 4
  %call = call i32 (i8*, ...)* @__isoc99_scanf(i8* getelementptr inbounds ([3 x i8]* @.str, i32 0, i32 0), i32* %a) #2, !dbg !25
  %0 = load i32* %a, align 4, !dbg !26
  %call1 = call i32 @getTIssue1(i32 %0) #2, !dbg !26
  store i32 %call1, i32* %a, align 4, !dbg !26
  %1 = load i32* %a, align 4, !dbg !27
  %call2 = call i32 @getTIssue2(i32 %1) #2, !dbg !27
  %2 = load i32* %a, align 4, !dbg !28
  %sub = sub nsw i32 %2, %call2, !dbg !28
  ret i32 %sub, !dbg !28
}

declare i32 @__isoc99_scanf(i8*, ...) #1

attributes #0 = { nounwind uwtable "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf"="true" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf"="true" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nobuiltin }

!llvm.dbg.cu = !{!0}

!0 = metadata !{i32 786449, metadata !1, i32 12, metadata !"clang version 3.3 (http://llvm.org/git/clang.git 82a5430ee824bbe3a49360db41d53d316f9fd016) (ssh://git@sp11.ecn.purdue.edu/export3/git/repo/timeBugs/llvm.git f667db3652e1fd198ce4e3aec4cebf080a124552)", i1 false, metadata !"", i32 0, metadata !2, metadata !2, metadata !3, metadata !2, metadata !2, metadata !""} ; [ DW_TAG_compile_unit ] [/export2/jindal0/kint/test/cases/try-func.c] [DW_LANG_C99]
!1 = metadata !{metadata !"try-func.c", metadata !"/export2/jindal0/kint/test/cases"}
!2 = metadata !{i32 0}
!3 = metadata !{metadata !4, metadata !10, metadata !15, metadata !16}
!4 = metadata !{i32 786478, metadata !5, metadata !6, metadata !"klock_getT2", metadata !"klock_getT2", metadata !"", i32 7, metadata !7, i1 false, i1 true, i32 0, i32 0, null, i32 0, i1 false, i8* ()* @klock_getT2, null, null, metadata !2, i32 7} ; [ DW_TAG_subprogram ] [line 7] [def] [klock_getT2]
!5 = metadata !{metadata !"./compiler.h", metadata !"/export2/jindal0/kint/test/cases"}
!6 = metadata !{i32 786473, metadata !5}          ; [ DW_TAG_file_type ] [/export2/jindal0/kint/test/cases/./compiler.h]
!7 = metadata !{i32 786453, i32 0, i32 0, metadata !"", i32 0, i64 0, i64 0, i64 0, i32 0, null, metadata !8, i32 0, i32 0} ; [ DW_TAG_subroutine_type ] [line 0, size 0, align 0, offset 0] [from ]
!8 = metadata !{metadata !9}
!9 = metadata !{i32 786447, null, null, metadata !"", i32 0, i64 64, i64 64, i64 0, i32 0, null} ; [ DW_TAG_pointer_type ] [line 0, size 64, align 64, offset 0] [from ]
!10 = metadata !{i32 786478, metadata !1, metadata !11, metadata !"getTIssue2", metadata !"getTIssue2", metadata !"", i32 10, metadata !12, i1 false, i1 true, i32 0, i32 0, null, i32 256, i1 false, i32 (i32)* @getTIssue2, null, null, metadata !2, i32 10} ; [ DW_TAG_subprogram ] [line 10] [def] [getTIssue2]
!11 = metadata !{i32 786473, metadata !1}         ; [ DW_TAG_file_type ] [/export2/jindal0/kint/test/cases/try-func.c]
!12 = metadata !{i32 786453, i32 0, i32 0, metadata !"", i32 0, i64 0, i64 0, i64 0, i32 0, null, metadata !13, i32 0, i32 0} ; [ DW_TAG_subroutine_type ] [line 0, size 0, align 0, offset 0] [from ]
!13 = metadata !{metadata !14, metadata !14}
!14 = metadata !{i32 786468, null, null, metadata !"int", i32 0, i64 32, i64 32, i64 0, i32 0, i32 5} ; [ DW_TAG_base_type ] [int] [line 0, size 32, align 32, offset 0, enc DW_ATE_signed]
!15 = metadata !{i32 786478, metadata !1, metadata !11, metadata !"getTIssue1", metadata !"getTIssue1", metadata !"", i32 16, metadata !12, i1 false, i1 true, i32 0, i32 0, null, i32 256, i1 false, i32 (i32)* @getTIssue1, null, null, metadata !2, i32 16} ; [ DW_TAG_subprogram ] [line 16] [def] [getTIssue1]
!16 = metadata !{i32 786478, metadata !1, metadata !11, metadata !"main", metadata !"main", metadata !"", i32 33, metadata !17, i1 false, i1 true, i32 0, i32 0, null, i32 0, i1 false, i32 ()* @main, null, null, metadata !2, i32 33} ; [ DW_TAG_subprogram ] [line 33] [def] [main]
!17 = metadata !{i32 786453, i32 0, i32 0, metadata !"", i32 0, i64 0, i64 0, i64 0, i32 0, null, metadata !18, i32 0, i32 0} ; [ DW_TAG_subroutine_type ] [line 0, size 0, align 0, offset 0] [from ]
!18 = metadata !{metadata !14}
!19 = metadata !{i32 8, i32 0, metadata !4, null}
!20 = metadata !{i32 12, i32 0, metadata !21, null}
!21 = metadata !{i32 786443, metadata !1, metadata !10} ; [ DW_TAG_lexical_block ] [/export2/jindal0/kint/test/cases/try-func.c]
!22 = metadata !{i32 13, i32 0, metadata !21, null}
!23 = metadata !{i32 18, i32 0, metadata !15, null}
!24 = metadata !{i32 19, i32 0, metadata !15, null}
!25 = metadata !{i32 35, i32 0, metadata !16, null}
!26 = metadata !{i32 36, i32 0, metadata !16, null}
!27 = metadata !{i32 37, i32 0, metadata !16, null}
!28 = metadata !{i32 41, i32 0, metadata !16, null}
