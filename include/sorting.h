#ifndef SORTING_H
#define SORTING_H

#include "listing.h"
#include "parser.h"

void sort_entries(
    FileEntry *entries,
    int count,
    Options *opt
);

void sort_operands(
    FileEntry *entries,
    int count
);

#endif
