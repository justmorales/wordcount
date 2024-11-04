#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>

/* PREPROCESSOR DIRECTIVES */
#define BUFSIZE 128
#ifndef DEBUG
#define DEBUG 0
#endif

/* FUNCTION DECLARATIONS */
int get_entry_type(const char* pathname);
void search_directory(const char *pathname);
void read_text(const char *pathname);
int has_double_dash(char *text);
void add_to_words(char *text);
void to_lowercase(char *str);

/* GLOBALS */
char *delimiters = "`1234567890=~!@#$%^&*()_+[]{}\\|:;\",./<>?";
char **words;
int total_word_count;   

//creating a struct to count the # of a word found
typedef struct {
    char *word;
    int count;
} WordCount;

// finding the word in the array given after the 
int findWord(WordCount words[], int size, char *word) {
    for (int i = 0; i < size; i++) {
        if (strcmp(words[i].word, word) == 0) {
            return i;
        }
    }
    return -1;
}

// counting up the words in the array
void countWords(char **input, int size) {
    WordCount words[total_word_count];
    int uniqueWords = 0;

    for (int i = 0; i < size; i++) {
        to_lowercase(input[i]);  // Convert to lowercase before finding or adding
        int index = findWord(words, uniqueWords, input[i]);

        if (index != -1) {
            words[index].count++;  // Increase the count if word already exists
        } else {
            words[uniqueWords].word = malloc(strlen(input[i]) + 1);  
            if (words[uniqueWords].word == NULL) {
                fprintf(stderr, "Memory allocation failed\n");
                exit(1);
            }
            strcpy(words[uniqueWords].word, input[i]);
            words[uniqueWords].count = 1;
            uniqueWords++;
        }
    }

    printf("Word Counts:\n");
    for (int i = 0; i < uniqueWords; i++) {
        printf("%s: %d\n", words[i].word, words[i].count);
        free(words[i].word);  // Free allocated memory for each word
    }
}

//check if its a regular file or directory
int get_entry_type(const char *pathname) {
    struct stat sbuf;
    int r = stat(pathname, &sbuf);

    if (r != 0) perror(pathname);

    // 1 = Regular File
    // 2 = Directory
    if (S_ISREG(sbuf.st_mode)) {
        if (DEBUG) printf("get_entry_type: %s is a regular file\n", pathname);
        return 1;
    } else if (S_ISDIR(sbuf.st_mode)) {
        if (DEBUG) printf("get_entry_type: %s is a directory\n", pathname);
        return 2;
    } else
        return -1;
}

void search_directory(const char *pathname) {
    struct dirent *entry;
    DIR *dp = opendir(pathname);

    if (dp == NULL) {
        return;
    }

    // traverse through entire directory
    while ((entry = readdir(dp)) != NULL) {
        //exclude entries starting with .
        if (strncmp(entry->d_name, ".", 1) == 0) {
            continue;
        }

        //store full pathname in temp string
        char temp[1024];
        strcpy(temp, pathname);
        strcat(temp, "/");
        strcat(temp, entry->d_name);

        int entry_type = get_entry_type(temp);
        
        // If entry is a directory, search that subdirectory
        if (entry_type == 2)
            search_directory(temp);
        // If entry is a regular file, check if it's a text file
        else if (entry_type == 1 && strstr(entry->d_name, ".txt"))
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

    // Repeatedly fill the buffer until nothing is left to read
    while ((bytes = read(fd, buf, BUFSIZE)) > 0) {
        if (DEBUG) printf("read_text: read %d bytes\n", bytes);
        
        int pos;
        seg_start = 0;

        // Iterate through the buffer
        for (pos = 0; pos < bytes; pos++) {
            // Cut buffer at whitespace
            if (buf[pos] == '\n' || buf[pos] == '\t' || buf[pos] == ' ') {
                // Copy char bytes from current segment
                int seg_len = pos - seg_start;
                line = realloc(line, line_len + seg_len + 1);
                memcpy(line + line_len, buf + seg_start, seg_len);
                
                line[line_len + seg_len] = '\0';

                add_to_words(line);

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
    // Check for leftover characters in line
    if (line) {
        add_to_words(line);
    }
    free(line);
}

int has_double_dash(char *text) {
    char *pos = strstr(text, "--");

    if (pos) {
        size_t len1 = pos - text;
        char str1[len1];
        char str2[strlen(text) - len1 - 1];
        strncpy(str1, text , len1);
        str1[len1] = '\0';
        // 2 = strlen("--")
        strcpy(str2, pos + 2);

        add_to_words(str1);
        add_to_words(str2);

        return 1;
    }
    
    return 0;
}

void add_to_words(char *text) {
    if (DEBUG) printf("add_to_words: %s\n", text);

    // Word delimiters:
        // ' can occur anywhere in a word
        // - must be sandwiched by letters
            // "-foo" and "foo-" are considered "foo", "foo--bar" would be "foo" and "bar"
        // numbers, whitespace, and other punctuation are delimiters
            // "a2z" is considered "a" and "z"
    
    // Check for "--" delimiter

    if (has_double_dash(text)) return;

    // Check if string begins or ends with a "-" and remove it
    if (text[0] == '-')
        text++;
    else if (text[strlen(text)-1] == '-')
        text[strlen(text)-1] = '\0';
    
    char *token = NULL;

    token = strtok(text, delimiters);

    while (token != NULL) {
        int len = strlen(token);

        // Convert each token to lowercase before storing
        to_lowercase(token);  
        // Dynamic sizing array...
        words = realloc(words, sizeof(char *) * total_word_count + 2);
        words[total_word_count] = malloc(len + 1);

        strcpy(words[total_word_count], token);
        total_word_count++;

        token = strtok(NULL, delimiters);
    }
}

void to_lowercase(char *str) {
    for (int i = 0; str[i]; i++) {
        str[i] = tolower(str[i]);
    }
}

int main(int argc, char **argv) {
    words = NULL;
    total_word_count = 0;

    search_directory(argv[1]);
    countWords(words, total_word_count);

    for (int i = 0; i < total_word_count; i++) {
        printf("%s\n", words[i]);
    }

    free(words);
    return EXIT_SUCCESS;
}
