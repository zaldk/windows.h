#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#define NOB_EXPERIMENTAL_DELETE_OLD
#include "nob.h"

int main(int argc, char **argv) {
    NOB_GO_REBUILD_URSELF(argc, argv);
    Cmd cmd = {0};

    cmd_append(&cmd, "test", "-d", ".build");
    if (!cmd_run(&cmd)) {
        cmd_append(&cmd, "mkdir", ".build");
        if (!cmd_run(&cmd)) return 1;
    }

    cmd_append(&cmd, "test", "-f", ".build/.gitignore");
    if (!cmd_run(&cmd)) {
        cmd_append(&cmd, "printf", "*");
        if (!cmd_run(&cmd, .stdout_path = ".build/.gitignore")) return 1;
    }

    nob_cc(&cmd);
    cmd_append(&cmd, "-Wall", "-Wextra", "-pedantic", "-ggdb");
    cmd_append(&cmd, "-Wno-unused-parameter", "-Wno-unused-variable");
    cmd_append(&cmd, "-lX11");
    cmd_append(&cmd, "-o", ".build/exe", "src/main.c");
    if (!cmd_run(&cmd)) return 1;

    cmd_append(&cmd, "./.build/exe");
    if (!cmd_run(&cmd)) return 1;

    return 0;
}
