#ifndef UMAMUSUMELOCALIFYANDROID_HOOK_H
#define UMAMUSUMELOCALIFYANDROID_HOOK_H

#include "log.h"

static bool enable_hack;
// static bool enable_settings_hack;
static void *il2cpp_handle = nullptr;
// static void *app_handle = nullptr;

bool isGame(const char *appDataDir);
// bool isSettings(const char *appDataDir);

void hack_thread(void *arg);
// void hack_settings_thread(void *arg);

#define HOOK_DEF(ret, func, ...) \
  void* addr_##func; \
  ret (*orig_##func)(__VA_ARGS__); \
  ret new_##func(__VA_ARGS__)

#endif //UMAMUSUMELOCALIFYANDROID_HOOK_H
