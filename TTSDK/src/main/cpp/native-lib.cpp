#include <jni.h>
#include <vector>
#include <cstring>
#include <algorithm>
#include <android/bitmap.h>
#include <cstdlib>
#include <android/log.h>

#define LOG_TAG "JNI_DEBUG"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

extern "C" JNIEXPORT jobject JNICALL
Java_com_aiforpet_pet_activity_check_JointActivity_processImageNative(JNIEnv *env, jobject thiz, jbyteArray input, jint width, jint height) {
    jsize inputLength = env->GetArrayLength(input);
    jbyte* inputImage = env->GetByteArrayElements(input, nullptr);

    // Calculate the scale factor
    float scale = 640.0f / std::max(width, height);
    int newWidth = static_cast<int>(width * scale);
    int newHeight = static_cast<int>(height * scale);

    // Create the output image with padding, 3 channels (RGB)
    std::vector<float> outputImage(640 * 640 * 3, 0); // Initialize with black padding

    // Scale the input image and copy to the output image
    for (int y = 0; y < newHeight; ++y) {
        for (int x = 0; x < newWidth; ++x) {
            int srcX = static_cast<int>(x / scale);
            int srcY = static_cast<int>(y / scale);
            int srcIndex = srcY * width + srcX;
            int destIndex = ((y + (640 - newHeight) / 2) * 640 + (x + (640 - newWidth) / 2)) * 3;
            uint8_t pixelValue = static_cast<uint8_t>(inputImage[srcIndex]);
            // Assuming the input image is a single channel (grayscale)
            float normalizedPixelValue = pixelValue / 255.0f;
            outputImage[destIndex] = normalizedPixelValue;
            outputImage[destIndex + 1] = normalizedPixelValue;
            outputImage[destIndex + 2] = normalizedPixelValue;
        }
    }

    // Create a direct ByteBuffer from the output image
    jobject byteBuffer = env->NewDirectByteBuffer(outputImage.data(), outputImage.size() * sizeof(float));

    // Release the input image data
    env->ReleaseByteArrayElements(input, inputImage, JNI_ABORT);

    return byteBuffer;
}


extern "C" JNIEXPORT jobject JNICALL
Java_com_aiforpet_pet_activity_check_EyeCameraActivity_bitmapToByteBuffer(JNIEnv *env, jobject thiz, jobject bitmap) {
    AndroidBitmapInfo info;
    void* pixels;
    int ret;

    // Bitmap 정보 가져오기
    if ((ret = AndroidBitmap_getInfo(env, bitmap, &info)) < 0) {
        LOGD("AndroidBitmap_getInfo failed");
        return NULL;
    }

    // Bitmap 포맷이 RGBA_8888인지 확인
    if (info.format != ANDROID_BITMAP_FORMAT_RGBA_8888) {
        LOGD("Bitmap format is not RGBA_8888");
        return NULL;
    }

    // Bitmap 잠금
    if ((ret = AndroidBitmap_lockPixels(env, bitmap, &pixels)) < 0) {
        LOGD("AndroidBitmap_lockPixels failed");
        return NULL;
    }

    int width = 320;
    int height = 320;

    // ByteBuffer 할당
    size_t bufferSize = width * height * 3 * sizeof(float);
    float* buffer = (float*) malloc(bufferSize);
    if (buffer == nullptr) {
        LOGD("malloc failed");
        AndroidBitmap_unlockPixels(env, bitmap);
        return NULL;
    }

    jobject byteBuffer = env->NewDirectByteBuffer(buffer, bufferSize);
    if (byteBuffer == nullptr) {
        LOGD("NewDirectByteBuffer failed");
        free(buffer);
        AndroidBitmap_unlockPixels(env, bitmap);
        return NULL;
    }

    // Bitmap 처리 및 ByteBuffer 채우기
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            uint32_t pixel = *((uint32_t*)((char*)pixels + y * info.stride) + x);

            int r = (pixel >> 16) & 0xFF;
            int g = (pixel >> 8) & 0xFF;
            int b = pixel & 0xFF;

            // Normalize channel values to [-1.0, 1.0]
            float rf = (r - 127.0f) / 128.0f;
            float gf = (g - 127.0f) / 128.0f;
            float bf = (b - 127.0f) / 128.0f;

            buffer[(y * width + x) * 3 + 0] = bf; // Blue
            buffer[(y * width + x) * 3 + 1] = gf; // Green
            buffer[(y * width + x) * 3 + 2] = rf; // Red


        }
    }

    // Bitmap 잠금 해제
    AndroidBitmap_unlockPixels(env, bitmap);

    LOGD("Bitmap processing completed");

    // 10개의 값을 출력하여 확인
    for (int i = 0; i < 30; i++) {
        LOGD("Buffer value at index %d: %f", i, buffer[i]);
    }

    return byteBuffer;
}



