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

char **words;

int get_entry_type(const char* pathname);
void search_directory(const char *pathname);
void read_text(const char *pathname);
void count(char *text);

int get_entry_type(const char *pathname) {
    struct stat sbuf;
    int r = stat(pathname, &sbuf);

    if (r != 0) perror(pathname);

    // 1 = regular file
    // 2 = directory
    if (S_ISREG(sbuf.st_mode)) {
        if (DEBUG) printf("%s is a regular file\n", pathname);
        return 1;
    } else if (S_ISDIR(sbuf.st_mode)) {
        if (DEBUG) printf("%s is a directory\n", pathname);
        return 2;
    } else
        return -1;
}

/**
 * @brief Recursively traverse through directory
 * 
 * @param [in] dir_path directory to search through
 */
void search_directory(const char *pathname) {
    struct dirent *entry;
    DIR *dp = opendir(pathname);

    if (dp == NULL) {
        return;
    }

    while ((entry = readdir(dp)) != NULL) {
        // Skip the . and .. entries
        if (strncmp(entry->d_name, ".", 1) == 0) {
            continue;
        }

        char temp[1024];
        strcpy(temp, pathname);
        strcat(temp, "/");
        strcat(temp, entry->d_name);

        int entry_type = get_entry_type(temp);

        if (entry_type == 2)
            search_directory(temp);
        else if (entry_type == 1)
            if (strstr(entry->d_name, ".txt"))
                read_text(temp);
    }
    closedir(dp);
}

void read_text(const char *pathname) {
    int fd = open(pathname, O_RDONLY);

    if (fd < 0) {
        perror(pathname);
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
    // word delimiters
        // ' can occur anywhere in a word
        // - must be sandwiched by letters
            // "-foo" and "foo-" are considered "foo", "foo--bar" would be "foo" and "bar"
        // numbers, whitespace, and other punctuation are delimiters
            // "a2z" is considered "a" and "z"
}

int main(int argc, char **argv) {
    search_directory(argv[1]);
    return EXIT_SUCCESS;
}