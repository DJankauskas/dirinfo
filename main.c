#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <dirent.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>

//used to indicate directories
#define GREEN_BACKGROUND "\e[1;42m"
#define CLEAR_BACKGROUND "\e[0m"

size_t print_entry(struct dirent *entry, char *parent_dir);
void print_perms(int perms);
size_t get_file_size(struct stat *file, char *name, char *parent_dir);
void print_file_size(size_t size);
char *concat(char *a, char *b, char *c);

int main(int argc, char **argv)
{
    if (argc > 2)
    {
        fprintf(stderr, "Usage: ./dirinfo.out or ./dirinfo <directory path>");
    }

    char *directory;
    char user_input[256 * 16];
    if (argc == 2)
    {
        size_t len = strlen(argv[1]);
        directory = argv[1];
    }
    else
    {
        printf("Please enter a directory: ");
        fgets(user_input, sizeof user_input, stdin);
        printf("\n");
        size_t len_input = strlen(user_input);
        for (int i = 0; i <= len_input; i++)
        {
            if (user_input[i] == '\n')
            {
                user_input[i] = '\0';
                break;
            }
        }
        directory = user_input;
    }

    size_t dir_len = strlen(directory);
    if (directory[dir_len - 1] != '/')
    {
        char *new_directory = malloc(dir_len + 2);
        strcpy(new_directory, directory);
        new_directory[dir_len] = '/';
        directory = new_directory;
    }

    DIR *dir_handle = opendir(directory);

    if (!dir_handle)
    {
        fprintf(stderr, "Error %d: %s.\n", errno, strerror(errno));
        return 1;
    }

    size_t total_size = 0;

    for (struct dirent *dir_entry; dir_entry = readdir(dir_handle); dir_entry != NULL)
    {
        total_size += print_entry(dir_entry, directory);
    }

    printf("Total, recursive directory size: ");
    print_file_size(total_size);
    printf("\n");

    return 0;
}

size_t print_entry(struct dirent *entry, char *parent_dir)
{
    struct stat entry_info;
    char *entry_path = concat(parent_dir, entry->d_name, "");
    if (stat(entry_path, &entry_info) < 0)
    {
        print_perms(0);
        printf("\t%s\tError %d: %s\n", entry->d_name, errno, strerror(errno));
        return 0;
    }
    free(entry_path);
    entry_path = NULL;

    print_perms(entry_info.st_mode);
    printf("\t%s%s%s\t", S_ISDIR(entry_info.st_mode) ? GREEN_BACKGROUND : "", entry->d_name, CLEAR_BACKGROUND);
    size_t size = get_file_size(&entry_info, entry->d_name, parent_dir);
    print_file_size(size);

    printf("\t%s\n", ctime(&entry_info.st_atime));
    return size;
}

void print_file_size(size_t size)
{
    if (size == -1)
    {
        printf("----");
    }
    else if (size >= 1000000000)
    {
        printf("%.2lfGB", size / 1000000000.0);
    }
    else if (size >= 1000000)
    {
        printf("%.2lfMB", size / 1000000.0);
    }
    else if (size >= 1000)
    {
        printf("%.2lfKB", size / 1000.0);
    }
    else
    {
        printf("%ldB", size);
    }
}

//return should be freed by caller when done
char *concat(char *a, char *b, char *c)
{
    size_t len_a = strlen(a);
    size_t len_b = strlen(b);
    size_t len_c = strlen(c);

    char *new_str = malloc(len_a + len_b + len_c + 1);
    strcpy(new_str, a);
    strcpy(new_str + len_a, b);
    strcpy(new_str + len_a + len_b, c);

    return new_str;
}

size_t get_file_size(struct stat *file, char *name, char *parent_dir)
{
    //ignore current dir and parent dir
    if ((strcmp(name, ".") == 0 || strcmp(name, "..") == 0))
    {
        return 0;
    }

    if (!S_ISDIR(file->st_mode) || (strcmp(name, ".") == 0 || strcmp(name, "..") == 0))
    {
        return file->st_size;
    }
    char *path = concat(parent_dir, name, "/");

    DIR *dir_handle = opendir(path);

    if (!dir_handle)
    {
        fprintf(stderr, "In get_file_size reading %s- rror %d: %s.\n", path, errno, strerror(errno));
        return 1;
    }

    size_t dir_contents = 4096;

    for (struct dirent *dir_entry; dir_entry = readdir(dir_handle); dir_entry != NULL)
    {

        struct stat entry_info;
        char *item_path = concat(path, dir_entry->d_name, "");
        if (stat(item_path, &entry_info) < 0)
        {
            printf("File size of %s in path %s - error %d: %s\n", dir_entry->d_name, path, errno, strerror(errno));
            break;
        }
        dir_contents += get_file_size(&entry_info, dir_entry->d_name, path);
        free(item_path);
    }

    closedir(dir_handle);
    free(path);
    path = NULL;

    return dir_contents;
}

void print_perm(int perm)
{
    printf("%c%c%c", perm & 4 ? 'r' : '-', perm & 2 ? 'w' : '-', perm & 1 ? 'x' : '-');
}

void print_perms(int perms)
{
    int others_perms = perms & 7;       //7 == 0b000000111
    int group_perms = perms & 56 >> 3;  //56 == 0b000111000
    int owner_perms = perms & 448 >> 6; //448 == 0b111000000

    printf("-");
    print_perm(owner_perms);
    print_perm(group_perms);
    print_perm(others_perms);
}
