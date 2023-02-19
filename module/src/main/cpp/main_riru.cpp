#include <riru.h>
#include <config.hpp>

#include <sys/mman.h>

#include "stdinclude.hpp"
#include "hook.h"

#include "lsplant.hpp"

#include "elf_util.h"

#include "zygoteloader/serializer.h"
#include "zygoteloader/dex.hpp"
#include "zygoteloader/main.h"

#define _uintval(p)               reinterpret_cast<uintptr_t>(p)
#define _ptr(p)                   (reinterpret_cast<void *>(p))
#define _align_up(x, n)           (((x) + ((n) - 1)) & ~((n) - 1))
#define _align_down(x, n)         ((x) & -(n))
#define _page_size                4096
#define _page_align(n)            _align_up(static_cast<uintptr_t>(n), _page_size)
#define _ptr_align(x)             _ptr(_align_down(reinterpret_cast<uintptr_t>(x), _page_size))
#define _make_rwx(p, n)           ::mprotect(_ptr_align(p), \
                                              _page_align(_uintval(p) + (n)) != _page_align(_uintval(p)) ? _page_align(n) + _page_size : _page_align(n), \
                                              PROT_READ | PROT_WRITE | PROT_EXEC)

string moduleApi = "riru";

void *InlineHooker(void *target, void *hooker) {
    _make_rwx(target, _page_size);
    void *origin_call;
    if (DobbyHook(target, hooker, &origin_call) == RS_SUCCESS) {
        return origin_call;
    } else {
        return nullptr;
    }
}

bool InlineUnhooker(void *func) {
    return DobbyDestroy(func) == RT_SUCCESS;
}

static struct Resource *classesDex;

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
    /* if (!enable_hack) {
        enable_settings_hack = isSettings(pkgNm);
    } */
    env->ReleaseStringUTFChars(*niceName, pkgNm);
}

static void specializeAppProcessPost(
        JNIEnv *env, jclass clazz) {
    // Called "after" com_android_internal_os_Zygote_nativeSpecializeAppProcess in frameworks/base/core/jni/com_android_internal_os_Zygote.cpp
    if (enable_hack/*  || enable_settings_hack */) {
        /*if (enable_hack && Game::currentGameRegion == Game::Region::KOR) {
            SandHook::ElfImg art("libart.so");
            lsplant::InitInfo initInfo{
                    .inline_hooker = InlineHooker,
                    .inline_unhooker = InlineUnhooker,
                    .art_symbol_resolver = [&art](std::string_view symbol) -> void * {
                        return art.getSymbAddress(symbol);
                    },
                    .art_symbol_prefix_resolver = [&art](auto symbol) {
                        return art.getSymbPrefixFirstOffset(symbol);
                    },
            };
            if (lsplant::Init(env, initInfo) && classesDex != nullptr) {
                dex_load_and_invoke(
                        env,
                        classesDex->base, classesDex->length
                );
            }
        }*/
        int ret;
        pthread_t ntid;
        if ((ret = pthread_create(&ntid, nullptr,
                                  reinterpret_cast<void *(*)(
                                          void *)>(/* enable_settings_hack ? hack_settings_thread :  */hack_thread),
                                  nullptr))) {
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


    char path[PATH_MAX] = {0};
    sprintf(path, "%s/%s", riru_magisk_module_path, "classes.dex");
    classesDex = resource_map_file(path);

    return &module;
}
}
