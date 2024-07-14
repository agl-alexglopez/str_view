/* Author: Alexander G. Lopez
   File: mini_grep.c
   =================
   Mini Grep is a small tester program that can search a provided file or
   directory for all occurrences of a target string. The program does not
   support regular expression parsing and can only search strings as
   provided. Its purpose is to test the str_view module. Here is usage
   (replace rel in the executable path with deb if built in debug mode):

       ./build/rel/mini_grep [OPTIONAL IO] [FILE/DIRECTORY] [string]
       # Searching one file for all occurrences of string "const"
       ./build/rel/mini_grep str_view/str_view.c const
       # Searching all files in directory for string ")"
       ./build/rel/mini_grep str_view/ \)
       ./build/rel/mini_grep --mmap str_view/ \)

   The optional IO flag let's one choose to use mmap as the memory backing
   for the file IO. This is just for experimental purposes and for the use
   cases of this program the default file reading is faster. No flags are
   required for the --read method but the flag is provided for completeness.
   Directories must end in a slash to be searched as directories. Directories
   are not searched recursively. Only files in a directory are searched.
   If multiple files in a directory search have output the file name follows
   the output and can be seen at the bottom below the last found output
   for that file line. */
#include "str_view.h"
#include <dirent.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>

#ifdef __linux__
#    include <linux/limits.h>
#    define FILESYS_MAX_PATH PATH_MAX
#endif
#ifdef __APPLE__
#    include <sys/syslimits.h>
#    define FILESYS_MAX_PATH NAME_MAX
#endif

#define CYAN "\033[38;5;14m"
#define NONE "\033[0m"
#define RED "\033[38;5;9m"
#define PNK "\033[38;5;13m"

enum io_method
{
    READ,
    MMAP,
};

struct file_buf
{
    char const *const buf;
    size_t size;
};

str_view const mmap_flag = SV("--mmap");
str_view const read_flag = SV("--read");

static int run(char * [static 1], size_t);
static bool match_file_mmap(str_view, str_view);
static bool match_file_read(FILE *, str_view);
static bool match_line(size_t, str_view, str_view);
static void search_directory(str_view, DIR *, enum io_method, str_view);
static bool fill_path(char[static FILESYS_MAX_PATH], str_view, str_view);
static struct file_buf get_file_buf(FILE *);

int
main(int argc, char **argv)
{
    if (argc <= 1)
    {
        return 0;
    }
    return run(argv + 1, (size_t)(argc - 1));
}

static int
run(char *args[static 1], size_t argc)
{
    enum io_method io_style = READ;
    size_t start = 0;
    if (sv_strcmp(mmap_flag, args[start]) == SV_EQL)
    {
        io_style = MMAP;
        ++start;
    }
    else if (sv_strcmp(read_flag, args[start]) == SV_EQL)
    {
        io_style = READ;
        ++start;
    }
    struct stat file_type = {0};
    stat(args[start], &file_type);
    if (S_ISDIR(file_type.st_mode))
    {
        DIR *d = opendir(args[start]);
        if (!d)
        {
            (void)fprintf(stderr, "Could not open directory.\n");
            return 1;
        }
        str_view const dir_view = sv(args[start]);
        for (size_t i = start + 1; i < argc; ++i)
        {
            search_directory(dir_view, d, io_style, sv(args[i]));
        }
        closedir(d);
    }
    else
    {
        bool opened_file = true;
        str_view const filename = sv(args[start]);
        FILE *f = fopen(sv_begin(filename), "r");
        if (!f)
        {
            opened_file = false;
            f = stdin;
            io_style = READ;
        }
        for (size_t i = start + opened_file; i < argc; ++i)
        {
            switch (io_style)
            {
            case READ:
                (void)match_file_read(f, sv(args[i]));
                break;
            case MMAP:
                (void)match_file_mmap(filename, sv(args[i]));
                break;
            }
        }
        if (opened_file)
        {
            (void)fclose(f);
        }
    }
    return 0;
}

