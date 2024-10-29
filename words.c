#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

//new and experimenting
#include <sys/stat.h>
#include <unistd.h>

//  just place holder for me to remember
// struct dirent {
//     ino_t          d_ino;       /* inode number */
//     off_t          d_off;       /* offset to the next dirent */
//     unsigned short d_reclen;    /* length of this record */
//     unsigned char  d_type;      /* type of file; not supported
//                                    by all file system types */
//     char           d_name[256]; /* filename */
// };

int isFile(const char* name)
{
    DIR* directory = opendir(name);

    if(directory != NULL)
    {
     closedir(directory);
     //is a directory
     return 0;
    }
    //is a file
    return 1;
}

void search_directory(const char *dir_path) {
    struct dirent *entry;
    DIR *dp = opendir(dir_path);

    if (dp == NULL) {
        return;
    }

    while ((entry = readdir(dp)) != NULL) {
        // skip the . and .. entries
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            return;
        }

        char full_path[1024];
        //grab the full directory name---------dont know if this works entirely
        strcat(dir_path, entry->d_name);
        

        // check if the entry is a directory
        //for some reason DT_DIR and DT_REG not working for me so might look for another way to do this
        if (isFile(dir_path) == 0) {
            search_directory(full_path);
            
        // Check if the file ends with ".txt"
        } else if (strstr(dir_path, ".txt") != NULL) {
            // call wordcount or add the path to a place to all execute at once
        }
    }

    closedir(dp);
}

int main(){
    
}