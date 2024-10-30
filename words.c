#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#define BUFSIZE 128
#ifndef DEBUG
#define DEBUG 0
#endif

int get_entry_type(const char* path);
void search_directory(char *dir_path);
void read_text(char *dir_path);
void count(char *dir_path);

int get_entry_type(const char *path) {
    struct stat sbuf;
    int r = stat(path, &sbuf);

    if (r != 0) perror(path);

    // 1 = regular file
    // 2 = directory
    if (S_ISREG(sbuf.st_mode)) {
        if (DEBUG) printf("%s is a regular file\n", path);
        return 1;
    } else if (S_ISDIR(sbuf.st_mode)) {
        if (DEBUG) printf("%s is a directory\n", path);
        return 2;
    } else
        return -1;
}

/**
 * @brief Recursively traverse through directory
 * 
 * @param [in] dir_path directory to search through
 */
void search_directory(char *dir_path) {
    struct dirent *entry;
    DIR *dp = opendir(dir_path);

    if (dp == NULL) {
        return;
    }

    while ((entry = readdir(dp)) != NULL) {
        // Skip the . and .. entries
        if (strncmp(entry->d_name, ".", 1) == 0) {
            continue;
        }

        char temp[1024];
        strcpy(temp, dir_path);
        strcat(temp, "/");
        strcat(temp, entry->d_name);

        int entry_type = get_entry_type(temp);

        if (entry_type == 2) {
            search_directory(temp);
        } else if (entry_type == 1) {
            if (strstr(entry->d_name, ".txt")) {
                read_text(temp);
            }
        }
    }
    closedir(dp);
}

void read_text(char *dir_path) {
    int fd = open(dir_path, O_RDONLY);

    if (fd < 0) {
        perror(dir_path);
        return;
    }

    char buf[BUFSIZE];
    char *line = NULL;
    int line_len = 0;
    int bytes, seg_start;

    while ((bytes = read(fd, buf, BUFSIZE)) > 0) {
        if (DEBUG) printf("read %d bytes\n", bytes);
        
        int pos;
        seg_start = 0;
        for (pos = 0; pos < bytes; pos++) {
            if (buf[pos] == '\n') {
                int seg_len = pos - seg_start;
                line = realloc(line, line_len + seg_len + 1);
                memcpy(line + line_len, buf + seg_start, seg_len);
                
                line[line_len + seg_len] = '\0';

                count(line);

                seg_start = pos + 1;
                line = NULL;
                line_len = 0;
            }
        }
        if (seg_start < pos) {
            int seg_len = pos - seg_start;
            line = realloc(line, line_len + seg_len + 1);
            
            memcpy(line + line_len, buf + seg_start, seg_len);
            line_len += seg_len;
            line[line_len] = '\0';
        }
    }
    if (line) {
        count(line);
    }
}

void count(char *text) {
    
}

int main(int argc, char **argv) {
    search_directory(argv[1]);
    return EXIT_SUCCESS;
}