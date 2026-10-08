#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "parser.h"
#include "listing.h"
#include "sorting.h"
#include "display.h"
#include "recursion.h"

typedef struct {
    FileEntry *items;
    int count;
    int capacity;
} EntryList;

static void init_entry_list(
    EntryList *list
)
{
    list->items = NULL;
    list->count = 0;
    list->capacity = 0;
}

static int add_entry(
    EntryList *list,
    const FileEntry *entry
)
{
    FileEntry *new_items;
    int new_capacity;

    if (list->count < list->capacity) {
        list->items[list->count] = *entry;
        list->count++;
        return 0;
    }

    if (list->capacity == 0) {
        new_capacity = 16;
    } else {
        if (list->capacity > 1073741823) {
            fprintf(
                stderr,
                "myls: too many operands\n"
            );
            return -1;
        }

        new_capacity = list->capacity * 2;
    }

    new_items = realloc(
        list->items,
        sizeof(FileEntry) * (size_t)new_capacity
    );

    if (new_items == NULL) {
        perror("myls: realloc");
        return -1;
    }

    list->items = new_items;
    list->capacity = new_capacity;

    list->items[list->count] = *entry;
    list->count++;

    return 0;
}

static void free_entry_list(
    EntryList *list
)
{
    free(list->items);

    list->items = NULL;
    list->count = 0;
    list->capacity = 0;
}

static int operand_is_directory(
    const char *path,
    Options *opt,
    struct stat *st
)
{
    struct stat local;

    if (lstat(
            path,
            &local
        ) == -1) {
        perror(path);
        return -1;
    }

    /*
     * -d treats directories as plain files and does not
     * indirect through symbolic links supplied as operands.
     */
    if (opt->d) {
        *st = local;
        return 0;
    }

    /*
     * Without -d, a symbolic link supplied as an operand
     * is followed when deciding whether it names a directory.
     */
    if (S_ISLNK(local.st_mode)) {
        if (stat(
                path,
                st
            ) == -1) {
            perror(path);
            return -1;
        }
    } else {
        *st = local;
    }

    return S_ISDIR(st->st_mode);
}

static int process_file_operands(
    EntryList *files,
    Options *opt
)
{
    int i;

    if (files->count == 0) {
        return 0;
    }

    /*
     * Operands are sorted lexicographically.
     * This is independent from directory-entry sorting.
     */
    sort_operands(
        files->items,
        files->count
    );

    for (i = 0; i < files->count; i++) {
        display_single(
            &files->items[i],
            opt
        );
    }

    return 0;
}

static int process_directory_operand(
    const char *path,
    Options *opt,
    int multiple_operands
)
{
    FileEntry *entries;
    int count;
    int result;

    /*
     * This is also a safety guard for -d.
     * A directory must be displayed as a plain file.
     */
    if (opt->d) {
        FileEntry entry;

        if (read_file(
                path,
                &entry
            ) == -1) {
            return -1;
        }

        display_single(
            &entry,
            opt
        );

        return 0;
    }

    if (opt->R) {
        if (multiple_operands) {
            printf(
                "%s:\n",
                path
            );
        }

        result = list_recursive(
            path,
            opt
        );

        return result;
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

    if (multiple_operands) {
        printf(
            "%s:\n",
            path
        );
    }

    display_entries(
        entries,
        count,
        opt
    );

    free(entries);

    return 0;
}

int main(
    int argc,
    char *argv[]
)
{
    Options opt;
    EntryList files;
    EntryList directories;
    int i;
    int errors;

    parse_options(
        argc,
        argv,
        &opt
    );

    init_entry_list(&files);
    init_entry_list(&directories);

    errors = 0;

    /*
     * With no operands, ls operates on ".".
     * With -d, "." itself must be displayed as a plain file.
     */
    if (optind >= argc) {
        if (opt.d) {
            FileEntry entry;

            if (read_file(
                    ".",
                    &entry
                ) == -1) {

                errors = 1;
            } else {
                display_single(
                    &entry,
                    &opt
                );
            }
        } else {
            if (process_directory_operand(
                    ".",
                    &opt,
                    0
                ) == -1) {

                errors = 1;
            }
        }

        free_entry_list(&files);
        free_entry_list(&directories);

        return errors;
    }

    for (i = optind; i < argc; i++) {
        FileEntry entry;
        struct stat st;
        int is_directory;

        is_directory =
            operand_is_directory(
                argv[i],
                &opt,
                &st
            );

        if (is_directory == -1) {
            errors = 1;
            continue;
        }

        if (read_file(
                argv[i],
                &entry
            ) == -1) {

            errors = 1;
            continue;
        }

        if (is_directory) {
            if (add_entry(
                    &directories,
                    &entry
                ) == -1) {

                errors = 1;
            }
        } else {
            if (add_entry(
                    &files,
                    &entry
                ) == -1) {

                errors = 1;
            }
        }
    }

    /*
     * Non-directory operands are displayed first.
     */
    process_file_operands(
        &files,
        &opt
    );

    if (files.count > 0 &&
        directories.count > 0) {
        putchar('\n');
    }

    /*
     * Directory operands are sorted lexicographically.
     */
    sort_operands(
        directories.items,
        directories.count
    );

    for (i = 0;
         i < directories.count;
         i++) {

        if (directories.count > 1 ||
            files.count > 0) {

            if (i > 0) {
                putchar('\n');
            }
        }

        if (process_directory_operand(
                directories.items[i].path,
                &opt,
                directories.count > 1 ||
                files.count > 0
            ) == -1) {

            errors = 1;
        }
    }

    free_entry_list(&files);
    free_entry_list(&directories);

    return errors ? 1 : 0;
}
