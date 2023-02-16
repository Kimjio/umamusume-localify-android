#include <jni.h>
#include <string>
#include <sys/system_properties.h>

extern "C" JNIEXPORT jstring JNICALL
Java_com_kimjio_umamusumelocalify_settings_ModuleUtils_getModuleVersion(
        JNIEnv *env,
        jclass /* this class */) {
    char nativeBridge[PROP_VALUE_MAX];
    __system_property_get("ro.com.kimjio.umamusumelocalify.version", nativeBridge);
    return env->NewStringUTF(nativeBridge);
}
