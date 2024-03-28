#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

// Write (Parent) -> Read (Child 1)
//         (First Pipe)
//          Write (Child 1) -> Read (Child 2)
//         (Second Pipe)

// Parent Process           Child Process 1          Child Process 2
// --------------------     ---------------------     ---------------------
// | Create Pipes         | | Read Input String     | | Read Converted String |
// | Write Input String  | | Convert to Lowercase | | Count Words             |
// | Wait for Children     | | Write Converted String| | Print Word Count     |
// --------------------     ---------------------     ---------------------

int main() {
    int first_pipe[2];
    int second_pipe[2];
    int third_pipe[2];
    char buffer[2560];

    // Create two pipes
    if (pipe(first_pipe) == -1) {
        perror("couldn't create first pipe");
        return 1;
    }

    if (pipe(second_pipe) == -1) {
        perror("couldn't create second pipe");
        return 1;
    }


    if (pipe(third_pipe)== -1){
        perror("couldn't create third pipe");
        return 1;
    }

    // Fork a child process to handle converting the input string to lowercase
    pid_t first_child = fork();

    if (first_child == 0) {
        // First child process
        close(first_pipe[1]); // Close the write side

        // Read from the read side of the first pipe into the buffer
        read(first_pipe[0], buffer, sizeof(buffer));

        printf("OG string: %s \n", buffer);

        // Convert the string in buffer to lowercase
        for (int i = 0; i < 2560; i++) {
            buffer[i] = tolower(buffer[i]);
        }
        printf("Converted string: %s\n", buffer);

        // Close the read side of the first pipe
        close(first_pipe[0]);

        // Write the converted string to the write side of the second pipe
        close(second_pipe[0]); // Close the read side
        write(second_pipe[1], buffer, sizeof(buffer));
        close(second_pipe[1]); // Close the write side

        return 0;
    }

    // Fork another child process to count the number of words in the converted string
    pid_t second_child = fork();

    if (second_child == 0) {
        // Second child process
        close(second_pipe[1]); // Close the write side

        // Read from the read side of the second pipe into the buffer
        read(second_pipe[0], buffer, sizeof(buffer));

        int num_words = 0;
        for (int i = 0; i < strlen(buffer); i++) {
            if (isspace(buffer[i])) {
                num_words++;
            }
        }

        printf("The number of words: %d\n", num_words);
        // Close the read side of the second pipe
        close(second_pipe[0]);

        // while the current string into the write side of the third pipe
        close(third_pipe[0]);
        write(third_pipe[1], buffer, sizeof(buffer));
        close(third_pipe[1]);

        return 0;
    }

    pid_t third_child = fork();

    if (third_child == 0){

        close(third_pipe[1]);
        read(third_pipe[0], buffer, sizeof(buffer));

        printf("The length of the string is %ld\n", strlen(buffer));

        close(third_pipe[0]);
        
        return 0; // IF WE DIDN'T RETURN 0 IT WOULDN'T PRINT
    }

    // Parent process
    close(first_pipe[0]); // Close the read side

    // Write the input string to the write side of the first pipe
    char input_string[] = "Hello World! This is a test string.";
    write(first_pipe[1], input_string, sizeof(input_string));

    close(first_pipe[1]); // Close the write side

    // Wait for both child processes to finish
    wait(NULL);
    wait(NULL);

   return 0;
}
