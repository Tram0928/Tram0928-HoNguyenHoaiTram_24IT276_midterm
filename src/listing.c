#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

#include "listing.h"

static int should_show(
    const char *name,
    Options *opt
)
{
    if (opt->a) {
        return 1;
    }

    if (opt->A) {
        if (strcmp(name, ".") == 0 ||
            strcmp(name, "..") == 0) {
            return 0;
        }

        return 1;
    }

    if (name[0] == '.') {
        return 0;
    }

    return 1;
}

int read_directory(
    const char *path,
    FileEntry **entries,
    int *count,
    Options *opt
)
{
    DIR *dir;
    struct dirent *de;
    FileEntry *list;
    int capacity;
    int size;

    *entries = NULL;
    *count = 0;

    dir = opendir(path);

    if (dir == NULL) {
        perror(path);
        return -1;
    }

    capacity = 32;
    size = 0;

    list = malloc(
        sizeof(FileEntry) *
        (size_t)capacity
    );

    if (list == NULL) {
        perror("myls: malloc");
        closedir(dir);
        return -1;
    }

    while ((de = readdir(dir)) != NULL) {
        FileEntry *entry;
        int result;

        if (!should_show(
                de->d_name,
                opt
            )) {
            continue;
        }

        if (size >= capacity) {
            FileEntry *new_list;

            if (capacity > 1073741823) {
                fprintf(
                    stderr,
                    "myls: too many directory entries\n"
                );

                free(list);
                closedir(dir);
                return -1;
            }

            capacity *= 2;

            new_list = realloc(
                list,
                sizeof(FileEntry) *
                (size_t)capacity
            );

            if (new_list == NULL) {
                perror("myls: realloc");
                free(list);
                closedir(dir);
                return -1;
            }

            list = new_list;
        }

        entry = &list[size];

        memset(
            entry,
            0,
            sizeof(FileEntry)
        );

        result = snprintf(
            entry->name,
            sizeof(entry->name),
            "%s",
            de->d_name
        );

        if (result < 0 ||
            (size_t)result >=
                sizeof(entry->name)) {
            fprintf(
                stderr,
                "myls: filename too long: %s\n",
                de->d_name
            );

            continue;
        }

        if (strcmp(path, "/") == 0) {
            result = snprintf(
                entry->path,
                sizeof(entry->path),
                "/%s",
                de->d_name
            );
        } else {
            result = snprintf(
                entry->path,
                sizeof(entry->path),
                "%s/%s",
                path,
                de->d_name
            );
        }

        if (result < 0 ||
            (size_t)result >=
                sizeof(entry->path)) {
            fprintf(
                stderr,
                "myls: path too long: %s/%s\n",
                path,
                de->d_name
            );

            continue;
        }

        if (lstat(
                entry->path,
                &entry->st
            ) == -1) {
            perror(entry->path);
            continue;
        }

        size++;
    }

    if (closedir(dir) == -1) {
        perror(path);
    }

    *entries = list;
    *count = size;

    return 0;
}

int read_file(
    const char *path,
    FileEntry *entry
)
{
    int result;

    memset(
        entry,
        0,
        sizeof(FileEntry)
    );

    /*
     * Keep the original operand pathname.
     *
     * Examples:
     *   src       -> src
     *   ./src     -> ./src
     *   src/      -> src/
     *   /tmp/a    -> /tmp/a
     */
    result = snprintf(
        entry->path,
        sizeof(entry->path),
        "%s",
        path
    );

    if (result < 0 ||
        (size_t)result >=
            sizeof(entry->path)) {
        fprintf(
            stderr,
            "myls: path too long: %s\n",
            path
        );

        return -1;
    }

    result = snprintf(
        entry->name,
        sizeof(entry->name),
        "%s",
        path
    );

    if (result < 0 ||
        (size_t)result >=
            sizeof(entry->name)) {
        fprintf(
            stderr,
            "myls: pathname too long: %s\n",
            path
        );

        return -1;
    }

    if (lstat(
            path,
            &entry->st
        ) == -1) {
        perror(path);
        return -1;
    }

    return 0;
}
