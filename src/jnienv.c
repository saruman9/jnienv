#include <dlfcn.h>
#include <stdbool.h>

#include "jnienv.h"

#define LOG_TAG "jnienv"
#define ANDROID_RUNTIME_DSO "libandroid_runtime.so"

typedef jint (*JNI_CreateJavaVM_t)(JavaVM **p_vm, JNIEnv **p_env, void *vm_args);

int init_jni_env(JniCtx *ctx, char **jvm_options, uint8_t jvm_nb_options)
{
    JNI_CreateJavaVM_t JNI_CreateJavaVM;
    jint (*registerFrameworkNatives)(JNIEnv *env);
    JniInvocationImpl *(*JniInvocationCreate)();
    bool (*JniInvocationInit)(JniInvocationImpl *instance, const char *library);
    void *libandroid_runtime_dso;

    LOGV("Initialize JNI environment");

    if ((libandroid_runtime_dso = dlopen(ANDROID_RUNTIME_DSO, RTLD_NOW)) == NULL)
    {
        LOGE("%s\n", dlerror());
        return JNI_ERR;
    }

    if ((JniInvocationCreate = dlsym(libandroid_runtime_dso, "JniInvocationCreate")) == NULL)
    {
        LOGE("%s\n", dlerror());
        return JNI_ERR;
    }

    if ((JniInvocationInit = dlsym(libandroid_runtime_dso, "JniInvocationInit")) == NULL)
    {
        LOGE("%s\n", dlerror());
        return JNI_ERR;
    }

    if ((JNI_CreateJavaVM = (JNI_CreateJavaVM_t)dlsym(libandroid_runtime_dso, "JNI_CreateJavaVM")) == NULL)
    {
        LOGE("%s\n", dlerror());
        return JNI_ERR;
    }

    if ((registerFrameworkNatives = dlsym(libandroid_runtime_dso, "registerFrameworkNatives")) == NULL)
    {
        LOGE("%s\n", dlerror());
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

    jint status = JNI_CreateJavaVM(&ctx->vm, &ctx->env, &args);
    if (status == JNI_ERR)
        return JNI_ERR;

    LOGD("vm: %p, env: %p\n", ctx->vm, ctx->env);

    status = registerFrameworkNatives(ctx->env);
    if (status == JNI_ERR)
        return JNI_ERR;

    return JNI_OK;
}

int destroy_jni_env(JniCtx *ctx)
{
    void (*JniInvocationDestroy)(JniInvocationImpl *instance);
    void *libandroid_runtime_dso;

    LOGV("Destroy JNI environment");

    if (ctx == NULL || ctx->vm == NULL)
        return JNI_ERR;

    if ((libandroid_runtime_dso = dlopen(ANDROID_RUNTIME_DSO, RTLD_NOW)) == NULL)
    {
        LOGE("%s\n", dlerror());
        return JNI_ERR;
    }

    if ((JniInvocationDestroy = dlsym(libandroid_runtime_dso, "JniInvocationDestroy")) == NULL)
    {
        LOGE("%s\n", dlerror());
        return JNI_ERR;
    }

    (*ctx->vm)->DetachCurrentThread(ctx->vm);
    (*ctx->vm)->DestroyJavaVM(ctx->vm);
    JniInvocationDestroy(ctx->invocation);

    return JNI_OK;
}
