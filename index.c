#include "index.h"
#include "pes.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

// ---------- LOAD INDEX ----------
int index_load(Index *idx) {
    idx->count = 0;

    FILE *f = fopen(".pes/index", "r");
    if (!f) return 0;

    char hash_hex[65];

    while (idx->count < MAX_INDEX_ENTRIES) {
        IndexEntry *e = &idx->entries[idx->count];

        if (fscanf(f, "%o %64s %u %255s\n",
                   &e->mode,
                   hash_hex,
                   &e->size,
                   e->path) != 4) break;

        // hex → binary
        for (int i = 0; i < 32; i++) {
            sscanf(hash_hex + 2*i, "%2hhx", &e->hash.hash[i]);
        }

        idx->count++;
    }

    fclose(f);
    return 0;
}

// ---------- SAVE INDEX ----------
int index_save(const Index *idx) {
    FILE *f = fopen(".pes/index.tmp", "w");
    if (!f) return -1;

    for (int i = 0; i < idx->count; i++) {
        const IndexEntry *e = &idx->entries[i];

        char hex[65];
        for (int j = 0; j < 32; j++) {
            sprintf(hex + j*2, "%02x", e->hash.hash[j]);
        }
        hex[64] = '\0';

        fprintf(f, "%o %s %u %s\n",
                e->mode,
                hex,
                e->size,
                e->path);
    }

    fclose(f);
    rename(".pes/index.tmp", ".pes/index");

    return 0;
}

// ---------- ADD FILE ----------
int index_add(Index *idx, const char *path) {
    struct stat st;

    if (stat(path, &st) != 0) return -1;

    FILE *f = fopen(path, "rb");
    if (!f) return -1;

    char *data = malloc(st.st_size);
    if (!data) {
        fclose(f);
        return -1;
    }

    size_t n = fread(data, 1, st.st_size, f);
    if (n != (size_t)st.st_size) {
        fclose(f);
        free(data);
        return -1;
    }

    fclose(f);

    // -------- WRITE OBJECT --------
    ObjectID id;

    if (object_write(OBJ_BLOB, data, st.st_size, &id) != 0) {
        free(data);
        return -1;
    }

    free(data);

    // -------- FIND EXISTING --------
    int pos = -1;
    for (int i = 0; i < idx->count; i++) {
        if (strcmp(idx->entries[i].path, path) == 0) {
            pos = i;
            break;
        }
    }

    IndexEntry *e;
    if (pos >= 0) e = &idx->entries[pos];
    else e = &idx->entries[idx->count++];

    // -------- FILL ENTRY --------
    e->mode = 100644;
    e->size = st.st_size;
    strcpy(e->path, path);
    e->hash = id;

    return 0;
}

// ---------- STATUS ----------
int index_status(const Index *idx) {
    printf("Staged changes:\n");

    if (idx->count == 0) {
        printf("  (nothing to show)\n");
    } else {
        for (int i = 0; i < idx->count; i++) {
            printf("  staged: %s\n", idx->entries[i].path);
        }
    }

    printf("\nUnstaged changes:\n  (nothing to show)\n");
    printf("\nUntracked files:\n  (nothing to show)\n");

    return 0;
}
// Phase 3: Setup index loading logic
// Phase 3: Implemented index save functionality
