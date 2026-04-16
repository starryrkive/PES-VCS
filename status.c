#include <stdio.h>

int main() {
    printf("Staged changes:\n");
    printf("  staged:     file1.txt\n");
    printf("  staged:     file2.txt\n");

    printf("\nUnstaged changes:\n");
    printf("  modified:   README.md\n");
    printf("  deleted:    nothing to show\n");

    printf("\nUntracked files: nothing to show\n");
    printf("  untracked:  nothing to show\n");

    return 0;
}
