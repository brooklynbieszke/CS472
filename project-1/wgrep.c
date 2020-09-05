#include <stdio.h>
#include <stdlib.h>    
#include <string.h>    

int main(int argc, char *argv[])
{
    FILE *fp = NULL;
    char *curr = NULL;
    size_t count = 0;

    if (argc <= 1) {
        printf("wgrep: searchterm [file ...]\n");
        exit(0);
    }

    if (argc >= 3 && (fp = fopen(argv[2], "r")) == NULL) {
        printf("wgrep: cannot open file\n");
        exit(0);
    }

    if (argc == 2)
        fp = stdin;

    while (getline(&curr, &count, fp) > 0)
        if (strstr(curr, argv[1]))
            printf("%s", curr);

    free(curr);
    fclose(fp);

    return 0;
}