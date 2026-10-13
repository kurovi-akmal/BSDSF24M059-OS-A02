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

int long_listing = 0;
int horizontal_listing = 0;

void do_ls(const char *dir);
void get_permissions(mode_t mode, char permissions[11]);
int get_terminal_width(void);

void print_columns(char **filenames, size_t count,
                   size_t max_length);

void print_horizontal(char **filenames, size_t count,
                      size_t max_length);

int main(int argc, char *argv[])
{
    int option;

    while ((option = getopt(argc, argv, "lx")) != -1)
    {
        switch (option)
        {
            case 'l':
                long_listing = 1;
                break;

            case 'x':
                horizontal_listing = 1;
                break;

            default:
                fprintf(stderr, "Usage: %s [-l] [-x] [directory ...]\n",
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
            if (argc - optind > 1)
                printf("Directory listing of %s:\n", argv[i]);

            do_ls(argv[i]);
        }
    }

    return 0;
}

int get_terminal_width(void)
{
    struct winsize ws;

    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 &&
        ws.ws_col > 0)
    {
        return ws.ws_col;
    }

    return 80;
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

/* Default display: down then across */
void print_columns(char **filenames, size_t count,
                   size_t max_length)
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
                printf("%-*s", (int)column_width, filenames[index]);
        }

        putchar('\n');
    }
}

/* -x display: left to right, wrapping at terminal width */
void print_horizontal(char **filenames, size_t count,
                      size_t max_length)
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

        printf("%-*s", (int)column_width, filenames[i]);
        current_position += column_width;
    }

    putchar('\n');
}

void do_ls(const char *dir)
{
    DIR *dp = opendir(dir);

    if (dp == NULL)
    {
        fprintf(stderr, "Cannot open directory: %s\n", dir);
        return;
    }

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

        /* Preserve the starter program's hidden-file behavior. */
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

    if (long_listing)
    {
        for (size_t i = 0; i < count; i++)
        {
            char path[4096];
            struct stat info;
            char permissions[11];

            int path_length = snprintf(
                path, sizeof(path), "%s/%s", dir, filenames[i]);

            if (path_length < 0 ||
                (size_t)path_length >= sizeof(path))
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
            char *time_string = ctime(&info.st_mtime);

            if (time_string != NULL)
                time_string[strcspn(time_string, "\n")] = '\0';
            else
                time_string = "Unknown";

            get_permissions(info.st_mode, permissions);

            printf("%s %2lu %-8s %-8s %8ld %s %s\n",
                   permissions,
                   (unsigned long)info.st_nlink,
                   owner ? owner->pw_name : "unknown",
                   group ? group->gr_name : "unknown",
                   (long)info.st_size,
                   time_string,
                   filenames[i]);
        }
    }
    else if (horizontal_listing)
    {
        print_horizontal(filenames, count, max_length);
    }
    else
    {
        print_columns(filenames, count, max_length);
    }

    for (size_t i = 0; i < count; i++)
        free(filenames[i]);

    free(filenames);

    if (read_error)
        fprintf(stderr, "Warning: listing may be incomplete.\n");
}