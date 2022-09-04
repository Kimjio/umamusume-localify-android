#include <jni.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif
void yahfaInit(JNIEnv *env, jint sdkVersion);
static uint32_t getFlags(char *method);
static void setFlags(char *method, uint32_t access_flags);
static void setNonCompilable(void *method);
static int replaceMethod(void *fromMethod, void *toMethod, int isBackup);
static int doBackupAndHook(void *targetMethod, void *hookMethod, void *backupMethod);
static void *getArtMethod(JNIEnv *env, jobject jmethod);
jobject findMethodNative(JNIEnv *env,
                         jclass targetClass, jstring methodName,
                         jstring methodSig);
jboolean backupAndHookNative(JNIEnv *env,
                             jobject target, jobject hook,
                             jobject backup);

#ifdef __cplusplus
}
#endif
