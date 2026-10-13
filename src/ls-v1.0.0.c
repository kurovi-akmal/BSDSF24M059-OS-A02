/*
* Programming Assignment 02: lsv1.0.0
* This is the source file of version 1.0.0
* Read the write-up of the assignment to add the features to this base version
* Usage:
*       $ lsv1.0.0 
*       % lsv1.0.0  /home
*       $ lsv1.0.0  /home/kali/   /etc/
*/


#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>

#define COLOR_RESET "\033[0m"
#define COLOR_BLUE  "\033[1;34m"
#define COLOR_GREEN "\033[1;32m"
#define COLOR_CYAN  "\033[1;36m"

#define PATH_SIZE 4096

int long_listing = 0;
int horizontal_listing = 0;
int recursive_flag = 0;

void do_ls(const char *dir);
void get_permissions(mode_t mode, char permissions[11]);
int get_terminal_width(void);
int compare_filenames(const void *a, const void *b);
const char *get_file_color(const char *dir, const char *name);
void print_colored_name(const char *dir, const char *name);
void print_columns(const char *dir, char **filenames,
                   size_t count, size_t max_length);
void print_horizontal(const char *dir, char **filenames,
                      size_t count, size_t max_length);

int main(int argc, char *argv[])
{
    int option;

    while ((option = getopt(argc, argv, "lxR")) != -1)
    {
        switch (option)
        {
            case 'l':
                long_listing = 1;
                break;

            case 'x':
                horizontal_listing = 1;
                break;

            case 'R':
                recursive_flag = 1;
                break;

            default:
                fprintf(stderr, "Usage: %s [-l] [-x] [-R] [directory ...]\n",
                        argv[0]);
                return 1;
        }
    }

    if (long_listing && horizontal_listing)
    {
        fprintf(stderr, "Use either -l or -x, not both.\n");
        return 1;
    }

    if (optind == argc)
    {
        do_ls(".");
    }
    else
    {
        for (int i = optind; i < argc; i++)
        {
            if (recursive_flag || argc - optind > 1)
                printf("%s:\n", argv[i]);

            do_ls(argv[i]);

            if (i < argc - 1)
                putchar('\n');
        }
    }

    return 0;
}

int get_terminal_width(void)
{
    struct winsize ws;

    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 &&
        ws.ws_col > 0)
        return ws.ws_col;

    return 80;
}

int compare_filenames(const void *a, const void *b)
{
    const char *name_a = *(const char * const *)a;
    const char *name_b = *(const char * const *)b;

    return strcmp(name_a, name_b);
}

void get_permissions(mode_t mode, char permissions[11])
{
    permissions[0] = S_ISDIR(mode) ? 'd' :
                     S_ISLNK(mode) ? 'l' : '-';

    permissions[1] = (mode & S_IRUSR) ? 'r' : '-';
    permissions[2] = (mode & S_IWUSR) ? 'w' : '-';
    permissions[3] = (mode & S_IXUSR) ? 'x' : '-';
    permissions[4] = (mode & S_IRGRP) ? 'r' : '-';
    permissions[5] = (mode & S_IWGRP) ? 'w' : '-';
    permissions[6] = (mode & S_IXGRP) ? 'x' : '-';
    permissions[7] = (mode & S_IROTH) ? 'r' : '-';
    permissions[8] = (mode & S_IWOTH) ? 'w' : '-';
    permissions[9] = (mode & S_IXOTH) ? 'x' : '-';

    if (mode & S_ISUID)
        permissions[3] = (mode & S_IXUSR) ? 's' : 'S';

    if (mode & S_ISGID)
        permissions[6] = (mode & S_IXGRP) ? 's' : 'S';

    if (mode & S_ISVTX)
        permissions[9] = (mode & S_IXOTH) ? 't' : 'T';

    permissions[10] = '\0';
}

const char *get_file_color(const char *dir, const char *name)
{
    char path[PATH_SIZE];
    struct stat info;

    int length = snprintf(path, sizeof(path), "%s/%s", dir, name);

    if (length < 0 || (size_t)length >= sizeof(path))
        return COLOR_RESET;

    if (lstat(path, &info) == -1)
        return COLOR_RESET;

    if (S_ISDIR(info.st_mode))
        return COLOR_BLUE;

    if (S_ISLNK(info.st_mode))
        return COLOR_CYAN;

    if (info.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH))
        return COLOR_GREEN;

    return COLOR_RESET;
}

void print_colored_name(const char *dir, const char *name)
{
    printf("%s%s%s", get_file_color(dir, name),
           name, COLOR_RESET);
}

void print_columns(const char *dir, char **filenames,
                   size_t count, size_t max_length)
{
    if (count == 0)
        return;

    size_t column_width = max_length + 2;
    size_t terminal_width = (size_t)get_terminal_width();
    size_t columns = terminal_width / column_width;

    if (columns == 0)
        columns = 1;

    if (columns > count)
        columns = count;

    size_t rows = (count + columns - 1) / columns;

    for (size_t row = 0; row < rows; row++)
    {
        for (size_t col = 0; col < columns; col++)
        {
            size_t index = col * rows + row;

            if (index < count)
            {
                print_colored_name(dir, filenames[index]);

                size_t padding =
                    column_width - strlen(filenames[index]);

                printf("%*s", (int)padding, "");
            }
        }

        putchar('\n');
    }
}

