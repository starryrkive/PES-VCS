// index.c — Staging area implementation

#include "index.h"
#include "pes.h"
#include "tree.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

// ─── PROVIDED FUNCTIONS ─────────────────────────────────────────────

IndexEntry* index_find(Index *index, const char *path) {
    for (int i = 0; i < index->count; i++) {
        if (strcmp(index->entries[i].path, path) == 0)
            return &index->entries[i];
    }
    return NULL;
}

int index_remove(Index *index, const char *path) {
    for (int i = 0; i < index->count; i++) {
        if (strcmp(index->entries[i].path, path) == 0) {
            int remaining = index->count - i - 1;
            if (remaining > 0) {
                memmove(&index->entries[i],
                        &index->entries[i + 1],
                        remaining * sizeof(IndexEntry));
            }
            index->count--;
            return index_save(index);
        }
    }
    return -1;
}

int index_status(const Index *index) {
    printf("Staged changes:\n");

    if (index->count == 0) {
        printf("  (nothing to show)\n");
    } else {
        for (int i = 0; i < index->count; i++) {
            printf("  staged:     %s\n", index->entries[i].path);
        }
    }

    printf("\nUnstaged changes:\n");
    printf("  (nothing to show)\n");

    printf("\nUntracked files:\n");
    printf("  (nothing to show)\n");

    return 0;
}

// ─── index_load ─────────────────────────────────────────────────────

int index_load(Index *index) {
    index->count = 0;

    FILE *f = fopen(INDEX_FILE, "r");
    if (!f) return 0;  // no index yet

    char hash_hex[HASH_HEX_SIZE + 1];

    while (index->count < MAX_INDEX_ENTRIES) {
        IndexEntry *e = &index->entries[index->count];

        if (fscanf(f, "%o %64s %ld %u %255s\n",
                   &e->mode,
                   hash_hex,
                   &e->mtime_sec,
                   &e->size,
                   e->path) != 5) {
            break;
        }

        if (hex_to_hash(hash_hex, &e->hash) != 0) {
            fclose(f);
            return -1;
        }

        index->count++;
    }

    fclose(f);
    return 0;
}

// ─── index_save ─────────────────────────────────────────────────────

int index_save(const Index *index) {
    char tmp_path[256];
    snprintf(tmp_path, sizeof(tmp_path), "%s.tmp", INDEX_FILE);

    FILE *f = fopen(tmp_path, "w");
    if (!f) return -1;

    for (int i = 0; i < index->count; i++) {
        char hex[HASH_HEX_SIZE + 1];
        hash_to_hex(&index->entries[i].hash, hex);

        fprintf(f, "%o %s %ld %u %s\n",
                index->entries[i].mode,
                hex,
                index->entries[i].mtime_sec,
                index->entries[i].size,
                index->entries[i].path);
    }

    fflush(f);
    fsync(fileno(f));
    fclose(f);

    return rename(tmp_path, INDEX_FILE);
}

// ─── index_add (FIXED — NO SEGFAULT) ────────────────────────────────

int index_add(Index *index, const char *path) {
    struct stat st;
    if (stat(path, &st) != 0) return -1;

    size_t size = (size_t)st.st_size;

    FILE *f = fopen(path, "rb");
    if (!f) return -1;

    void *data = NULL;

    if (size > 0) {
        data = malloc(size);
        if (!data) {
            fclose(f);
            return -1;
        }

        if (fread(data, 1, size, f) != size) {
            free(data);
            fclose(f);
            return -1;
        }
    }

    fclose(f);

    ObjectID blob_id;
    if (object_write(OBJ_BLOB, data, size, &blob_id) != 0) {
        free(data);
        return -1;
    }

    free(data);

    IndexEntry *e = index_find(index, path);

    if (!e) {
        if (index->count >= MAX_INDEX_ENTRIES) return -1;
        e = &index->entries[index->count++];
    }

    e->mode = get_file_mode(path);
    e->hash = blob_id;
    e->mtime_sec = st.st_mtime;
    e->size = (uint32_t)size;

    strncpy(e->path, path, sizeof(e->path) - 1);
    e->path[sizeof(e->path) - 1] = '\0';

    return 0;
}
// phase3 step1
// phase3 step2
// phase3 step3
