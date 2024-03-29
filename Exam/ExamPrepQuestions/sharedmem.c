#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <ctype.h>

#define SHM_SIZE 1024

int main() {
    int fd = shm_open("/my_shm", O_CREAT | O_RDWR, 0666); // Create a shared memory object
    ftruncate(fd, SHM_SIZE); // Set the size of the shared memory object

    char *shmaddr = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0); // Map the shared memory object into the address space

    printf("Enter a string: ");
    fgets(shmaddr, SHM_SIZE, stdin); // Read the string into shared memory

    pid_t pid;
    int i;
    for (i = 0; i < 3; i++) {
        pid = fork();
        if (pid == 0) {
            // Child process
            switch (i) {
                case 0:
                    // Convert the string to lowercase
                    for (int i = 0; shmaddr[i]; i++) {
                        shmaddr[i] = tolower(shmaddr[i]);
                    }
                    break;
                case 1:
                    // Count the number of characters
                    printf("Child process %d: Number of characters = %ld\n", getpid(), strlen(shmaddr)-1);
                    break;
                case 2:
                    // Count the number of lines
                    int lines = 0; // At least one line
                    for (char *p = shmaddr; *p; p++) {
                        if (*p == '\n') {
                            lines++;
                        }
                    }
                    printf("Child process %d: Number of lines = %d\n", getpid(), lines);
                    break;
            }
            exit(0);
        }
    }

    // Wait for all child processes to finish
    for (i = 0; i < 3; i++) {
        wait(NULL);
    }

    printf("Converted string: %s\n", shmaddr);

    pid_t word_count = fork();
    if (word_count == 0){
        int words = 0;
        for (int i = 0 ; shmaddr[i] ; i++){
            if (isspace(shmaddr[i])){
                words++;
            }
        }
        printf("The sentence has %d words\n", words);
        return 0;
    }

    // Unmap the shared memory object
    munmap(shmaddr, SHM_SIZE);

    // Close the shared memory object
    close(fd);

    // Remove the shared memory object
    shm_unlink("/my_shm");

    return 0;
}
