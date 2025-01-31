#include "objparser.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("usage: %s file.obj\n", argv[0]);
        return 1;
    }
    objctx ctx = {0};
    parse_obj(&ctx, argv[1]);
    objctx_print(&ctx);
    objctx_free(&ctx);
    return 0;
}