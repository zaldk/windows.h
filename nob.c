#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#ifndef _WIN32
#define NOB_EXPERIMENTAL_DELETE_OLD
#endif
#include "nob.h"

int main(int argc, char **argv) {
    NOB_GO_REBUILD_URSELF(argc, argv);
    Cmd cmd = {0};

#ifndef _WIN32
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
    cmd_append(&cmd, "-Wall", "-Wextra", "-ggdb");
    cmd_append(&cmd, "-Wno-unused-parameter", "-Wno-unused-variable");
    cmd_append(&cmd, "-lX11");
    cmd_append(&cmd, "-o", ".build/exe", "src/main.c");
    if (!cmd_run(&cmd)) return 1;
#else
    cmd_append(&cmd, "cl", "/Fe:.build/windows", "/nologo", "/W3", "src/main.c", "/link", "User32.lib", "Gdi32.lib", "/subsystem:windows");
    if (!cmd_run(&cmd)) return 1;

    cmd_append(&cmd, "./.build/windows.exe");
    if (!cmd_run(&cmd)) return 1;
#endif

    return 0;
}
