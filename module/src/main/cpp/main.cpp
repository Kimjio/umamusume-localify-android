#include <fcntl.h>

#include <sys/mman.h>

#include "stdinclude.hpp"
#include "hook.h"
#include "zygisk.hpp"

#include "lsplant.hpp"

#include "elf_util.h"

#include "zygoteloader/serializer.h"
#include "zygoteloader/dex.hpp"
#include "zygoteloader/main.h"

#define _u_int_val(p)               reinterpret_cast<uintptr_t>(p)
#define _ptr(p)                   (reinterpret_cast<void *>(p))
#define _align_up(x, n)           (((x) + ((n) - 1)) & ~((n) - 1))
#define _align_down(x, n)         ((x) & -(n))
#define _page_size                4096
#define _page_align(n)            _align_up(static_cast<uintptr_t>(n), _page_size)
#define _ptr_align(x)             _ptr(_align_down(reinterpret_cast<uintptr_t>(x), _page_size))
#define make_rwx(p, n)           ::mprotect(_ptr_align(p), \
                                              _page_align(_u_int_val(p) + (n)) != _page_align(_u_int_val(p)) ? _page_align(n) + _page_size : _page_align(n), \
                                              PROT_READ | PROT_WRITE | PROT_EXEC)

enum FileCommand : int {
    INITIALIZE, IS_INITIALIZED, GET_RESOURCES
};

using zygisk::Api;
using zygisk::AppSpecializeArgs;
using zygisk::ServerSpecializeArgs;

void *InlineHooker(void *target, void *hooker) {
    make_rwx(target, _page_size);
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

void handleFileRequest(int client) {
    static pthread_mutex_t initializeLock = PTHREAD_MUTEX_INITIALIZER;
    static int classesDex = -1;
    static int moduleDirectory = -1;

    int command = -1;
    serializer_read_int(client, &command);

    switch (static_cast<FileCommand>(command)) {
        case INITIALIZE: {
            pthread_mutex_lock(&initializeLock);
            if (moduleDirectory == -1) {
                serializer_read_file_descriptor(client, &moduleDirectory);
                classesDex = openat(moduleDirectory, "classes.dex", O_RDONLY);
            }
            serializer_write_int(client, 1);
            pthread_mutex_unlock(&initializeLock);
            break;
        }
        case IS_INITIALIZED: {
            pthread_mutex_lock(&initializeLock);
            serializer_write_int(client, moduleDirectory != -1 ? 1 : 0);
            pthread_mutex_unlock(&initializeLock);
            break;
        }
        case GET_RESOURCES: {
            serializer_write_file_descriptor(client, classesDex);
            break;
        }
    }
}

class Module : public zygisk::ModuleBase {
public:
    void onLoad(Api *pApi, JNIEnv *pEnv) override {
        api = pApi;
        env = pEnv;

        if (!isInitialized()) {
            initialize();
        }
    }

    void preAppSpecialize(AppSpecializeArgs *args) override {
        if (!args || !args->nice_name) {
            LOGW("Skip unknown process");
            return;
        }
        auto pkgNm = env->GetStringUTFChars(args->nice_name, nullptr);
        enable_hack = isGame(pkgNm);
        /* if (!enable_hack) {
            enable_settings_hack = isSettings(pkgNm);
        } */
        if (enable_hack && Game::currentGameRegion == Game::Region::KOR) {
            fetchResources();
        }
        env->ReleaseStringUTFChars(args->nice_name, pkgNm);
    }

    void postAppSpecialize(const AppSpecializeArgs *) override {
        if (enable_hack /* || enable_settings_hack */) {
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
            pthread_t t;
            ret = pthread_create(&t, nullptr,
                                 reinterpret_cast<void *(*)(void *)>(/* enable_settings_hack
                                                                          ? hack_settings_thread
                                                                          :  */hack_thread),
                                 classesDex);
            if (ret != 0) {
                LOGE("can't create thread: %s\n", strerror(ret));
            }
        }
    }

    void fetchResources() {
        const int remote = api->connectCompanion();
        serializer_write_int(remote, GET_RESOURCES);
        int classesDexFd = -1;
        serializer_read_file_descriptor(remote, &classesDexFd);
        classesDex = resource_map_fd(classesDexFd);
        close(remote);
        close(classesDexFd);
    }

private:
    JNIEnv *env{};
    Api *api{};
    Resource *classesDex{};

    bool isInitialized() {
        const int remote = api->connectCompanion();

        serializer_write_int(remote, IS_INITIALIZED);

        int initialized = -1;
        serializer_read_int(remote, &initialized);

        close(remote);

        return initialized != 0;
    }

    void initialize() {
        const int remote = api->connectCompanion();

        const int moduleDir = api->getModuleDir();

        serializer_write_int(remote, INITIALIZE);
        serializer_write_file_descriptor(remote, moduleDir);
    }
};

REGISTER_ZYGISK_MODULE(Module)

REGISTER_ZYGISK_COMPANION(handleFileRequest)

extern "C" {
[[gnu::visibility("default")]] [[gnu::used]]
void hook() __attribute__((constructor));

void hook() {
    if (IsRunningOnNativeBridge()) {
        Game::currentGameRegion = Game::CheckPackageNameByDataPath();
        if (Game::currentGameRegion == Game::Region::UNKNOWN) {
            LOGW("Region UNKNOWN...");
            return;
        }
        int ret;
        pthread_t t;
        ret = pthread_create(&t, nullptr,
                             reinterpret_cast<void *(*)(void *)>(hack_thread), nullptr);
        if (ret != 0) {
            LOGE("can't create thread: %s\n", strerror(ret));
        }
    }
}
}
