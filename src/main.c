#include <stddef.h>

#include "jnienv.h"

static JniCtx ctx;

int main(int argc, char const *argv[])
{
    int status;

    if ((status = init_jni_env(&ctx, NULL, 0)) != JNI_OK)
    {
        return status;
    }
    destroy_jni_env(&ctx);

    return 0;
}
