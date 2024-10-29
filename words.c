#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

//  just place holder for me to remember
// struct dirent {
//     ino_t          d_ino;       /* inode number */
//     off_t          d_off;       /* offset to the next dirent */
//     unsigned short d_reclen;    /* length of this record */
//     unsigned char  d_type;      /* type of file; not supported
//                                    by all file system types */
//     char           d_name[256]; /* filename */
// };

void search_directory(const char *dir_path) {
    struct dirent *entry;
    DIR *dp = opendir(dir_path);

    if (dp == NULL) {
        return;
    }

    while ((entry = readdir(dp)) != NULL) {
        // skip the . and .. entries
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        char full_path[1024];
        
        

        // check if the entry is a directory
        //for some reason DT_DIR and DT_REG not working for me so might look for another way to do this
        if (entry->d_type == DT_DIR) {
            search_directory(full_path);
        } else if (entry->d_type == DT_REG) {
            // Check if the file ends with ".txt"
            if (strstr(entry->d_name, ".txt") != NULL) {
                // call wordcount or add the path to a place to all execute at once
            }
        }
    }

    closedir(dp);
}

int main(){
    
}