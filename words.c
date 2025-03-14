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

/* GLOBALS */
char *delimiters = "`1234567890=~!@#$%^&*()_+[]{}\\|:;\",./<>?";
char **words;
int total_word_count;

typedef struct {
    char *word;
    int count;
} WordCount;

/* FUNCTION DECLARATIONS */
int get_entry_type(const char* pathname);
void search(const char *pathname);
void read_text(const char *pathname);
int has_double_dash(char *text);
void add_to_words(char *text);
int find_word(WordCount words[], int size, char *word);
void count_words();


/**
 * @brief Return type of file
 * 
 * @param [in] pathname path to file
 */
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

/**
 * @brief Recursively search through directories
 * 
 * @param [in] pathname path to file to search
*/
void search(const char *pathname) {
    struct dirent *entry;
    DIR *dp = opendir(pathname);

    // If passed argument is a regular file
    if (get_entry_type(pathname) == 1)
        read_text(pathname);

    if (dp == NULL)
        return;

    // Traverse through entire directory
    while ((entry = readdir(dp)) != NULL) {
        // Exclude entries that start with .
        if (strncmp(entry->d_name, ".", 1) == 0) {
            continue;
        }

        // Store full pathname in a temp string
        char temp[1024];
        sprintf(temp, "%s/%s", pathname, entry->d_name);

        int entry_type = get_entry_type(temp);
        // If entry is a directory, search that subdirectory
        if (entry_type == 2)
            search(temp);
        // If entry is a regular file, check if it's a text file
        else if (entry_type == 1)
            if (strstr(entry->d_name, ".txt"))
                read_text(temp);
    }
    closedir(dp);
}

/**
 * @brief Read bytes of a given file
 * 
 * @param [in] pathname path to given file
 */
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

/**
 * @brief Check if string contains "--" substring
 * 
 * @param [in] text string to check
 */
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

/**
 * @brief Splits strings according to delimiters and add them to global words array
 * 
 * @param [in] text line of text to split and add to words
 */
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

        // Dynamic sizing array...
        words = realloc(words, sizeof(char *) * total_word_count + 2);
        words[total_word_count] = malloc(len + 1);

        strcpy(words[total_word_count], token);
        total_word_count++;

        token = strtok(NULL, delimiters);
    }
}

/**
 * @brief Locate index of word in word_counts array
 * 
 * @param [in] word_counts WordCount struct array
 * @param [in] size number of elements in word_counts
 * @param [in] word string to search for in word_counts
 */
int find_word(WordCount word_counts[], int size, char *word) {
    for (int i = 0; i < size; i++) {
        if (strcmp(word_counts[i].word, word) == 0) {
            return i;
        }
    }
    return -1;
}

/**
 * @brief Prints count of each word in words array
 */
void count_words() {
    WordCount word_counts[total_word_count];
    int uniqueWords = 0;

    for (int i = 0; i < total_word_count; i++) {
        int index = find_word(word_counts, uniqueWords, words[i]);

        if (index != -1) {
            // Increase the count if word already exists
            word_counts[index].count++;
        } else {
            word_counts[uniqueWords].word = malloc(strlen(words[i]) + 1);  
            if (word_counts[uniqueWords].word == NULL) {
                fprintf(stderr, "Memory allocation failed\n");
                exit(1);
            }
            strcpy(word_counts[uniqueWords].word, words[i]);
            word_counts[uniqueWords].count = 1;
            uniqueWords++;
        }
    }

    for (int i = 0; i < uniqueWords; i++) {
        printf("%s: %d\n", word_counts[i].word, word_counts[i].count);
        // Free allocated memory for each word
        free(word_counts[i].word);
    }
}

int main(int argc, char **argv) {
    words = NULL;
    total_word_count = 0;

    for (int i = 1; i < argc; i++)
        search(argv[i]);

    count_words(words, total_word_count);

    free(words);
    return EXIT_SUCCESS;
}
