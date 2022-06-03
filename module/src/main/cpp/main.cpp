#include "stdinclude.hpp"
#include "hook.h"
#include "zygisk.hpp"
#include <dlfcn.h>
#include "game.h"

using zygisk::Api;
using zygisk::AppSpecializeArgs;
using zygisk::ServerSpecializeArgs;

using namespace std;

class Module : public zygisk::ModuleBase {
public:
    void onLoad(Api *api, JNIEnv *env) override {
        env_ = env;
    }

    void preAppSpecialize(AppSpecializeArgs *args) override {
        if (!args || !args->nice_name) {
            LOGE("Skip unknown process");
            return;
        }
        auto pkgNm = env_->GetStringUTFChars(args->nice_name, nullptr);
        if (pkgNm == GamePackageNameS || pkgNm == GamePackageNameKorS)
            enable_hack = isGame(env_, args->app_data_dir);
    }

    void postAppSpecialize(const AppSpecializeArgs *) override {
        if (enable_hack) {
            int ret;
            pthread_t ntid;
            if ((ret = pthread_create(&ntid, nullptr, reinterpret_cast<void *(*)(void *)>(hack_thread), nullptr))) {
                LOGE("can't create thread: %s\n", strerror(ret));
            }
        }
    }

private:
    JNIEnv *env_{};
};

REGISTER_ZYGISK_MODULE(Module)

[[gnu::visibility("default")]] [[gnu::used]]
static void hook() __attribute__((constructor));

void hook() {
    if (IsRunningOnNativeBridge()) {
        int ret;
        pthread_t ntid;
        if ((ret = pthread_create(&ntid, nullptr,
                                  reinterpret_cast<void *(*)(void *)>(hack_thread), nullptr))) {
            LOGE("can't create thread: %s\n", strerror(ret));
        }
    }
}
