#include <android/log.h>
#include <jni.h>

#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, __VA_ARGS__)

typedef struct JniInvocation
{
    // Name of library providing JNI_ method implementations.
    const char *jni_provider_library_name;

    // Opaque pointer to shared library from dlopen / LoadLibrary.
    void *jni_provider_library;

    // Function pointers to methods in JNI provider.
    jint (*JNI_GetDefaultJavaVMInitArgs)(void *);
    jint (*JNI_CreateJavaVM)(JavaVM **, JNIEnv **, void *);
    jint (*JNI_GetCreatedJavaVMs)(JavaVM **, jsize, jsize *);
} JniInvocationImpl;

typedef struct JniContext
{
    JavaVM *vm;
    JNIEnv *env;
    JniInvocationImpl *invocation;

} JniCtx;

int init_jni_env(JniCtx *, char **, uint8_t);
int deinit_jni_env(JniCtx *);
