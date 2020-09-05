#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]){
	char last;
    char curr;
    int count;

    
    if(argc < 2){
        printf("wzip: file1 [file2 ...]\n");
        exit(1);
    }

    for(int i = 1; i < argc; i++){

        FILE *fp = fopen(argv[i], "r");

        last = 0;
        curr = 0;
        count = 0;


        if(fp == NULL){
            printf("[%s] file does not exist, exiting program.. \n", argv[i]);
            exit(1);
        }

        while(1){
            curr = fgetc(fp);
            if(curr == EOF){
                if(count > 0){
                    fwrite(&count, 1, 4, stdout);
                    fwrite(&last, 1, 1, stdout);
                }
                break;
                
            }
            if(curr != last){
                if(count > 0){
                    fwrite(&count, 1, 4, stdout);
                    fwrite(&last, 1, 1, stdout);
                }
                count = 0;
            }

            last = curr;
            count++;
            
        }

        fclose(fp);

    }

    return 0;
}