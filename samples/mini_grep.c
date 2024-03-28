/* Author: Alexander G. Lopez
   File: mini_grep.c
   =================
   Mini Grep is a small tester program that can search a provided file or
   directory for all occurrences of a target string. The program does not
   support regular expression parsing and can only search strings as
   provided. Its purpose is to test the str_view module. Here is usage
   (replace rel in the executable path with deb if built in debug mode):

       ./build/rel/mini_grep [FILE/DIRECTORY] [string]
       # Searching one file for all occurrences of string "const"
       ./build/rel/mini_grep str_view/str_view.c const
       # Searching all files in directory for string ")"
       ./build/rel/mini_grep str_view/ \)

   Directories must end in a slash to be searched as directories. Directories
   are not searched recursively. Only files in a directory are searched. */
#include "str_view.h"
#include <dirent.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
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

struct file_buf
{
    const char *buf;
    size_t size;
};

static int run(char * [static 1], size_t);
static void search_file(str_view, str_view);
static bool match_line(int, size_t, str_view, str_view);
static void search_directory(str_view, DIR *, str_view);
static bool fill_path(char[static FILESYS_MAX_PATH], str_view, str_view);
static int num_digit_places(size_t);
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
    struct stat file_type = {0};
    stat(args[0], &file_type);
    if (S_ISDIR(file_type.st_mode))
    {
        DIR *d = opendir(args[0]);
        if (!d)
        {
            (void)fprintf(stderr, "Could not open directory.\n");
            return 1;
        }
        const str_view dir_view = sv(args[0]);
        for (size_t i = 1; i < argc; ++i)
        {
            search_directory(dir_view, d, sv(args[i]));
        }
        closedir(d);
    }
    else if (S_ISREG(file_type.st_mode))
    {
        FILE *f = fopen(args[0], "r");
        if (!f)
        {
            (void)fprintf(stderr, "Could not open file.\n");
            return 1;
        }
        const str_view filename = sv(args[0]);
        for (size_t i = 1; i < argc; ++i)
        {
            search_file(filename, sv(args[i]));
        }
    }
    else
    {
        (void)fprintf(stderr,
                      "mini grep can only search files and directories.\n");
        return 1;
    }
    return 0;
}

static void
search_directory(str_view dirname, DIR *d, str_view needle)
{
    const struct dirent *de;
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
        search_file(sv(path_buf), needle);
    }
    seekdir(d, 0);
}

static void
search_file(const str_view filename, str_view needle)
{
    FILE *f = fopen(sv_begin(filename), "r");
    if (!f)
    {
        (void)fprintf(stderr, "error opening file %s, continuing.\n",
                      sv_begin(filename));
        return;
    }
    const struct file_buf fb = get_file_buf(f);
    if (fclose(f))
    {
        (void)fprintf(stderr, "Error closing file.\n");
        return;
    }
    if (!fb.buf)
    {
        return;
    }
    const int print_width = num_digit_places(fb.size);
    size_t read = 0;
    size_t lineno = 1;
    bool found = false;
    while (read < fb.size)
    {
        const str_view line = sv_delim(fb.buf + read, "\n");
        if (sv_empty(line))
        {
            break;
        }
        read += sv_size(line);
        if (match_line(print_width, lineno, line, needle))
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
    if (found)
    {
        (void)fprintf(stdout, CYAN "\n↑↑↑ ");
        sv_print(stdout, filename);
        (void)fprintf(stdout, " ↑↑↑\n\n" NONE);
    }
}

static bool
match_line(int print_width, size_t lineno, str_view line, str_view needle)
{
    size_t last_pos = 0;
    size_t pos = 0;
    while ((pos = sv_find(line, pos, needle)) != sv_npos(line))
    {
        if (!last_pos)
        {
            (void)fprintf(stdout, CYAN "%-*zu " NONE, print_width, lineno);
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
    const size_t dir_bytes = sv_fill(FILESYS_MAX_PATH, path_buf, tests_dir);
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
    const size_t size = ftell(f);
    if (fseek(f, 0L, SEEK_SET) < 0)
    {
        (void)fprintf(stderr, "error seeking in file.\n");
        return (struct file_buf){0};
    }
    const char *const buf
        = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fileno(f), 0);
    if (buf == MAP_FAILED)
    {
        (void)fprintf(stderr, "could not read file into memory.\n");
        return (struct file_buf){0};
    }
    return (struct file_buf){.buf = buf, .size = size};
}

static int
num_digit_places(size_t size)
{
    int digit_places = 0;
    while (size)
    {
        ++digit_places;
        size /= 10;
    }
    return digit_places;
}
