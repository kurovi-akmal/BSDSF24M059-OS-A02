/*
* Programming Assignment 02: lsv1.0.0
* This is the source file of version 1.0.0
* Read the write-up of the assignment to add the features to this base version
* Usage:
*       $ lsv1.0.0 
*       % lsv1.0.0  /home
*       $ lsv1.0.0  /home/kali/   /etc/
*/
#include <stdio.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>

extern int errno;

int long_listing = 0;

void do_ls(const char *dir);

void get_permissions(mode_t mode, char permissions[11]);


int main(int argc, char *argv[])
{
    int option;

    while ((option = getopt(argc, argv, "l")) != -1)
    {
        switch (option)
        {
            case 'l':
                long_listing = 1;
                break;

            default:
                fprintf(stderr, "Usage: %s [-l] [directory ...]\n",
                        argv[0]);
                return 1;
        }
    }

    if (optind == argc)
    {
        do_ls(".");
    }
    else
    {
        for (int i = optind; i < argc; i++)
        {
            printf("Directory listing of %s : \n", argv[i]);
            do_ls(argv[i]);
            puts("");
        }
    }

    return 0;
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

    /* Handle special permission bits */
    if (mode & S_ISUID)
        permissions[3] = (mode & S_IXUSR) ? 's' : 'S';

    if (mode & S_ISGID)
        permissions[6] = (mode & S_IXGRP) ? 's' : 'S';

    if (mode & S_ISVTX)
        permissions[9] = (mode & S_IXOTH) ? 't' : 'T';

    permissions[10] = '\0';
}



void do_ls(const char *dir)
{
    struct dirent *entry;
    DIR *dp = opendir(dir);

    if (dp == NULL)
    {
        fprintf(stderr, "Cannot open directory: %s\n", dir);
        return;
    }

    errno = 0;

    while ((entry = readdir(dp)) != NULL)
    {
        if (entry->d_name[0] == '.')
            continue;

        if (long_listing)
        {
            struct stat info;
            char permissions[11];

            char path[4096];
            snprintf(path, sizeof(path), "%s/%s",
                     dir, entry->d_name);

            if (stat(path, &info) == -1)
            {
                perror(path);
                continue;
            }

            
            struct passwd *owner = getpwuid(info.st_uid);get_permissions(info.st_mode, permissions);

            printf("%s %s\n", permissions, entry->d_name);
            struct group *group = getgrgid(info.st_gid);

            char time_buffer[32];
            struct tm *time_info = localtime(&info.st_mtime);

            if (time_info != NULL)
            {
                strftime(time_buffer, sizeof(time_buffer),
                         "%b %d %H:%M", time_info);
            }
            else
            {
                snprintf(time_buffer, sizeof(time_buffer), "Unknown");
            }

            get_permissions(info.st_mode, permissions);

            printf("%s %2lu %-8s %-8s %8ld %s %s\n",
                   permissions,
                   (unsigned long)info.st_nlink,
                   owner ? owner->pw_name : "unknown",
                   group ? group->gr_name : "unknown",
                   (long)info.st_size,
                   time_buffer,
                   entry->d_name);

                    }
                    else
                    {
                        printf("%s\n", entry->d_name);
                    }
                }

    if (errno != 0)
        perror("readdir failed");

    closedir(dp);
}
