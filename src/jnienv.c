#include "jnienv.h"

#include <dlfcn.h>
#include <stdbool.h>

#define LOG_TAG "jnienv"
#define LIBART_DSO "libart.so"
#define LIBANDROID_RUNTIME_DSO "libandroid_runtime.so"

typedef jint (*JNI_CreateJavaVM_t)(JavaVM **p_vm, JNIEnv **p_env, void *vm_args);
typedef jint (*registerNatives_t)(JNIEnv *env, jclass clazz);

int init_jni_env(JniCtx *ctx, char **jvm_options, uint8_t jvm_nb_options) {
    JNI_CreateJavaVM_t JNI_CreateJavaVM;
    void *libart_dso;
    void *libandroid_runtime_dso;

    LOGI("[i] Initialize JNI environment");

    if ((libart_dso = dlopen(LIBART_DSO, RTLD_NOW)) == NULL) {
        LOGE("[X] %s", dlerror());
        return JNI_ERR;
    }

    if ((libandroid_runtime_dso = dlopen(LIBANDROID_RUNTIME_DSO, RTLD_NOW)) == NULL) {
        LOGE("[X] %s", dlerror());
        return JNI_ERR;
    }

    if ((JNI_CreateJavaVM = (JNI_CreateJavaVM_t)dlsym(libart_dso, "JNI_CreateJavaVM")) == NULL) {
        LOGE("[X] %s", dlerror());
        return JNI_ERR;
    }

    registerNatives_t registerNatives =
        (registerNatives_t)dlsym(libandroid_runtime_dso, "registerFrameworkNatives");
    if (!registerNatives) {
        LOGE("[X] %s", dlerror());
        return JNI_ERR;
    }

    JavaVMOption options[jvm_nb_options];

    for (int i = 0; i < jvm_nb_options; ++i) options[i].optionString = jvm_options[i];

    JavaVMInitArgs args;
    args.version = JNI_VERSION_1_6;
    args.nOptions = jvm_nb_options;
    args.options = options;
    args.ignoreUnrecognized = JNI_FALSE;

    int status = JNI_OK;
    if ((status = JNI_CreateJavaVM(&ctx->vm, &ctx->env, &args)) != JNI_OK) {
        LOGE("[X] Failed JNI_CreateJavaVM: %d", status);
        return status;
    } else {
        LOGI("[+] vm: %p, env: %p", ctx->vm, ctx->env);
    }

    if ((status = registerNatives(ctx->env, 0)) != JNI_OK) {
        LOGE("[X] Failed registerNatives: %d", status);
        return status;
    } else {
        LOGI("[+] registerNatives success");
    }

    return status;
}

int destroy_jni_env(JniCtx *ctx) {
    LOGI("[i] Destroy JNI environment");

    if (ctx == NULL || ctx->vm == NULL) {
        return JNI_ERR;
    }

    int status = JNI_OK;
    if ((status = (*ctx->vm)->DetachCurrentThread(ctx->vm)) != JNI_OK) {
        LOGE("[X] DetachCurrentThread failed: %d", status);
        return status;
    } else {
        LOGI("[+] DetachCurrentThread success");
    };

    if ((status = (*ctx->vm)->DestroyJavaVM(ctx->vm)) != JNI_OK) {
        LOGE("[X] DestroyJavaVM failed: %d", status);
        return status;
    } else {
        LOGI("[+] DestroyJavaVM success");
    };

    return status;
}
