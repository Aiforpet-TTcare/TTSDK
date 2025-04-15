# (1) 난독화(off)
-dontobfuscate

# (2) 축소(off): 사용되지 않는 코드를 제거하지 않음
-dontshrink

# (3) 최적화(off): 최적화 과정을 생략
-dontoptimize

# (4) 사전검증(off) – 경우에 따라
-dontpreverify

# (5) 모든 클래스, 인터페이스, enum, annotation 이름과 멤버를 유지
-keep class * {
    *;
}
-keep interface * {
    *;
}
-keep enum * {
    *;
}
-keep @interface * {
    *;
}