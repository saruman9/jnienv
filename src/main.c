#include <stddef.h>

#include "jnienv.h"

static JniCtx ctx;

int fuzz(uint8_t *buffer, size_t size) {
    int status;
    char *options[2] = {"-verbose:jni", "-Djava.library.path=/data/local/tmp"};

    if ((status = init_jni_env(&ctx, options, 2)) != JNI_OK) {
        return status;
    }

    destroy_jni_env(&ctx);

    return 0;
}

int main(int argc, char const *argv[]) {
    fuzz(NULL, 0);

    return 0;
}
