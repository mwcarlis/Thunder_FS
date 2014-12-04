#include <stdlib.h>
#include <stdio.h>
#include <string.h>


struct file_block {
        char data[4096];
        unsigned long file_id;
        unsigned long used;
        //struct *file_block next_block;
};

#define NUM_FILES 10
struct file_system {
        struct file_block files[NUM_FILES];
        int num_files;
};

void init_filesystem(struct file_system *system){
        system->num_files = 0;
}

unsigned long open_file(struct file_system *system, unsigned long this_fid){
        int i;
        for(i=0; i < system->num_files; i++){
                if(system->files[i].file_id == this_fid ){
                        printf("Opening[%i]: %lu\n", i, this_fid);
                        return system->files[i].used;
                }
        }
        system->files[system->num_files].file_id = this_fid;
        system->files[system->num_files].used = 0;
        printf("Making File[%i] %lu\n", system->num_files, this_fid);
        system->num_files += 1;
        return 0;
}

unsigned long get_file(struct file_system *system, char * buf, unsigned long this_fid){
        int i;
        for(i=0; i < system->num_files; i++){
                if(system->files[i].file_id == this_fid ){
                        strncpy(buf, system->files[i].data, system->files[i].used);
                        printf("getf [%i]:\n%s", i, buf);
                        return system->files[i].used;
                }
        }
        return 0;
}

unsigned long write_file(struct file_system *system, char *data, 
                                unsigned long data_size, unsigned long this_fid){
        int i;
        for(i=0; i < system->num_files; i++){
                if(system->files[i].file_id == this_fid ){
                        strncpy(system->files[i].data, data, data_size);
                        system->files[i].used = data_size;
                        printf("Writing to file[%i]:\n%s", i, system->files[i].data);
                        return system->files[i].used;
                }
        }
        return 0;
}





