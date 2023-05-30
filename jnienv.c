#include <dlfcn.h>
#include <stdbool.h>
#include "jnienv.h"

#define LOG_TAG "jnienv"
#define ANDROID_RUNTIME_DSO "libandroid_runtime.so"

typedef jint (*JNI_CreateJavaVM_t)(JavaVM **p_vm, JNIEnv **p_env, void *vm_args);

int init_jni_env(JniCtx *ctx, char **jvm_options, uint8_t jvm_nb_options)
{
    JNI_CreateJavaVM_t JNI_CreateJVM;
    JniInvocationImpl *(*JniInvocationCreate)();
    bool (*JniInvocationInit)(JniInvocationImpl *, const char *);
    jint (*registerFrameworkNatives)(JNIEnv *);
    void *runtime_dso;

    LOGV("[+] Initialize Java environment");

    if ((runtime_dso = dlopen(ANDROID_RUNTIME_DSO, RTLD_NOW)) == NULL)
    {
        LOGE("[!] %s\n", dlerror());
        return JNI_ERR;
    }

    if ((JniInvocationCreate = dlsym(runtime_dso, "JniInvocationCreate")) == NULL)
    {
        LOGE("[!] %s\n", dlerror());
        return JNI_ERR;
    }

    if ((JniInvocationInit = dlsym(runtime_dso, "JniInvocationInit")) == NULL)
    {
        LOGE("[!] %s\n", dlerror());
        return JNI_ERR;
    }

    if ((JNI_CreateJVM = (JNI_CreateJavaVM_t)dlsym(runtime_dso, "JNI_CreateJavaVM")) == NULL)
    {
        LOGE("[!] %s\n", dlerror());
        return JNI_ERR;
    }

    if ((registerFrameworkNatives = dlsym(runtime_dso, "registerFrameworkNatives")) == NULL)
    {
        LOGE("[!] %s\n", dlerror());
        return JNI_ERR;
    }

    ctx->invocation = JniInvocationCreate();
    JniInvocationInit(ctx->invocation, ANDROID_RUNTIME_DSO);

    JavaVMOption options[jvm_nb_options];

    for (int i = 0; i < jvm_nb_options; ++i)
        options[i].optionString = jvm_options[i];

    JavaVMInitArgs args;
    args.version = JNI_VERSION_1_6;
    args.nOptions = jvm_nb_options;
    args.options = options;
    args.ignoreUnrecognized = JNI_TRUE;

    jint status = JNI_CreateJVM(&ctx->vm, &ctx->env, &args);
    if (status == JNI_ERR)
        return JNI_ERR;

    LOGD("[d] vm: %p, env: %p\n", ctx->vm, ctx->env);

    status = registerFrameworkNatives(ctx->env);
    if (status == JNI_ERR)
        return JNI_ERR;

    return JNI_OK;
}

int deinit_jni_env(JniCtx *ctx)
{
    void (*JniInvocationDestroy)(JniInvocationImpl *);
    void *runtime_dso;

    LOGV("[+] Deinit JNI environment");

    if (ctx == NULL || ctx->vm == NULL)
        return JNI_ERR;

    if ((runtime_dso = dlopen(ANDROID_RUNTIME_DSO, RTLD_NOW)) == NULL)
    {
        LOGE("[!] %s\n", dlerror());
        return JNI_ERR;
    }

    if ((JniInvocationDestroy = dlsym(runtime_dso, "JniInvocationDestroy")) == NULL)
    {
        LOGE("[!] %s\n", dlerror());
        return JNI_ERR;
    }

    (*ctx->vm)->DetachCurrentThread(ctx->vm);
    (*ctx->vm)->DestroyJavaVM(ctx->vm);
    JniInvocationDestroy(ctx->invocation);

    return JNI_OK;
}
