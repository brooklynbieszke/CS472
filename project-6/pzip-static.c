#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <sys/sysinfo.h>
#pragma pack(1)

//Part 1 added
typedef struct myResults {
    int count;
    char character;
} myResults;

//part 2 added
typedef struct zip_job {
	char *read_buffer;
	int read_size;
	myResults *result_buffer;
	int result_size;
} zip_job;

//part 2 added
typedef struct zip_queue {
	zip_job *head;
	zip_job *tail;
	pthread_mutex_t headLock; 
	pthread_mutex_t tailLock;
} zip_queue;


// Function Definitions
void makeResult(int count, char character, myResults *buffer, int index) {
	myResults result;	
	result.count = count;
	result.character = character;
	buffer[index] = result;
}

void *run_job(void *arg) {
	
	zip_job *zip = (zip_job*)arg;
	char *address = zip->read_buffer;
	int length = zip->read_size;
	char character = address[0];
	int count = 0;
	int size = 10;
	int index = 0;	

	myResults *buffer = malloc(size * sizeof(myResults));

	for(int i = 0; i < length; i++) {
		char temp = address[i];

		if(temp == character) {
			count++;
		} else {
			if(i > size) {
				size *= 2;
				buffer = realloc(buffer, (size * sizeof(myResults)));
			}
			makeResult(count, character, buffer, index);
			
			character = temp;
			count = 1;
			index++;
		}
	}

	makeResult(count, character, buffer, index);
	index++;

	zip->result_buffer = buffer;
	zip->result_size = index;
	return NULL;
}

void makeResult(int count, char character, myResults *buffer, int index);
void *run_job(void *arg);

int main(int argc, char** argv) {

	int numProcs = get_nprocs();

	if(argc < 2) {
		printf("pzip: file1 [file2 ...]\n");
		exit(1);
	}

	myResults *buffer = NULL;
	int index = 0;

	int max = argc - 1;
	int fd;
	struct stat sb;

	for(int i = 1; i <= max; i++) {

		fd = open(argv[i], O_RDONLY);

		//error checking part 1
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

        //mapping part 1
		char *address = mmap(NULL, length, PROT_READ, MAP_PRIVATE, fd, 0);

		//checks if map failed part 1
		close(fd);
		if(address == MAP_FAILED) {
			close(fd);
			perror("mmap");
			exit(1);
		}
		
		//create job, threads, and job description 
		//part 2
		zip_job* tempJob;
		tempJob = malloc(sizeof(zip_job) * numProcs);
		pthread_t *threads;
		threads = malloc(sizeof(pthread_t) * numProcs);
		int new_length = 0;

		for(int i = 0; i < numProcs; i++) {
			int block = length / numProcs;
			if(i == 0) {
				block += length % numProcs;
			}
			tempJob[i].read_buffer = address + new_length;
			tempJob[i].read_size = block;
			new_length += block;
			pthread_create(threads + i, NULL, run_job, tempJob + i);
		}

		//wait for threads to finish
		//part 26
		for (int i = 0; i < numProcs; i++) {
			pthread_join(threads[i], NULL);
			if(buffer != NULL) {
				if(buffer[index - 1].character == tempJob[i].result_buffer[0].character) {
					tempJob[i].result_buffer[0].count += buffer[index - 1].count;
					index--;
				}
			}
			int freshIndex = tempJob[i].result_size + index;
			buffer = realloc(buffer, (freshIndex * sizeof(myResults)));
			memcpy(buffer + index, tempJob[i].result_buffer, tempJob[i].result_size * sizeof(myResults));
			index = freshIndex;
			free(tempJob[i].result_buffer);
		}

		free(threads);
		free(tempJob);
		
		munmap(address, length);
	}
	
	if(write(STDOUT_FILENO, buffer, index * sizeof(myResults)) != -1);
	free(buffer);
	return 0;
}