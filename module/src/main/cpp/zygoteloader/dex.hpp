#pragma once

#include <jni.h>

#ifdef __cplusplus
extern "C" {
#endif

void dex_load_and_invoke(
        JNIEnv *env,
        const void *dex_block, uint32_t dex_length
);

#ifdef __cplusplus
};
#endif
