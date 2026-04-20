#include "pes.h"
#include "index.h"
#include "tree.h"
#include "commit.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

// ---------- INIT ----------
int cmd_init() {
    mkdir(".pes", 0755);
    mkdir(".pes/objects", 0755);

    printf("Initialized empty PES repository in .pes/\n");
    return 0;
}

// ---------- ADD ----------
int cmd_add(int argc, char *argv[]) {
    Index idx;
    index_load(&idx);

    for (int i = 0; i < argc; i++) {
        if (index_add(&idx, argv[i]) != 0) {
            printf("error: failed to add '%s'\n", argv[i]);
        }
    }

    // 🔥 IMPORTANT FIX
    index_save(&idx);

    return 0;
}

// ---------- STATUS ----------
int cmd_status() {
    Index idx;
    index_load(&idx);

    index_status(&idx);
    return 0;
}

// ---------- COMMIT ----------
int cmd_commit(const char *message) {
    Index idx;
    index_load(&idx);

    if (idx.count == 0) {
        printf("nothing to commit\n");
        return 0;
    }

    ObjectID commit_id;

    // ✅ correct signature
    if (commit_create(message, &commit_id) != 0) {
        printf("error: failed to create commit\n");
        return -1;
    }

    char hex[65];
    for (int i = 0; i < 32; i++) {
        sprintf(hex + i * 2, "%02x", commit_id.hash[i]);
    }
    hex[64] = '\0';

    printf("Committed as %s\n", hex);
    return 0;
}

// ---------- LOG CALLBACK ----------
static void print_commit(const ObjectID *id, const Commit *commit, void *ctx) {
    (void)ctx; // avoid unused warning

    char hex[65];
    for (int i = 0; i < 32; i++) {
        sprintf(hex + i * 2, "%02x", id->hash[i]);
    }
    hex[64] = '\0';

    printf("commit %s\n    %s\n\n", hex, commit->message);
}

// ---------- LOG ----------
int cmd_log() {
    commit_walk(print_commit, NULL);
    return 0;
}

// ---------- MAIN ----------
int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("usage: pes <command>\n");
        return 1;
    }

    if (strcmp(argv[1], "init") == 0) {
        return cmd_init();
    }

    else if (strcmp(argv[1], "add") == 0) {
        if (argc < 3) {
            printf("usage: pes add <files>\n");
            return 1;
        }
        return cmd_add(argc - 2, &argv[2]);
    }

    else if (strcmp(argv[1], "status") == 0) {
        return cmd_status();
    }

    else if (strcmp(argv[1], "commit") == 0) {
        if (argc < 3) {
            printf("usage: pes commit \"message\"\n");
            return 1;
        }
        return cmd_commit(argv[2]);
    }

    else if (strcmp(argv[1], "log") == 0) {
        return cmd_log();
    }

    else {
        printf("unknown command: %s\n", argv[1]);
        return 1;
    }
}