extern "C" JNIEXPORT jobject JNICALL
Java_com_aiforpet_pet_activity_check_EyeCameraActivity_extractAndResizeBitmapToByteBuffer(JNIEnv *env, jobject thiz, jobject bitmap, jobject rectF) {
    AndroidBitmapInfo info;
    void* pixels;
    int ret;

    if ((ret = AndroidBitmap_getInfo(env, bitmap, &info)) < 0) {
        LOGD("AndroidBitmap_getInfo failed");
        return NULL;
    }

    if ((ret = AndroidBitmap_lockPixels(env, bitmap, &pixels)) < 0) {
        LOGD("AndroidBitmap_lockPixels failed");
        return NULL;
    }

    jclass rectFClass = env->GetObjectClass(rectF);
    jfieldID fidLeft = env->GetFieldID(rectFClass, "left", "F");
    jfieldID fidTop = env->GetFieldID(rectFClass, "top", "F");
    jfieldID fidRight = env->GetFieldID(rectFClass, "right", "F");
    jfieldID fidBottom = env->GetFieldID(rectFClass, "bottom", "F");

    float left = env->GetFloatField(rectF, fidLeft);
    float top = env->GetFloatField(rectF, fidTop);
    float right = env->GetFloatField(rectF, fidRight);
    float bottom = env->GetFloatField(rectF, fidBottom);

    float squareSize = fmax(right - left, bottom - top);
    float centerX = (left + right) / 2;
    float centerY = (top + bottom) / 2;
    float newLeft = centerX - squareSize * 0.4f;
    float newTop = centerY - squareSize * 0.4f;
    float newRight = centerX + squareSize * 0.4f;
    float newBottom = centerY + squareSize * 0.4f;

    if (newRight > info.width) {
        newLeft -= (newRight - info.width);
        newRight = info.width;
    }
    if (newBottom > info.height) {
        newTop -= (newBottom - info.height);
        newBottom = info.height;
    }
    if (newLeft < 0) {
        newRight += -newLeft;
        newLeft = 0;
    }
    if (newTop < 0) {
        newBottom += -newTop;
        newTop = 0;
    }

    float widthIncrease = squareSize / 2;
    float heightIncrease = squareSize / 2;
    float expandedLeft = fmax(0, newLeft - widthIncrease);
    float expandedTop = fmax(0, newTop - heightIncrease);
    float expandedRight = fmin(info.width, newRight + widthIncrease);
    float expandedBottom = fmin(info.height, newBottom + heightIncrease);

    float expandedSquareSize = fmax(expandedRight - expandedLeft, expandedBottom - expandedTop);
    float expandedCenterX = (expandedLeft + expandedRight) / 2;
    float expandedCenterY = (expandedTop + expandedBottom) / 2;
    float finalLeft = fmax(0, expandedCenterX - expandedSquareSize / 2);
    float finalTop = fmax(0, expandedCenterY - expandedSquareSize / 2);
    float finalRight = fmin(info.width, expandedCenterX + expandedSquareSize / 2);
    float finalBottom = fmin(info.height, expandedCenterY + expandedSquareSize / 2);

    int cropWidth = (int)(finalRight - finalLeft);
    int cropHeight = (int)(finalBottom - finalTop);

    size_t bufferSize = 320 * 320 * 3 * sizeof(float);
    float* buffer = (float*) malloc(bufferSize);
    if (buffer == nullptr) {
        LOGD("malloc failed");
        AndroidBitmap_unlockPixels(env, bitmap);
        return NULL;
    }

    jobject byteBuffer = env->NewDirectByteBuffer(buffer, bufferSize);
    if (byteBuffer == nullptr) {
        LOGD("NewDirectByteBuffer failed");
        free(buffer);
        AndroidBitmap_unlockPixels(env, bitmap);
        return NULL;
    }

    for (int y = 0; y < 320; y++) {
        for (int x = 0; x < 320; x++) {
            int srcX = (int)finalLeft + (int)((float)x / 320 * cropWidth);
            int srcY = (int)finalTop + (int)((float)y / 320 * cropHeight);

            uint32_t pixel = *((uint32_t*)((char*)pixels + srcY * info.stride) + srcX);

            int r = (pixel >> 16) & 0xFF;
            int g = (pixel >> 8) & 0xFF;
            int b = pixel & 0xFF;

            float rf = (r - 127.0f) / 128.0f;
            float gf = (g - 127.0f) / 128.0f;
            float bf = (b - 127.0f) / 128.0f;

            buffer[(y * 320 + x) * 3 + 0] = bf;
            buffer[(y * 320 + x) * 3 + 1] = gf;
            buffer[(y * 320 + x) * 3 + 2] = rf;


        }
    }

    AndroidBitmap_unlockPixels(env, bitmap);
    LOGD("Bitmap processing completed");


    return byteBuffer;
}



