#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#ifdef __linux__
#include <sys/sysmacros.h>
#endif

#include "utils.h"

/*
 * Read BLOCKSIZE from the environment.
 *
 * Examples:
 *   BLOCKSIZE=512
 *   BLOCKSIZE=1024
 *   BLOCKSIZE=1K
 *   BLOCKSIZE=2M
 *
 * Invalid values fall back to the required 512-byte default.
 */
static unsigned long long get_block_size(void)
{
    const char *env;
    char *end;
    unsigned long long value;
    unsigned long long multiplier;

    env = getenv("BLOCKSIZE");

    if (env == NULL || *env == '\0') {
        return 512ULL;
    }

    value = strtoull(
        env,
        &end,
        10
    );

    if (end == env || value == 0) {
        return 512ULL;
    }

    multiplier = 1ULL;

    if (*end != '\0') {
        switch (tolower((unsigned char)*end)) {
        case 'k':
            multiplier = 1024ULL;
            break;

        case 'm':
            multiplier =
                1024ULL * 1024ULL;
            break;

        case 'g':
            multiplier =
                1024ULL * 1024ULL * 1024ULL;
            break;

        case 't':
            multiplier =
                1024ULL *
                1024ULL *
                1024ULL *
                1024ULL;
            break;

        default:
            return 512ULL;
        }

        end++;

        if (*end != '\0') {
            return 512ULL;
        }
    }

    if (value > ULLONG_MAX / multiplier) {
        return 512ULL;
    }

    return value * multiplier;
}

static char file_type_char(mode_t mode)
{
    if (S_ISREG(mode)) {
        return '-';
    }

    if (S_ISDIR(mode)) {
        return 'd';
    }

    if (S_ISLNK(mode)) {
        return 'l';
    }

    if (S_ISCHR(mode)) {
        return 'c';
    }

    if (S_ISBLK(mode)) {
        return 'b';
    }

#ifdef S_ISSOCK
    if (S_ISSOCK(mode)) {
        return 's';
    }
#endif

    if (S_ISFIFO(mode)) {
        return 'p';
    }

#ifdef S_ISWHT
    if (S_ISWHT(mode)) {
        return 'w';
    }
#endif

    return '?';
}

void mode_to_string(
    mode_t mode,
    char *str
)
{
    str[0] = file_type_char(mode);

    str[1] = (mode & S_IRUSR) ? 'r' : '-';
    str[2] = (mode & S_IWUSR) ? 'w' : '-';
    str[3] = (mode & S_IXUSR) ? 'x' : '-';

    str[4] = (mode & S_IRGRP) ? 'r' : '-';
    str[5] = (mode & S_IWGRP) ? 'w' : '-';
    str[6] = (mode & S_IXGRP) ? 'x' : '-';

    str[7] = (mode & S_IROTH) ? 'r' : '-';
    str[8] = (mode & S_IWOTH) ? 'w' : '-';
    str[9] = (mode & S_IXOTH) ? 'x' : '-';

    /*
     * setuid / setgid / sticky formatting follows ls conventions.
     */
    if (mode & S_ISUID) {
        str[3] =
            (mode & S_IXUSR) ? 's' : 'S';
    }

    if (mode & S_ISGID) {
        str[6] =
            (mode & S_IXGRP) ? 's' : 'S';
    }

    if (mode & S_ISVTX) {
        str[9] =
            (mode & S_IXOTH) ? 't' : 'T';
    }

    str[10] = '\0';
}

char get_file_suffix(
    const struct stat *st
)
{
    if (S_ISDIR(st->st_mode)) {
        return '/';
    }

    if (S_ISLNK(st->st_mode)) {
        return '@';
    }

#ifdef S_ISWHT
    if (S_ISWHT(st->st_mode)) {
        return '%';
    }
#endif

#ifdef S_ISSOCK
    if (S_ISSOCK(st->st_mode)) {
        return '=';
    }
#endif

    if (S_ISFIFO(st->st_mode)) {
        return '|';
    }

    if (S_ISREG(st->st_mode) &&
        (st->st_mode &
         (S_IXUSR | S_IXGRP | S_IXOTH))) {
        return '*';
    }

    return '\0';
}

