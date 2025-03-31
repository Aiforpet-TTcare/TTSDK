#include <jni.h>
#include <vector>
#include <cstring>
#include <algorithm>
#include <android/bitmap.h>
#include <cstdlib>
#include <android/log.h>


extern "C"
JNIEXPORT jobject JNICALL
Java_com_aiforpet_pet_check_SkinCameraActivity_cropBitmapByRectNative(
        JNIEnv *env,
        jobject thiz,
        jobject srcBitmap,
        jint x1,
        jint y1,
        jint x2,
        jint y2
) {
    // 1) 원본 Bitmap 정보 획득
    AndroidBitmapInfo srcInfo;
    if (AndroidBitmap_getInfo(env, srcBitmap, &srcInfo) < 0) {
        // 필요하면 예외 처리
        return nullptr;
    }
    if (srcInfo.format != ANDROID_BITMAP_FORMAT_RGBA_8888) {
        // 필요하면 예외 처리
        return nullptr;
    }

    int srcW = srcInfo.width;
    int srcH = srcInfo.height;

    // 2) 잘라낼 영역의 좌표 보정/확정
    //    (넘치지 않도록 경계 검사)
    if (x1 < 0)       x1 = 0;
    if (y1 < 0)       y1 = 0;
    if (x2 > srcW)    x2 = srcW;
    if (y2 > srcH)    y2 = srcH;
    if (x2 < x1)      x2 = x1;  // 혹은 필요하면 return nullptr 등 처리
    if (y2 < y1)      y2 = y1;  // 마찬가지

    int cropWidth  = x2 - x1;
    int cropHeight = y2 - y1;
    if (cropWidth <= 0 || cropHeight <= 0) {
        // 잘라낼 영역이 없으면 null 등의 처리
        return nullptr;
    }

    // 3) 원본 Bitmap lock
    void* srcPixels = nullptr;
    if (AndroidBitmap_lockPixels(env, srcBitmap, &srcPixels) < 0) {
        // 필요하면 예외 처리
        return nullptr;
    }

    // 4) 새 Bitmap 생성 (cropWidth x cropHeight)
    jclass bmpCls = env->FindClass("android/graphics/Bitmap");
    jclass configCls = env->FindClass("android/graphics/Bitmap$Config");
    jfieldID argb8888ID = env->GetStaticFieldID(configCls, "ARGB_8888", "Landroid/graphics/Bitmap$Config;");
    jobject argb8888Obj = env->GetStaticObjectField(configCls, argb8888ID);

    jmethodID createBitmap3 = env->GetStaticMethodID(
            bmpCls,
            "createBitmap",
            "(IILandroid/graphics/Bitmap$Config;)Landroid/graphics/Bitmap;"
    );
    jobject dstBitmap = env->CallStaticObjectMethod(
            bmpCls,
            createBitmap3,
            cropWidth,
            cropHeight,
            argb8888Obj
    );
    if (!dstBitmap) {
        // 해제
        AndroidBitmap_unlockPixels(env, srcBitmap);
        return nullptr;
    }

    // lock dst
    AndroidBitmapInfo dstInfo;
    if (AndroidBitmap_getInfo(env, dstBitmap, &dstInfo) < 0) {
        AndroidBitmap_unlockPixels(env, srcBitmap);
        return nullptr;
    }
    void* dstPixels = nullptr;
    if (AndroidBitmap_lockPixels(env, dstBitmap, &dstPixels) < 0) {
        AndroidBitmap_unlockPixels(env, srcBitmap);
        return nullptr;
    }

    // 5) 실제 픽셀 복사
    uint8_t* srcPtr = (uint8_t*)srcPixels;
    uint8_t* dstPtr = (uint8_t*)dstPixels;

    int srcStride = srcInfo.stride;  // 한 줄 당 바이트 수
    int dstStride = dstInfo.stride;

    for (int row = 0; row < cropHeight; row++) {
        int srcY = y1 + row;
        // 원본에서 해당 줄 주소
        uint8_t* srcLine = srcPtr + (srcY * srcStride);
        // 잘라낸 결과에서 해당 줄 주소
        uint8_t* dstLine = dstPtr + (row * dstStride);

        for (int col = 0; col < cropWidth; col++) {
            int srcX = x1 + col;
            // ARGB_8888 은 픽셀당 4바이트
            uint8_t* srcPx = srcLine + (srcX * 4);
            uint8_t* dstPx = dstLine + (col * 4);

            // R/G/B/A 4바이트 복사
            dstPx[0] = srcPx[0];
            dstPx[1] = srcPx[1];
            dstPx[2] = srcPx[2];
            dstPx[3] = srcPx[3];
        }
    }

    // unlock
    AndroidBitmap_unlockPixels(env, dstBitmap);
    AndroidBitmap_unlockPixels(env, srcBitmap);

    // 잘라낸 결과 Bitmap 리턴
    return dstBitmap;
}