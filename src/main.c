#include <stddef.h>

#include "jnienv.h"

static JniCtx ctx;

int fuzz(uint8_t *buffer, size_t size) {
    int status;
    char *options[1] = {"-Djava.library.path=/data/local/tmp"};

    if ((status = init_jni_env(&ctx, options, 1)) != JNI_OK) {
        return status;
    }

    destroy_jni_env(&ctx);

    return 0;
}

int main(int argc, char const *argv[]) { return fuzz(NULL, 0); }
