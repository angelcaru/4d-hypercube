#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#include "nob.h"

bool target_windows = true;

int main(int argc, char **argv) {
    NOB_GO_REBUILD_URSELF(argc, argv);

    if (!mkdir_if_not_exists("./build/")) return 1;

    Cmd cmd = {0};
    cmd_append(&cmd, target_windows ? "x86_64-w64-mingw32-gcc" : "cc");
    cmd_append(&cmd, "-Wall", "-Wextra", "-ggdb");
    cmd_append(&cmd, "-o", "./build/main");
    cmd_append(&cmd, "./src/main.c");
    cmd_append(&cmd, "-I.", "-I./raylib/");
    cmd_append(&cmd, "-L./raylib/", target_windows ? "-lwinraylib" : "-lraylib", "-lm");
    if (target_windows) {
        cmd_append(&cmd, "-lwinmm", "-lgdi32");
    }
    if (!cmd_run_sync_and_reset(&cmd)) return 1;

    const char *program_name = nob_shift(argv, argc);

    if (argc == 0) return 0;

    const char *subcmd = nob_shift(argv, argc);
    if (strcmp(subcmd, "run") == 0) {
        cmd_append(&cmd, "./build/main");
        da_append_many(&cmd, argv, argc);
        if (!cmd_run_sync_and_reset(&cmd)) return 1;
        return 0;
    } else {
        nob_log(INFO, "Usage: %s [run] [args...]", program_name);
        nob_log(ERROR, "Unknown subcommand: %s", subcmd);
        return 1;
    }
}
