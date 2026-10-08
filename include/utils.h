#ifndef UTILS_H
#define UTILS_H

#include <stddef.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "parser.h"

void mode_to_string(
    mode_t mode,
    char *str
);

char get_file_suffix(
    const struct stat *st
);

void format_size(
    unsigned long long size,
    char *buffer,
    size_t buffer_size
);

unsigned long long get_display_blocks(
    const struct stat *st,
    Options *opt
);

unsigned long long get_display_block_bytes(
    const struct stat *st
);

const char *display_name(
    const char *name,
    Options *opt,
    char *buffer,
    size_t buffer_size
);

int join_path(
    const char *directory,
    const char *name,
    char *buffer,
    size_t buffer_size
);

#endif
