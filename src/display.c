#include <stdio.h>
#include <string.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#ifdef __linux__
#include <sys/sysmacros.h>
#endif

#include "display.h"
#include "utils.h"

static void print_owner(
    uid_t uid,
    Options *opt
)
{
    struct passwd *pw;

    if (opt->n) {
        printf(
            "%u",
            (unsigned int)uid
        );
        return;
    }

    pw = getpwuid(uid);

    if (pw != NULL) {
        printf(
            "%s",
            pw->pw_name
        );
    } else {
        printf(
            "%u",
            (unsigned int)uid
        );
    }
}

static void print_group(
    gid_t gid,
    Options *opt
)
{
    struct group *gr;

    if (opt->n) {
        printf(
            "%u",
            (unsigned int)gid
        );
        return;
    }

    gr = getgrgid(gid);

    if (gr != NULL) {
        printf(
            "%s",
            gr->gr_name
        );
    } else {
        printf(
            "%u",
            (unsigned int)gid
        );
    }
}

static void print_time(
    const struct stat *st,
    Options *opt
)
{
    time_t value;
    struct tm *tm_info;
    char buffer[64];

    if (opt->c) {
        value = st->st_ctime;
    } else if (opt->u) {
        value = st->st_atime;
    } else {
        value = st->st_mtime;
    }

    tm_info = localtime(&value);

    if (tm_info == NULL) {
        printf("??? ?? ??:??");
        return;
    }

    if (strftime(
            buffer,
            sizeof(buffer),
            "%b %e %H:%M",
            tm_info
        ) == 0) {
        printf("??? ?? ??:??");
        return;
    }

    printf(
        "%s",
        buffer
    );
}

static void print_device_size(
    const struct stat *st
)
{
    printf(
        "%u,%u",
        (unsigned int)major(st->st_rdev),
        (unsigned int)minor(st->st_rdev)
    );
}

static void print_size_field(
    const struct stat *st,
    Options *opt
)
{
    char buffer[64];

    if (S_ISCHR(st->st_mode) ||
        S_ISBLK(st->st_mode)) {
        print_device_size(st);
        return;
    }

    if (opt->h) {
        format_size(
            (unsigned long long)st->st_size,
            buffer,
            sizeof(buffer)
        );

        printf(
            "%8s",
            buffer
        );
    } else {
        printf(
            "%8lld",
            (long long)st->st_size
        );
    }
}

static void print_block_field(
    const struct stat *st,
    Options *opt
)
{
    char buffer[64];

    if (!opt->s) {
        return;
    }

    if (opt->h) {
        /*
         * -h overrides -k and BLOCKSIZE.
         * st_blocks is expressed in 512-byte units.
         */
        format_size(
            get_display_block_bytes(st),
            buffer,
            sizeof(buffer)
        );

        printf(
            "%s ",
            buffer
        );
    } else {
        printf(
            "%llu ",
            get_display_blocks(st, opt)
        );
    }
}

static void print_inode(
    const struct stat *st,
    Options *opt
)
{
    if (opt->i) {
        printf(
            "%llu ",
            (unsigned long long)st->st_ino
        );
    }
}

static void print_long(
    FileEntry *entry,
    Options *opt
)
{
    char mode[11];
    char name_buffer[MYLS_NAME_SIZE];
    char link_buffer[MYLS_PATH_SIZE];
    ssize_t link_length;

    mode_to_string(
        entry->st.st_mode,
        mode
    );

    printf(
        "%s %3u ",
        mode,
        (unsigned int)entry->st.st_nlink
    );

    print_owner(
        entry->st.st_uid,
        opt
    );

    printf("  ");

    print_group(
        entry->st.st_gid,
        opt
    );

    printf(" ");

    print_size_field(
        &entry->st,
        opt
    );

    printf(" ");

    print_time(
        &entry->st,
        opt
    );

    printf(" ");

    display_name(
        entry->name,
        opt,
        name_buffer,
        sizeof(name_buffer)
    );

    printf(
        "%s",
        name_buffer
    );

    if (opt->F) {
        char suffix;

        suffix = get_file_suffix(
            &entry->st
        );

        if (suffix != '\0') {
            putchar(suffix);
        }
    }

    if (S_ISLNK(entry->st.st_mode)) {
        link_length = readlink(
            entry->path,
            link_buffer,
            sizeof(link_buffer) - 1
        );

        if (link_length >= 0) {
            link_buffer[link_length] = '\0';

            printf(
                " -> %s",
                link_buffer
            );
        }
    }

    putchar('\n');
}

static void print_plain(
    FileEntry *entry,
    Options *opt
)
{
    char name_buffer[MYLS_NAME_SIZE];

    print_inode(
        &entry->st,
        opt
    );

    print_block_field(
        &entry->st,
        opt
    );

    display_name(
        entry->name,
        opt,
        name_buffer,
        sizeof(name_buffer)
    );

    printf(
        "%s",
        name_buffer
    );

    if (opt->F) {
        char suffix;

        suffix = get_file_suffix(
            &entry->st
        );

        if (suffix != '\0') {
            putchar(suffix);
        }
    }

    putchar('\n');
}

void display_single(
    FileEntry *entry,
    Options *opt
)
{
    if (opt->l) {
        print_inode(
            &entry->st,
            opt
        );

        print_block_field(
            &entry->st,
            opt
        );

        print_long(
            entry,
            opt
        );
    } else {
        print_plain(
            entry,
            opt
        );
    }
}

void display_total(
    FileEntry *entries,
    int count,
    Options *opt
)
{
    unsigned long long total;
    int i;

    if (!opt->l && !opt->s) {
        return;
    }

    /*
     * NetBSD ls prints the directory total when the output
     * is going to a terminal.
     */
    if (!isatty(STDOUT_FILENO)) {
        return;
    }

    total = 0;

    for (i = 0; i < count; i++) {
        total += get_display_blocks(
            &entries[i].st,
            opt
        );
    }

    if (opt->h) {
        unsigned long long bytes;
        char buffer[64];

        /*
         * -h overrides -k and BLOCKSIZE.
         * Convert the actual 512-byte filesystem blocks
         * into a human-readable byte value.
         */
        bytes = 0;

        for (i = 0; i < count; i++) {
            bytes += get_display_block_bytes(
                &entries[i].st
            );
        }

        format_size(
            bytes,
            buffer,
            sizeof(buffer)
        );

        printf(
            "total %s\n",
            buffer
        );
    } else {
        printf(
            "total %llu\n",
            total
        );
    }
}

void display_entries(
    FileEntry *entries,
    int count,
    Options *opt
)
{
    int i;

    if (entries == NULL ||
        count <= 0) {
        if ((opt->l || opt->s) &&
            isatty(STDOUT_FILENO)) {
            printf("total 0\n");
        }

        return;
    }

    display_total(
        entries,
        count,
        opt
    );

    for (i = 0; i < count; i++) {
        display_single(
            &entries[i],
            opt
        );
    }
}
