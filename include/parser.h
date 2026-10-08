#ifndef PARSER_H
#define PARSER_H

typedef struct {
    int A;
    int a;
    int c;
    int d;
    int F;
    int f;
    int h;
    int i;
    int k;
    int l;
    int n;
    int q;
    int R;
    int r;
    int S;
    int s;
    int t;
    int u;
    int w;
} Options;

void parse_options(
    int argc,
    char *argv[],
    Options *opt
);

#endif
