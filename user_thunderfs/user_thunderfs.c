#include <stdlib.h>
#include <stdio.h>
#include <string.h>


struct file_block {
        char *data;
        int used;
        //struct *file_block next_block;
};

struct file_system {
        struct file_block files[100];
        int num_files;
};

ssize_t get_file(char * buf, unsigned long file_id){
        struct file_block myfile;
        printf("FileID: %i\n", (int) file_id);
        myfile.data = "Hello This is My file.\nI like this file because it is mine.\n";
        myfile.used = sizeof("Hello This is My file.\nI like this file because it is mine.\n");

        strncpy(buf, myfile.data, myfile.used);
        return myfile.used;
}





