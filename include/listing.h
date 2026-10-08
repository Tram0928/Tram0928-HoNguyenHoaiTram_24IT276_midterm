#ifndef LISTING_H
#define LISTING_H

#include <sys/stat.h>

#include "parser.h"

#define MYLS_NAME_SIZE 1024
#define MYLS_PATH_SIZE 4096

typedef struct {
    char name[MYLS_NAME_SIZE];
    char path[MYLS_PATH_SIZE];
    struct stat st;
} FileEntry;

int read_directory(
    const char *path,
    FileEntry **entries,
    int *count,
    Options *opt
);

int read_file(
    const char *path,
    FileEntry *entry
);

#endif
