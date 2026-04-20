#include "commit.h"
#include "tree.h"
#include "index.h"
#include "pes.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// declare object_write
int object_write(int type, const void *data, size_t size, ObjectID *out);

// ---------- CREATE COMMIT ----------
int commit_create(const char *message, ObjectID *commit_id_out) {
    // build tree from index
    ObjectID tree_id;
    if (tree_from_index(&tree_id) != 0) {
        return -1;
    }

    // build commit content
    char buffer[1024];

    time_t now = time(NULL);

    // convert tree hash to hex
    char tree_hex[65];
    for (int i = 0; i < 32; i++) {
        sprintf(tree_hex + i * 2, "%02x", tree_id.hash[i]);
    }
    tree_hex[64] = '\0';

    int len = snprintf(buffer, sizeof(buffer),
        "tree %s\n"
        "date %ld\n\n"
        "%s\n",
        tree_hex,
        now,
        message
    );

    // write commit object
    if (object_write(OBJ_COMMIT, buffer, len, commit_id_out) != 0) {
        return -1;
    }

    return 0;
}

// ---------- WALK COMMITS ----------
int commit_walk(commit_walk_fn callback, void *ctx) {
    (void)system("find .pes/objects -type f > .pes/tmp_list");

    FILE *list = fopen(".pes/tmp_list", "r");
    if (!list) return -1;

    char path[512];

    while (fgets(path, sizeof(path), list)) {
        path[strcspn(path, "\n")] = 0;

        FILE *f = fopen(path, "rb");
        if (!f) continue;

        char buf[1024];
        size_t n = fread(buf, 1, sizeof(buf) - 1, f);
        fclose(f);

        if (n == 0) continue;
        buf[n] = '\0';

        // only process commit-like objects
        if (strncmp(buf, "tree ", 5) != 0) continue;

        // safely extract message
        char *msg = strstr(buf, "\n\n");
        if (!msg) continue;
        msg += 2;

        Commit c;
        memset(&c, 0, sizeof(c));
        strncpy(c.message, msg, sizeof(c.message) - 1);

        ObjectID id;
        memset(&id, 0, sizeof(id));

        callback(&id, &c, ctx);
    }

    fclose(list);
    return 0;
}
// Phase 4: Setup commit creation flow
// Phase 4: Implemented commit object creation
// Phase 4: Added log traversal support