extern "C" JNIEXPORT jobject JNICALL
Java_com_aiforpet_pet_activity_check_ToothCameraActivity_bitmapToByteBuffer(JNIEnv *env, jobject thiz, jobject bitmap) {
    AndroidBitmapInfo info;
    void* pixels;
    int ret;

    // Bitmap 정보 가져오기
    if ((ret = AndroidBitmap_getInfo(env, bitmap, &info)) < 0) {
        LOGD("AndroidBitmap_getInfo failed");
        return NULL;
    }

    // Bitmap 포맷이 RGBA_8888인지 확인
    if (info.format != ANDROID_BITMAP_FORMAT_RGBA_8888) {
        LOGD("Bitmap format is not RGBA_8888");
        return NULL;
    }

    // Bitmap 잠금
    if ((ret = AndroidBitmap_lockPixels(env, bitmap, &pixels)) < 0) {
        LOGD("AndroidBitmap_lockPixels failed");
        return NULL;
    }

    int width = 320;
    int height = 320;

    // ByteBuffer 할당
    size_t bufferSize = width * height * 3 * sizeof(float);
    float* buffer = (float*) malloc(bufferSize);
    if (buffer == nullptr) {
        LOGD("malloc failed");
        AndroidBitmap_unlockPixels(env, bitmap);
        return NULL;
    }

    jobject byteBuffer = env->NewDirectByteBuffer(buffer, bufferSize);
    if (byteBuffer == nullptr) {
        LOGD("NewDirectByteBuffer failed");
        free(buffer);
        AndroidBitmap_unlockPixels(env, bitmap);
        return NULL;
    }

    // Bitmap 처리 및 ByteBuffer 채우기
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            uint32_t pixel = *((uint32_t*)((char*)pixels + y * info.stride) + x);

            int r = (pixel >> 16) & 0xFF;
            int g = (pixel >> 8) & 0xFF;
            int b = pixel & 0xFF;

            // Normalize channel values to [-1.0, 1.0]
            float rf = (r - 127.0f) / 128.0f;
            float gf = (g - 127.0f) / 128.0f;
            float bf = (b - 127.0f) / 128.0f;

            buffer[(y * width + x) * 3 + 0] = bf; // Blue
            buffer[(y * width + x) * 3 + 1] = gf; // Green
            buffer[(y * width + x) * 3 + 2] = rf; // Red


        }
    }

    // Bitmap 잠금 해제
    AndroidBitmap_unlockPixels(env, bitmap);

    LOGD("Bitmap processing completed");



    return byteBuffer;
}



