#include "tree.h"
#include "pes.h"
#include <stdio.h>

int main() {
    ObjectID id;

    if (tree_from_index(&id) != 0) {
        printf("tree creation failed\n");
        return 1;
    }

    char hex[65];
    hash_to_hex(&id, hex);
    printf("Tree object created: %s\n", hex);

    return 0;
}