void print_horizontal(const char *dir, char **filenames,
                      size_t count, size_t max_length)
{
    if (count == 0)
        return;

    size_t column_width = max_length + 2;
    size_t terminal_width = (size_t)get_terminal_width();
    size_t current_position = 0;

    for (size_t i = 0; i < count; i++)
    {
        if (current_position > 0 &&
            current_position + column_width > terminal_width)
        {
            putchar('\n');
            current_position = 0;
        }

        print_colored_name(dir, filenames[i]);

        size_t padding = column_width - strlen(filenames[i]);
        printf("%*s", (int)padding, "");

        current_position += column_width;
    }

    putchar('\n');
}

void do_ls(const char *dir)
{
    DIR *dp = opendir(dir);

    if (dp == NULL)
    {
        fprintf(stderr, "Cannot open directory: %s: %s\n",
                dir, strerror(errno));
        return;
    }

    if (recursive_flag)
        printf("%s:\n", dir);

    size_t capacity = 10;
    size_t count = 0;
    size_t max_length = 0;

    char **filenames = malloc(capacity * sizeof(*filenames));

    if (filenames == NULL)
    {
        perror("malloc");
        closedir(dp);
        return;
    }

    struct dirent *entry;
    int read_error = 0;

    while (1)
    {
        errno = 0;
        entry = readdir(dp);

        if (entry == NULL)
        {
            if (errno != 0)
            {
                perror("readdir");
                read_error = 1;
            }
            break;
        }

        if (entry->d_name[0] == '.')
            continue;

        if (count == capacity)
        {
            size_t new_capacity = capacity * 2;

            char **temp = realloc(
                filenames, new_capacity * sizeof(*filenames));

            if (temp == NULL)
            {
                perror("realloc");
                read_error = 1;
                break;
            }

            filenames = temp;
            capacity = new_capacity;
        }

        char *name = strdup(entry->d_name);

        if (name == NULL)
        {
            perror("strdup");
            read_error = 1;
            break;
        }

        filenames[count++] = name;

        size_t length = strlen(name);

        if (length > max_length)
            max_length = length;
    }

    closedir(dp);

    qsort(filenames, count, sizeof(*filenames), compare_filenames);

    if (recursive_flag || long_listing)
        putchar('\n');

    if (long_listing)
    {
        for (size_t i = 0; i < count; i++)
        {
            char path[PATH_SIZE];
            struct stat info;
            char permissions[11];

            int length = snprintf(path, sizeof(path), "%s/%s",
                                  dir, filenames[i]);

            if (length < 0 || (size_t)length >= sizeof(path))
            {
                fprintf(stderr, "Path too long: %s\n", filenames[i]);
                continue;
            }

            if (lstat(path, &info) == -1)
            {
                perror(path);
                continue;
            }

            struct passwd *owner = getpwuid(info.st_uid);
            struct group *group = getgrgid(info.st_gid);

            char time_buffer[32];
            struct tm *tm_info = localtime(&info.st_mtime);

            if (tm_info != NULL)
                strftime(time_buffer, sizeof(time_buffer),
                         "%b %d %H:%M", tm_info);
            else
                snprintf(time_buffer, sizeof(time_buffer), "Unknown");

            get_permissions(info.st_mode, permissions);

            printf("%s %2lu %-8s %-8s %8ld %s ",
                   permissions, (unsigned long)info.st_nlink,
                   owner ? owner->pw_name : "unknown",
                   group ? group->gr_name : "unknown",
                   (long)info.st_size, time_buffer);

            print_colored_name(dir, filenames[i]);
            putchar('\n');
        }
    }
    else if (horizontal_listing)
    {
        print_horizontal(dir, filenames, count, max_length);
    }
    else
    {
        print_columns(dir, filenames, count, max_length);
    }

    /*
     * Recurse only after displaying the current directory.
     * lstat prevents symbolic links to directories from being followed.
     */
    if (recursive_flag)
    {
        for (size_t i = 0; i < count; i++)
        {
            char path[PATH_SIZE];
            struct stat info;

            int length = snprintf(path, sizeof(path), "%s/%s",
                                  dir, filenames[i]);

            if (length < 0 || (size_t)length >= sizeof(path))
            {
                fprintf(stderr, "Path too long: %s\n", filenames[i]);
                continue;
            }

            if (lstat(path, &info) == -1)
            {
                perror(path);
                continue;
            }

            if (S_ISDIR(info.st_mode))
            {
                putchar('\n');
                do_ls(path);
            }
        }
    }

    for (size_t i = 0; i < count; i++)
        free(filenames[i]);

    free(filenames);

    if (read_error)
        fprintf(stderr, "Warning: listing may be incomplete.\n");
}
