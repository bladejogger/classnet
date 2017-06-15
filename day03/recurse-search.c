#define _BSD_SOURCE
#define _XOPEN_SOURCE

#include <dirent.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>

void walk_dir(char *needle, char *path);
void search_file(char *needle, char *filename);

static FILE *fp = NULL;
static DIR *dir = NULL;

void file_cleanup(int sig)
{
    if(fp) {
        fclose(fp);
    }
    if(dir) {
        closedir(dir);
    }
    exit(0);
}

int main(int argc, char *argv[])
{
    if(argc != 3) {
        fprintf(stderr, "Usage: %s <path> <needle>\n", argv[0]);
        return 1;
    }

    struct sigaction close_handler = {0};
    close_handler.sa_handler = file_cleanup;
    sigaction(SIGTERM, &close_handler, NULL);

    walk_dir(argv[2], argv[1]);
}

void walk_dir(char *needle, char *path)
{
    dir = opendir(path);
    if(!dir) {
        perror("Could not open path");
        return;
    }

    struct dirent *entry;
    while((entry = readdir(dir))) {
        if(strcmp(entry->d_name, ".") == 0 ||
                strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        char buf[PATH_MAX];
        snprintf(buf, PATH_MAX, "%s/%s", path, entry->d_name);
        if(entry->d_type == DT_DIR) {
            if(fork() == 0) {
                closedir(dir);
                dir = NULL;
                walk_dir(needle, buf);
                exit(0);
            }
        } else if(entry->d_type == DT_REG) {
            if(fork() == 0) {
                closedir(dir);
                dir = NULL;
                search_file(needle, buf);
                exit(0);
            }
        }
    }
    closedir(dir);
}

void search_file(char *needle, char *filename)
{

    fp = fopen(filename, "r");
    if(!fp) {
        return;
    }

    // Picked an arbitrary, non-thought-out value for a buffer
    char buf[512];

    while(fgets(buf, sizeof(buf), fp)) {
        // TODO: Handle needle spanning two fgets calls
        // To fix, allocate a buffer that is 10*strlen(needle),
        // and after each fgets call, rewind the fp by strlen(needle)
        if(strstr(buf, needle)) {
            printf("%s contains %s\n", filename, needle);
            fclose(fp);
            fp = NULL;
            kill(0, SIGTERM);
            exit(0);
        }
    }

    fclose(fp);
}

