#include <jni.h>
#include <string>
#include <android/log.h>

#define LOG_TAG "UmamusumeLocalifySettings"

extern "C" JNIEXPORT jstring JNICALL
Java_com_kimjio_umamusumelocalify_settings_ModuleUtils_getModuleVersion(
        JNIEnv* env,
        jclass /* this class */) {
    __android_log_print(ANDROID_LOG_WARN, LOG_TAG, "Module not installed.");
    return nullptr;
}
