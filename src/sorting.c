#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "sorting.h"

static Options *current_options;

static time_t entry_time(
    const FileEntry *entry
)
{
    if (current_options->c) {
        return entry->st.st_ctime;
    }

    if (current_options->u) {
        return entry->st.st_atime;
    }

    return entry->st.st_mtime;
}

static int compare_names(
    const FileEntry *a,
    const FileEntry *b
)
{
    return strcmp(
        a->name,
        b->name
    );
}

/*
 * Compare directory contents according to -S, -t or normal
 * lexicographical ordering.
 */
static int compare_entries(
    const void *left,
    const void *right
)
{
    const FileEntry *a;
    const FileEntry *b;
    int result;

    a = (const FileEntry *)left;
    b = (const FileEntry *)right;

    if (current_options->S) {
        if (a->st.st_size < b->st.st_size) {
            result = 1;
        } else if (a->st.st_size > b->st.st_size) {
            result = -1;
        } else {
            result = compare_names(a, b);
        }
    } else if (current_options->t) {
        time_t ta;
        time_t tb;

        ta = entry_time(a);
        tb = entry_time(b);

        if (ta < tb) {
            result = 1;
        } else if (ta > tb) {
            result = -1;
        } else {
            result = compare_names(a, b);
        }
    } else {
        result = compare_names(a, b);
    }

    if (current_options->r) {
        result = -result;
    }

    return result;
}

/*
 * Operand ordering is different from directory-content ordering.
 *
 * The manual requires non-directory and directory operands to be
 * sorted separately and lexicographically. Therefore -S and -t
 * do not affect this comparison.
 */
static int compare_operands(
    const void *left,
    const void *right
)
{
    const FileEntry *a;
    const FileEntry *b;

    a = (const FileEntry *)left;
    b = (const FileEntry *)right;

    return strcmp(
        a->path,
        b->path
    );
}

void sort_entries(
    FileEntry *entries,
    int count,
    Options *opt
)
{
    if (entries == NULL || count <= 1) {
        return;
    }

    /*
     * -f explicitly disables sorting.
     */
    if (opt->f) {
        return;
    }

    current_options = opt;

    qsort(
        entries,
        (size_t)count,
        sizeof(FileEntry),
        compare_entries
    );
}

void sort_operands(
    FileEntry *entries,
    int count
)
{
    if (entries == NULL || count <= 1) {
        return;
    }

    qsort(
        entries,
        (size_t)count,
        sizeof(FileEntry),
        compare_operands
    );
}