extern "C" JNIEXPORT jobject JNICALL
Java_com_aiforpet_pet_activity_check_ToothCameraActivity_extractAndResizeBitmapToByteBuffer(JNIEnv *env, jobject thiz, jobject bitmap, jobject rectF) {
    AndroidBitmapInfo info;
    void* pixels;
    int ret;

    if ((ret = AndroidBitmap_getInfo(env, bitmap, &info)) < 0) {
        LOGD("AndroidBitmap_getInfo failed");
        return NULL;
    }

    if ((ret = AndroidBitmap_lockPixels(env, bitmap, &pixels)) < 0) {
        LOGD("AndroidBitmap_lockPixels failed");
        return NULL;
    }

    jclass rectFClass = env->GetObjectClass(rectF);
    jfieldID fidLeft = env->GetFieldID(rectFClass, "left", "F");
    jfieldID fidTop = env->GetFieldID(rectFClass, "top", "F");
    jfieldID fidRight = env->GetFieldID(rectFClass, "right", "F");
    jfieldID fidBottom = env->GetFieldID(rectFClass, "bottom", "F");

    float left = env->GetFloatField(rectF, fidLeft);
    float top = env->GetFloatField(rectF, fidTop);
    float right = env->GetFloatField(rectF, fidRight);
    float bottom = env->GetFloatField(rectF, fidBottom);

    float squareSize = fmax(right - left, bottom - top);
    float centerX = (left + right) / 2;
    float centerY = (top + bottom) / 2;
    float newLeft = centerX - squareSize * 0.2f;
    float newTop = centerY - squareSize * 0.2f;
    float newRight = centerX + squareSize * 0.2f;
    float newBottom = centerY + squareSize * 0.2f;

    if (newRight > info.width) {
        newLeft -= (newRight - info.width);
        newRight = info.width;
    }
    if (newBottom > info.height) {
        newTop -= (newBottom - info.height);
        newBottom = info.height;
    }
    if (newLeft < 0) {
        newRight += -newLeft;
        newLeft = 0;
    }
    if (newTop < 0) {
        newBottom += -newTop;
        newTop = 0;
    }

    float widthIncrease = squareSize / 2;
    float heightIncrease = squareSize / 2;
    float expandedLeft = fmax(0, newLeft - widthIncrease);
    float expandedTop = fmax(0, newTop - heightIncrease);
    float expandedRight = fmin(info.width, newRight + widthIncrease);
    float expandedBottom = fmin(info.height, newBottom + heightIncrease);

    float expandedSquareSize = fmax(expandedRight - expandedLeft, expandedBottom - expandedTop);
    float expandedCenterX = (expandedLeft + expandedRight) / 2;
    float expandedCenterY = (expandedTop + expandedBottom) / 2;
    float finalLeft = fmax(0, expandedCenterX - expandedSquareSize / 2);
    float finalTop = fmax(0, expandedCenterY - expandedSquareSize / 2);
    float finalRight = fmin(info.width, expandedCenterX + expandedSquareSize / 2);
    float finalBottom = fmin(info.height, expandedCenterY + expandedSquareSize / 2);

    int cropWidth = (int)(finalRight - finalLeft);
    int cropHeight = (int)(finalBottom - finalTop);

    size_t bufferSize = 320 * 320 * 3 * sizeof(float);
    float* buffer = (float*) malloc(bufferSize);
    if (buffer == nullptr) {
        LOGD("malloc failed");
        AndroidBitmap_unlockPixels(env, bitmap);
        return NULL;
    }

    jobject byteBuffer = env->NewDirectByteBuffer(buffer, bufferSize);
    if (byteBuffer == nullptr) {
        LOGD("NewDirectByteBuffer failed");
        free(buffer);
        AndroidBitmap_unlockPixels(env, bitmap);
        return NULL;
    }

    for (int y = 0; y < 320; y++) {
        for (int x = 0; x < 320; x++) {
            int srcX = (int)finalLeft + (int)((float)x / 320 * cropWidth);
            int srcY = (int)finalTop + (int)((float)y / 320 * cropHeight);

            uint32_t pixel = *((uint32_t*)((char*)pixels + srcY * info.stride) + srcX);

            int r = (pixel >> 16) & 0xFF;
            int g = (pixel >> 8) & 0xFF;
            int b = pixel & 0xFF;

            float rf = (r - 127.0f) / 128.0f;
            float gf = (g - 127.0f) / 128.0f;
            float bf = (b - 127.0f) / 128.0f;

            buffer[(y * 320 + x) * 3 + 0] = bf;
            buffer[(y * 320 + x) * 3 + 1] = gf;
            buffer[(y * 320 + x) * 3 + 2] = rf;


        }
    }

    AndroidBitmap_unlockPixels(env, bitmap);
    LOGD("Bitmap processing completed");

    for (int i = 0; i < 30; i++) {
        LOGD("Buffer value at index %d: %f", i, buffer[i]);
    }

    return byteBuffer;
}