static void
search_directory(str_view dirname, DIR *d, enum io_method io, str_view needle)
{
    struct dirent const *de;
    char path_buf[FILESYS_MAX_PATH];
    while ((de = readdir(d)))
    {
        struct stat file_type = {0};
        stat(de->d_name, &file_type);
        if (S_ISDIR(file_type.st_mode))
        {
            continue;
        }
        if (!fill_path(path_buf, dirname, sv(de->d_name)))
        {
            continue;
        }
        str_view const path_view = sv(path_buf);
        bool match_res = false;
        if (io == READ)
        {
            FILE *f = fopen(sv_begin(path_view), "r");
            if (f)
            {
                match_res = match_file_read(f, needle);
                (void)fclose(f);
            }
        }
        else if (io == MMAP)
        {
            match_res = match_file_mmap(path_view, needle);
        }
        else
        {
            (void)fprintf(stderr, "Review io options, one slipped through.\n");
        }
        if (match_res)
        {
            (void)fprintf(stdout, PNK);
            sv_print(stdout, path_view);
            (void)fprintf(stdout, "\n\n" NONE);
        }
    }
    seekdir(d, 0);
}

static bool
match_file_read(FILE *f, str_view needle)
{
    char *lineptr = NULL;
    size_t len = 0;
    ssize_t read = 0;
    size_t lineno = 1;
    bool found = false;
    while ((read = getline(&lineptr, &len, f)) != -1)
    {
        if (read && match_line(lineno, (str_view){lineptr, read - 1}, needle))
        {
            found = true;
        }
        ++lineno;
    }
    free(lineptr);
    return found;
}

static bool
match_file_mmap(str_view const filename, str_view needle)
{
    FILE *f = fopen(sv_begin(filename), "r");
    if (!f)
    {
        (void)fprintf(stderr, "error opening file %s, continuing.\n",
                      sv_begin(filename));
        return false;
    }
    struct file_buf const fb = get_file_buf(f);
    if (fclose(f))
    {
        (void)fprintf(stderr, "Error closing file.\n");
        return false;
    }
    if (!fb.buf)
    {
        return false;
    }
    size_t read = 0;
    size_t lineno = 1;
    bool found = false;
    for (str_view line = sv_delim(fb.buf + read, "\n"); !sv_empty(line);
         line = sv_delim(fb.buf + read, "\n"))
    {
        read += sv_size(line);
        if (match_line(lineno, line, needle))
        {
            found = true;
        }
        ++lineno;
        /* The str_view type conveniently skips leading delims but if the
           line number shall be accurate, these empty lines need to be
           counted.*/
        while (read < fb.size && fb.buf[read] == '\n')
        {
            read++;
            lineno++;
        }
    }
    if (munmap((void *)fb.buf, fb.size) < 0)
    {
        (void)fprintf(stderr, "error unmapping file.\n");
    }
    return found;
}

static bool
match_line(size_t lineno, str_view line, str_view needle)
{
    size_t last_pos = 0;
    size_t pos = 0;
    while ((pos = sv_find(line, pos, needle)) != sv_npos(line))
    {
        if (!last_pos)
        {
            (void)fprintf(stdout, CYAN "%zu:" NONE, lineno);
        }
        sv_print(stdout, sv_substr(line, last_pos, pos - last_pos));
        (void)fprintf(stdout, RED);
        sv_print(stdout, needle);
        (void)fprintf(stdout, NONE);
        last_pos = pos + sv_len(needle);
        ++pos;
    }
    if (last_pos)
    {
        sv_print(stdout, sv_substr(line, last_pos, sv_len(line)));
        (void)fprintf(stdout, "\n");
    }
    return last_pos != 0;
}

static bool
fill_path(char path_buf[static FILESYS_MAX_PATH], str_view tests_dir,
          str_view entry)
{
    size_t const dir_bytes = sv_fill(FILESYS_MAX_PATH, path_buf, tests_dir);
    if (FILESYS_MAX_PATH - dir_bytes < sv_size(entry))
    {
        (void)fprintf(stderr, "Relative path exceeds FILESYS_MAX_PATH?\n%s",
                      path_buf);
        return false;
    }
    (void)sv_fill(FILESYS_MAX_PATH - dir_bytes, path_buf + sv_len(tests_dir),
                  entry);
    return true;
}

static struct file_buf
get_file_buf(FILE *f)
{
    if (fseek(f, 0L, SEEK_END) < 0)
    {
        (void)fprintf(stderr, "error seeking in file.\n");
        return (struct file_buf){0};
    }
    size_t const size = ftell(f);
    if (fseek(f, 0L, SEEK_SET) < 0)
    {
        (void)fprintf(stderr, "error seeking in file.\n");
        return (struct file_buf){0};
    }
    char const *const buf
        = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fileno(f), 0);
    if (buf == MAP_FAILED)
    {
        (void)fprintf(stderr, "could not read file into memory.\n");
        return (struct file_buf){0};
    }
    return (struct file_buf){.buf = buf, .size = size};
}
