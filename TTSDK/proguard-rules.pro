# Add project specific ProGuard rules here.
# You can control the set of applied configuration files using the
# proguardFiles setting in build.gradle.
#
# For more details, see
#   http://developer.android.com/guide/developing/tools/proguard.html

# If your project uses WebView with JS, uncomment the following
# and specify the fully qualified class name to the JavaScript interface
# class:
#-keepclassmembers class fqcn.of.javascript.interface.for.webview {
#   public *;
#}

# Uncomment this to preserve the line number information for
# debugging stack traces.
#-keepattributes SourceFile,LineNumberTable

# If you keep the line number information, uncomment this to
# hide the original source file name.
#-renamesourcefileattribute SourceFile


# androidx.camera:camera-core 관련 규칙
-keep class androidx.camera.** { *; }
-keep interface androidx.camera.** { *; }
-dontwarn androidx.camera.**

# org.tensorflow:tensorflow-lite 관련 규칙
-keep class org.tensorflow.lite.** { *; }
-keep interface org.tensorflow.lite.** { *; }
-dontwarn org.tensorflow.lite.**

# com.squareup.okhttp3 관련 규칙
-keep class com.squareup.okhttp3.** { *; }
-dontwarn com.squareup.okhttp3.**

# com.airbnb.android 관련 규칙
-keep class com.airbnb.android.** { *; }
-dontwarn com.airbnb.android.**

# androidx.appcompat 관련 규칙
-keep class androidx.appcompat.** { *; }
-dontwarn androidx.appcompat.**

# androidx.gridlayout 관련 규칙
-keep class androidx.gridlayout.** { *; }
-dontwarn androidx.gridlayout.**

# androidx.lifecycle 관련 규칙
-keep class androidx.lifecycle.** { *; }
-dontwarn androidx.lifecycle.**

# com.tbuonomo 관련 규칙
-keep class com.tbuonomo.** { *; }
-dontwarn com.tbuonomo.**

# com.squareup.picasso 관련 규칙
-keep class com.squareup.picasso.** { *; }
-keep class com.aiforpet.pet.network.TTRequest

-keep class com.aiforpet.pet.network.TTRequest {
    public void makeRequest(int, com.aiforpet.pet.network.TTRequest$CallbackListener);
}

-keep interface com.aiforpet.pet.network.TTRequest$CallbackListener {
    public void onSuccess(java.lang.String);
    public void onFailure(java.lang.String);
}


-dontwarn com.squareup.picasso.**

-keep class com.google.android.gms.internal.** { *; }
-keep class com.google.firebase.messaging.** { *; }


-keep public class com.aiforpet.pet.activity.PercentView
-keep public class com.aiforpet.pet.vet.pulltorefresh.FPullToRefreshView
-keep public class com.aiforpet.pet.activity.check.EyeCameraActivity
-keep public class com.aiforpet.pet.activity.check.SkinCameraActivity
-keep public class com.aiforpet.pet.activity.check.ToothCameraActivity
-keep public class com.aiforpet.pet.activity.check.KneeActivity
-keep public class com.aiforpet.pet.activity.check.GuideActivity



# JNI 함수가 있는 클래스는 난독화를 피합니다.
-keep class com.aiforpet.pet.activity.check.ToothCameraActivity {
    native <methods>;
}

# JNI 함수가 있는 클래스는 난독화를 피합니다.
-keep class com.aiforpet.pet.activity.check.EyeCameraActivity {
    native <methods>;
}

# JNI 함수가 있는 클래스는 난독화를 피합니다.
-keep class com.aiforpet.pet.activity.check.KneeActivity {
    native <methods>;
}

# RectF 클래스의 필드 이름도 난독화를 피합니다.
-keepclassmembers class android.graphics.RectF {
    float left;
    float top;
    float right;
    float bottom;
}


# LibraryLottieView 클래스와 해당 패키지 내의 모든 클래스의 난독화를 방지
-keep class com.aiforpet.pet.util.LibraryLottieView { *; }

# MixpanelTracker 인터페이스와 그 메서드 유지
-keep interface com.aiforpet.pet.util.MixpanelTracker {
    void trackEvent(java.lang.String, java.util.Map);
    void trackScreen(java.lang.String);
}

# MySDK 클래스와 그 메서드 유지
-keep class com.aiforpet.pet.util.MySDK {
    public static void setMixpanelTracker(com.aiforpet.pet.util.MixpanelTracker);
    public static void trackScreen(java.lang.String);
    public static void trackEvent(java.lang.String, java.util.Map);
}

# MixpanelTracker를 구현한 모든 클래스 유지
-keep class * implements com.aiforpet.pet.util.MixpanelTracker {
    public void trackEvent(java.lang.String, java.util.Map);
    public void trackScreen(java.lang.String);
}










# 전체 패키지의 모든 클래스와 멤버를 난독화하지 않도록 할 수도 있습니다.
#-keep class com.aiforpet.pet.util.** { *; }