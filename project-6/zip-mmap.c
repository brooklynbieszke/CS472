#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>

#pragma pack(1)
typedef struct results{
    int count;
    char character;
}myResults;

int main(int argc, char *argv[]){

    char character = '\0';
    int count = 0;
    int size = 10;
    int index = 0;

    if (argc < 2)
    {
        printf("pzip: file1 [file2 ...]\n");
        exit(1);
    }

    myResults *buffer = malloc(size * sizeof(myResults));

    int max = argc - 1;
    for(int i = 1; i <= max; i++){

        //Declarations
        int fd;
        struct stat sb;
        fd = open(argv[i], O_RDONLY);

        //Checking for errors
        if(fd == -1){
            close(fd);
            perror("open"); 
            exit(1);
        }
        if(fstat(fd, &sb) == -1){
            close(fd);
            perror("fstat");
            exit(1);
        }
        int length = sb.st_size;
        if(length == 0){
            close(fd);
            continue;
        }

        //Mapping
        char *address = mmap(0, length, PROT_READ, MAP_PRIVATE, fd, 0);

        //Checks if map failed
        if(address == MAP_FAILED){
            close(fd);
            perror("mmap");
            exit(1);
        }

        if(character == '\0'){
            character = address[0];
        }

        for(int i = 0; i < length; i++){
            char temp = address[i];

            if(temp == character){
                count++;
            }else{
                if(i > size){
                    size *= 2;
                    buffer = realloc(buffer, (size * sizeof(myResults)));
                }

                myResults myResults;  
                myResults.count = count;
                myResults.character = character;

                buffer[index] = myResults;
                character = temp;
                count = 1;
                index++;

            }

        }

        //closing anthing that is open
        close(fd);
        munmap(address, length);
    }

    myResults myResults;  
    myResults.count = count;
    myResults.character = character;

    buffer[index] = myResults;
    index++;

    if(write(STDOUT_FILENO, buffer, index*sizeof(myResults)) !=-1);
    free(buffer);

    return 0;
}
