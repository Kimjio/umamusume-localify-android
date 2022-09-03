#include "stdinclude.hpp"
#include "hook.h"
#include "zygisk.hpp"

using zygisk::Api;
using zygisk::AppSpecializeArgs;
using zygisk::ServerSpecializeArgs;

using namespace std;

string moduleApi = "zygisk";

class Module : public zygisk::ModuleBase {
public:
    void onLoad(Api *api, JNIEnv *env) override {
        env_ = env;
    }

    void preAppSpecialize(AppSpecializeArgs *args) override {
        if (!args || !args->nice_name) {
            LOGW("Skip unknown process");
            return;
        }
        auto pkgNm = env_->GetStringUTFChars(args->nice_name, nullptr);
        enable_hack = isGame(pkgNm);
        env_->ReleaseStringUTFChars(args->nice_name, pkgNm);
    }

    void postAppSpecialize(const AppSpecializeArgs *args) override {
        if (enable_hack) {
            int ret;
            pthread_t ntid;
            if ((ret = pthread_create(&ntid, nullptr,
                                      reinterpret_cast<void *(*)(void *)>(hack_thread), nullptr))) {
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
        Game::currentGameRegion = Game::CheckPackageNameByDataPath();
        if (Game::currentGameRegion == Game::Region::UNKNOWN) {
            LOGW("Region UNKNOWN...");
            return;
        }
        int ret;
        pthread_t ntid;
        if ((ret = pthread_create(&ntid, nullptr,
                                  reinterpret_cast<void *(*)(void *)>(hack_thread), nullptr))) {
            LOGE("can't create thread: %s\n", strerror(ret));
        }
    }
}
