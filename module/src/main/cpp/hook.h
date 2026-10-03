#pragma once

#include <jni.h>
#include "zygoteloader/zygoteloader.h"

struct HookArgs {
    JNIEnv *env;
    Resource *classesDex;
};

void hack_thread(const HookArgs *args);

extern "C" void
onLayoutChange_native(JNIEnv *env, jclass clazz, jobject activity, jobject view, jint left,
                      jint top, jint right, jint bottom, jint oldLeft, jint oldTop, jint oldRight,
                      jint oldBottom);
extern "C" void handleSetPlayWhenReady_native(JNIEnv *env, jclass clazz, jboolean playWhenReady);
extern "C" void handleSeek_native(JNIEnv *env, jclass clazz, jint mediaItemIndex, jlong positionMs,
                                  jint seekCommand);
extern "C" void handleSetRepeatMode_native(JNIEnv *env, jclass clazz, jboolean isRepeatEnabled);

#define HOOK_DEF(ret, func, ...) \
  void* addr_##func; \
  ret (*orig_##func)(__VA_ARGS__); \
  ret new_##func(__VA_ARGS__)
