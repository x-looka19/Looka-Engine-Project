# حماية الكود
-keep class com.looka.engine.** { *; }
-keepclasseswithmembernames class * {
    native <methods>;
}

# إزالة معلومات التصحيح
-assumenosideeffects class android.util.Log {
    public static *** d(...);
    public static *** v(...);
    public static *** i(...);
    public static *** w(...);
}

# تحسينات التجميع
-optimizations !code/simplification/arithmetic,!field/*,!class/merging/*
-optimizationpasses 5
-dontusemixedcaseclassnames
-dontskipnonpubliclibraryclasses
-verbose

# الاحتفاظ بـ Annotations
-keepattributes *Annotation*
-keepattributes Signature

# الاحتفاظ بالـ Resources
-keepclassmembers class **.R$* {
    public static <fields>;
}