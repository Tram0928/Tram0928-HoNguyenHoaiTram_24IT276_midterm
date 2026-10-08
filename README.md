

## Introduction

This project is a reimplementation of the NetBSD `ls` command in C.

Supported options:

```text
-A -a -c -d -F -f -h -i -k -l
-n -q -R -r -S -s -t -u -w
```

The program is implemented using multiple source files and header files and is compiled using the provided Makefile.

---

## Project Structure

```text
.
├── Makefile
├── README.md
├── .gitignore
├── include/
│   ├── display.h
│   ├── listing.h
│   ├── parser.h
│   ├── recursion.h
│   ├── sorting.h
│   └── utils.h
└── src/
    ├── main.c
    ├── parser.c
    ├── listing.c
    ├── sorting.c
    ├── display.c
    ├── recursion.c
    └── utils.c
```

---

## Build

Compile:

```bash
make
```

Clean build files:

```bash
make clean
```

---

## Usage

General syntax:

```bash
./myls [options] [file ...]
```

Examples:

List current directory:

```bash
./myls
```

Show hidden files:

```bash
./myls -a
```

Long format:

```bash
./myls -l
```

Recursive listing:

```bash
./myls -R
```

Sort by size:

```bash
./myls -S
```

Sort by modification time:

```bash
./myls -t
```

Directory as file:

```bash
./myls -d src
```

Show inode numbers:

```bash
./myls -i
```

Human-readable sizes:

```bash
./myls -hs
```

Display file type indicators:

```bash
./myls -F
```

---

## Implemented Options

| Option | Description                               |
| ------ | ----------------------------------------- |
| -A     | Show all entries except `.` and `..`      |
| -a     | Show hidden files                         |
| -c     | Use status change time                    |
| -d     | Treat directories as normal files         |
| -F     | Append file type indicators               |
| -f     | Disable sorting                           |
| -h     | Human readable sizes                      |
| -i     | Display inode number                      |
| -k     | Display sizes in KB                       |
| -l     | Long format                               |
| -n     | Numeric UID/GID                           |
| -q     | Replace non-printable characters with `?` |
| -R     | Recursive listing                         |
| -r     | Reverse sorting order                     |
| -S     | Sort by file size                         |
| -s     | Show block usage                          |
| -t     | Sort by time                              |
| -u     | Use access time                           |
| -w     | Print raw filenames                       |

---

## Notes

Output may differ slightly between systems because of:

* Different inode numbers
* Different owner/group names
* Different timestamps
* Different filesystem block sizes
* Different compiler versions

Examples:

```text
root wheel
```

or

```text
root root
```

Both are valid depending on the operating system.

The implementation follows the NetBSD `ls` specification required by the assignment.

