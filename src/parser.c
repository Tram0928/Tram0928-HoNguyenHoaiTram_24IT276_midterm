#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "parser.h"

void parse_options(
    int argc,
    char *argv[],
    Options *opt
)
{
    int c;

    opt->A = 0;
    opt->a = 0;
    opt->c = 0;
    opt->d = 0;
    opt->F = 0;
    opt->f = 0;
    opt->h = 0;
    opt->i = 0;
    opt->k = 0;
    opt->l = 0;
    opt->n = 0;
    opt->q = 0;
    opt->R = 0;
    opt->r = 0;
    opt->S = 0;
    opt->s = 0;
    opt->t = 0;
    opt->u = 0;
    opt->w = 0;

    /*
     * NetBSD ls automatically enables -A for the super-user.
     * An explicit -a can override this behavior.
     */
    if (getuid() == 0) {
        opt->A = 1;
    }

    optind = 1;
    opterr = 0;

    while ((c = getopt(
                argc,
                argv,
                "AacdFfhiklnqRrSstuw"
            )) != -1) {

        switch (c) {
        case 'A':
            opt->A = 1;
            opt->a = 0;
            break;

        case 'a':
            opt->a = 1;
            opt->A = 0;
            break;

        case 'c':
            opt->c = 1;
            opt->u = 0;
            break;

        case 'd':
            /*
             * -d disables recursion and is itself a last-option-wins
             * option relative to -R.
             */
            opt->d = 1;
            opt->R = 0;
            break;

        case 'F':
            opt->F = 1;
            break;

        case 'f':
            /*
             * -f means unsorted directory output and implies -a.
             */
            opt->f = 1;
            opt->a = 1;
            opt->A = 0;
            break;

        case 'h':
            /*
             * -h overrides -k when it appears later.
             */
            opt->h = 1;
            opt->k = 0;
            break;

        case 'i':
            opt->i = 1;
            break;

        case 'k':
            /*
             * -k overrides -h when it appears later.
             */
            opt->k = 1;
            opt->h = 0;
            break;

        case 'l':
            /*
             * -l and -n determine the long-format owner/group mode.
             * The last one wins.
             */
            opt->l = 1;
            opt->n = 0;
            break;

        case 'n':
            opt->n = 1;
            opt->l = 1;
            break;

        case 'q':
            /*
             * -q and -w are last-option-wins.
             */
            opt->q = 1;
            opt->w = 0;
            break;

        case 'R':
            opt->R = 1;
            opt->d = 0;
            break;

        case 'r':
            opt->r = 1;
            break;

        case 'S':
            /*
             * -S and -t are mutually exclusive.
             */
            opt->S = 1;
            opt->t = 0;
            break;

        case 's':
            opt->s = 1;
            break;

        case 't':
            opt->t = 1;
            opt->S = 0;
            break;

        case 'u':
            opt->u = 1;
            opt->c = 0;
            break;

        case 'w':
            /*
             * Raw printing overrides -q.
             */
            opt->w = 1;
            opt->q = 0;
            break;

        case '?':
            if (optopt != 0) {
                fprintf(
                    stderr,
                    "myls: unknown option -- %c\n",
                    optopt
                );
            } else {
                fprintf(
                    stderr,
                    "myls: invalid option\n"
                );
            }

            exit(1);

        default:
            fprintf(
                stderr,
                "myls: invalid option\n"
            );

            exit(1);
        }
    }
}