void format_size(
    unsigned long long size,
    char *buffer,
    size_t buffer_size
)
{
    static const char units[] = "BKMGTPE";
    double value;
    int unit;

    value = (double)size;
    unit = 0;

    while (value >= 1024.0 &&
           unit < (int)sizeof(units) - 2) {
        value /= 1024.0;
        unit++;
    }

    if (unit == 0) {
        snprintf(
            buffer,
            buffer_size,
            "%lluB",
            size
        );
    } else if (value >= 10.0) {
        snprintf(
            buffer,
            buffer_size,
            "%.0f%c",
            value,
            units[unit]
        );
    } else {
        snprintf(
            buffer,
            buffer_size,
            "%.1f%c",
            value,
            units[unit]
        );
    }
}

/*
 * st_blocks is reported in 512-byte filesystem blocks on the
 * target NetBSD environment.
 */
unsigned long long get_display_block_bytes(
    const struct stat *st
)
{
    if (st->st_blocks < 0) {
        return 0;
    }

    return (unsigned long long)st->st_blocks * 512ULL;
}

unsigned long long get_display_blocks(
    const struct stat *st,
    Options *opt
)
{
    unsigned long long bytes;
    unsigned long long block_size;

    bytes = get_display_block_bytes(st);

    /*
     * -k always means 1024-byte blocks.
     * Otherwise BLOCKSIZE is used, with 512 as default.
     */
    if (opt->k) {
        block_size = 1024ULL;
    } else {
        block_size = get_block_size();
    }

    if (bytes == 0) {
        return 0;
    }

    /*
     * Round partial blocks upward.
     */
    return (
        bytes + block_size - 1
    ) / block_size;
}

/*
 * Print a pathname according to -q / -w and terminal defaults.
 *
 * -q  -> non-printable characters become '?'
 * -w  -> raw characters
 *
 * If neither option is specified:
 *   terminal     -> ? behavior
 *   non-terminal -> raw behavior
 */
const char *display_name(
    const char *name,
    Options *opt,
    char *buffer,
    size_t buffer_size
)
{
    size_t i;
    size_t out;
    int use_question_mark;

    if (buffer_size == 0) {
        return buffer;
    }

    if (opt->w) {
        snprintf(
            buffer,
            buffer_size,
            "%s",
            name
        );

        return buffer;
    }

    if (opt->q) {
        use_question_mark = 1;
    } else if (isatty(STDOUT_FILENO)) {
        use_question_mark = 1;
    } else {
        use_question_mark = 0;
    }

    if (!use_question_mark) {
        snprintf(
            buffer,
            buffer_size,
            "%s",
            name
        );

        return buffer;
    }

    out = 0;

    for (i = 0; name[i] != '\0'; i++) {
        unsigned char ch;

        ch = (unsigned char)name[i];

        if (isprint(ch)) {
            if (out + 1 >= buffer_size) {
                break;
            }

            buffer[out++] = (char)ch;
        } else {
            if (out + 1 >= buffer_size) {
                break;
            }

            buffer[out++] = '?';
        }
    }

    buffer[out] = '\0';

    return buffer;
}

int join_path(
    const char *directory,
    const char *name,
    char *buffer,
    size_t buffer_size
)
{
    int result;

    if (strcmp(directory, "/") == 0) {
        result = snprintf(
            buffer,
            buffer_size,
            "/%s",
            name
        );
    } else {
        result = snprintf(
            buffer,
            buffer_size,
            "%s/%s",
            directory,
            name
        );
    }

    if (result < 0 ||
        (size_t)result >= buffer_size) {
        return -1;
    }

    return 0;
}
