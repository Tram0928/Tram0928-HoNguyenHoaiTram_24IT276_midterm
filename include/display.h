#ifndef DISPLAY_H
#define DISPLAY_H

#include "listing.h"
#include "parser.h"

void display_entries(
    FileEntry *entries,
    int count,
    Options *opt
);

void display_single(
    FileEntry *entry,
    Options *opt
);

void display_total(
    FileEntry *entries,
    int count,
    Options *opt
);

#endif
