#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "recursion.h"
#include "listing.h"
#include "sorting.h"
#include "display.h"
#include "utils.h"

static int recursive_directory(
    const char *path,
    Options *opt,
    int print_header
)
{
    FileEntry *entries;
    int count;
    int i;
    int errors;

    entries = NULL;
    count = 0;
    errors = 0;

    if (print_header) {
        printf(
            "%s:\n",
            path
        );
    }

    if (read_directory(
            path,
            &entries,
            &count,
            opt
        ) == -1) {
        return -1;
    }

    sort_entries(
        entries,
        count,
        opt
    );

    display_entries(
        entries,
        count,
        opt
    );

    for (i = 0; i < count; i++) {
        char child_path[MYLS_PATH_SIZE];

        if (!S_ISDIR(
                entries[i].st.st_mode
            )) {
            continue;
        }

        /*
         * Không đệ quy vào "." và "..".
         */
        if (strcmp(
                entries[i].name,
                "."
            ) == 0 ||
            strcmp(
                entries[i].name,
                ".."
            ) == 0) {
            continue;
        }

        if (join_path(
                path,
                entries[i].name,
                child_path,
                sizeof(child_path)
            ) == -1) {
            fprintf(
                stderr,
                "myls: path too long: %s/%s\n",
                path,
                entries[i].name
            );

            errors = 1;
            continue;
        }

        putchar('\n');

        if (recursive_directory(
                child_path,
                opt,
                1
            ) == -1) {
            errors = 1;
        }
    }

    free(entries);

    if (errors) {
        return -1;
    }

    return 0;
}

int list_recursive(
    const char *path,
    Options *opt
)
{
    return recursive_directory(
        path,
        opt,
        0
    );
}
