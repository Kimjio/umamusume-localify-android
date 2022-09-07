#include "dex.hpp"

#include "../log.h"

#include <string>

#include "lsplant.hpp"


#define find_class(var_name, name) jclass var_name = env->FindClass(name);
#define find_static_method(var_name, clazz, name, signature) jmethodID var_name = env->GetStaticMethodID(clazz, name, signature);
#define find_method(var_name, clazz, name, signature) jmethodID var_name = env->GetMethodID(clazz, name, signature);
#define new_string(text) env->NewStringUTF(text)

#ifdef __cplusplus
extern "C" {
#endif

jobject doHook(JNIEnv *env, jobject thiz, jobject target, jobject callback) {
    return lsplant::Hook(env, target, thiz, callback);
}

jboolean doUnhook(JNIEnv *env, jobject, jobject target) {
    return lsplant::UnHook(env, target);
}

std::string error_msg;

void _append_exception_trace_messages(
        JNIEnv &a_jni_env,
        std::string &a_error_msg,
        jthrowable a_exception,
        jmethodID a_mid_throwable_getCause,
        jmethodID a_mid_throwable_getStackTrace,
        jmethodID a_mid_throwable_toString,
        jmethodID a_mid_frame_toString) {
    // Get the array of StackTraceElements.
    auto frames =
            (jobjectArray) a_jni_env.CallObjectMethod(
                    a_exception,
                    a_mid_throwable_getStackTrace);
    jsize frames_length = a_jni_env.GetArrayLength(frames);

    // Add Throwable.toString() before descending
    // stack trace messages.
    if (frames) {
        auto msg_obj =
                (jstring) a_jni_env.CallObjectMethod(a_exception,
                                                     a_mid_throwable_toString);
        const char *msg_str = a_jni_env.GetStringUTFChars(msg_obj, nullptr);

        // If this is not the top-of-the-trace then
        // this is a cause.
        if (!a_error_msg.empty()) {
            a_error_msg += "\nCaused by: ";
            a_error_msg += msg_str;
        } else {
            a_error_msg = msg_str;
        }

        a_jni_env.ReleaseStringUTFChars(msg_obj, msg_str);
        a_jni_env.DeleteLocalRef(msg_obj);
    }

    // Append stack trace messages if there are any.
    if (frames_length > 0) {
        jsize i;
        for (i = 0; i < frames_length; i++) {
            // Get the string returned from the 'toString()'
            // method of the next frame and append it to
            // the error message.
            jobject frame = a_jni_env.GetObjectArrayElement(frames, i);
            auto msg_obj =
                    (jstring) a_jni_env.CallObjectMethod(frame,
                                                         a_mid_frame_toString);

            const char *msg_str = a_jni_env.GetStringUTFChars(msg_obj, nullptr);

            a_error_msg += "\n    ";
            a_error_msg += msg_str;

            a_jni_env.ReleaseStringUTFChars(msg_obj, msg_str);
            a_jni_env.DeleteLocalRef(msg_obj);
            a_jni_env.DeleteLocalRef(frame);
        }
    }

    // If 'a_exception' has a cause then append the
    // stack trace messages from the cause.
    if (frames) {
        auto cause =
                (jthrowable) a_jni_env.CallObjectMethod(
                        a_exception,
                        a_mid_throwable_getCause);
        if (cause) {
            _append_exception_trace_messages(a_jni_env,
                                             a_error_msg,
                                             cause,
                                             a_mid_throwable_getCause,
                                             a_mid_throwable_getStackTrace,
                                             a_mid_throwable_toString,
                                             a_mid_frame_toString);
            LOGE("ERROR: %s", error_msg.data());
        }
    }
}

void dex_load_and_invoke(
        JNIEnv *env,
        const void *dex_block, uint32_t dex_length
) {
    find_class(c_class_loader, "java/lang/ClassLoader");
    find_static_method(m_get_system_class_loader, c_class_loader, "getSystemClassLoader",
                       "()Ljava/lang/ClassLoader;");
    jobject o_system_class_loader = env->CallStaticObjectMethod(
            c_class_loader,
            m_get_system_class_loader
    );

    find_class(c_dex_class_loader, "dalvik/system/InMemoryDexClassLoader");
    find_method(m_dex_class_loader, c_dex_class_loader, "<init>",
                "(Ljava/nio/ByteBuffer;Ljava/lang/ClassLoader;)V");
    jobject o_dex_class_loader = env->NewObject(
            c_dex_class_loader,
            m_dex_class_loader,
            env->NewDirectByteBuffer((void *) dex_block, dex_length),
            o_system_class_loader
    );

    find_method(m_load_class, c_class_loader, "loadClass",
                "(Ljava/lang/String;)Ljava/lang/Class;");
    auto c_loader = (jclass) env->CallObjectMethod(
            o_dex_class_loader,
            m_load_class,
            new_string("com.kimjio.umamusumelocalify.Hooker")
    );

    JNINativeMethod methods[] = {
            {.name = "doHook",
                    .signature = "(Ljava/lang/reflect/Member;Ljava/lang/reflect/Method;)Ljava/lang/reflect/Method;",
                    .fnPtr = (void *) doHook},
            {.name = "doUnhook",
                    .signature = "(Ljava/lang/reflect/Member;)Z",
                    .fnPtr = (void *) doUnhook}
    };

    env->RegisterNatives(c_loader, methods, 2);

    find_static_method(m_load, c_loader, "load", "()V");

    env->CallStaticVoidMethod(
            c_loader,
            m_load
    );

    if (env->ExceptionCheck()) {
        LOGW("Dex exception!");

        jthrowable exception = env->ExceptionOccurred();
        env->ExceptionClear();

        jclass throwable_class = env->FindClass("java/lang/Throwable");
        jmethodID mid_throwable_getCause =
                env->GetMethodID(throwable_class,
                                 "getCause",
                                 "()Ljava/lang/Throwable;");
        jmethodID mid_throwable_getStackTrace =
                env->GetMethodID(throwable_class,
                                 "getStackTrace",
                                 "()[Ljava/lang/StackTraceElement;");
        jmethodID mid_throwable_toString =
                env->GetMethodID(throwable_class,
                                 "toString",
                                 "()Ljava/lang/String;");

        jclass frame_class = env->FindClass("java/lang/StackTraceElement");
        jmethodID mid_frame_toString =
                env->GetMethodID(frame_class,
                                 "toString",
                                 "()Ljava/lang/String;");
        _append_exception_trace_messages(*env,
                                         error_msg,
                                         exception,
                                         mid_throwable_getCause,
                                         mid_throwable_getStackTrace,
                                         mid_throwable_toString,
                                         mid_frame_toString);
    }
}

#ifdef __cplusplus
}
#endif
