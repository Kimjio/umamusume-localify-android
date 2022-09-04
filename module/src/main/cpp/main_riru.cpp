#include "stdinclude.hpp"
#include "hook.h"

#include <riru.h>
#include <config.hpp>

string moduleApi = "riru";

static void specializeAppProcessPre(
        JNIEnv *env, jclass clazz, jint *uid, jint *gid, jintArray *gids, jint *runtimeFlags,
        jobjectArray *rlimits, jint *mountExternal, jstring *seInfo, jstring *niceName,
        jboolean *startChildZygote, jstring *instructionSet, jstring *appDataDir,
        jboolean *isTopApp, jobjectArray *pkgDataInfoList, jobjectArray *whitelistedDataInfoList,
        jboolean *bindMountAppDataDirs, jboolean *bindMountAppStorageDirs) {
    // Called "before" com_android_internal_os_Zygote_nativeSpecializeAppProcess in frameworks/base/core/jni/com_android_internal_os_Zygote.cpp
    // Parameters are pointers, you can change the value of them if you want
    // Some parameters are not exist is older Android versions, in this case, they are null or 0
    if (!niceName) {
        LOGE("Skip unknown process");
        return;
    }
    auto pkgNm = env->GetStringUTFChars(*niceName, nullptr);
    enable_hack = isGame(pkgNm);
    if (!enable_hack) {
        enable_settings_hack = isSettings(pkgNm);
    }
    env->ReleaseStringUTFChars(*niceName, pkgNm);
}

static void specializeAppProcessPost(
        JNIEnv *env, jclass clazz) {
    // Called "after" com_android_internal_os_Zygote_nativeSpecializeAppProcess in frameworks/base/core/jni/com_android_internal_os_Zygote.cpp
    if (enable_hack || enable_settings_hack) {
        int ret;
        pthread_t ntid;
        if ((ret = pthread_create(&ntid, nullptr,
                                  reinterpret_cast<void *(*)(void *)>(enable_settings_hack ? hack_settings_thread : hack_thread), nullptr))) {
            LOGE("can't create thread: %s\n", strerror(ret));
        }
    }

    // When unload allowed is true, the module will be unloaded (dlclose) by Riru
    // If this modules has hooks installed, DONOT set it to true, or there will be SIGSEGV
    // This value will be automatically reset to false before the "pre" function is called
    riru_set_unload_allowed(false);
}

extern "C" {
int riru_api_version;
const char *riru_magisk_module_path = nullptr;
int *riru_allow_unload = nullptr;

static auto module = RiruVersionedModuleInfo{
        .moduleApiVersion = riru::moduleApiVersion,
        .moduleInfo= RiruModuleInfo{
                .supportHide = true,
                .version = Module::moduleVersionCode,
                .versionName = Module::moduleVersionName,
                .specializeAppProcessPre = specializeAppProcessPre,
                .specializeAppProcessPost = specializeAppProcessPost
        }
};

RiruVersionedModuleInfo *init(Riru *riru) {
    auto core_max_api_version = riru->riruApiVersion;
    riru_api_version = core_max_api_version <= riru::moduleApiVersion ? core_max_api_version
                                                                      : riru::moduleApiVersion;
    module.moduleApiVersion = riru_api_version;

    riru_magisk_module_path = strdup(riru->magiskModulePath);
    if (riru_api_version >= 25) {
        riru_allow_unload = riru->allowUnload;
    }
    return &module;
}
}
