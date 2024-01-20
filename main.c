#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <pthread.h>
#include<assert.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <fcntl.h>


/*
This is my main function. This function uses the commands forks() and execvp().
The fork() command is used to split the process into the child and the parent.
the execvp() tranforms the current running process into a different running process.
the parent wait for child to finish by using the wait() function.
the execvp() takes in two parameters first is the executable file, "./child" and then myargs.
the myargs is an array of 3 elements the executable file and the fd which is converted to string before
being put in the myargs array. 
*/

int main(int argc, char **argv) {

    // plant manager
    printf("Number of arguments = %d\n", argc);
    int fd = open("railwayCars.txt",  O_RDONLY); 

    int rc = fork();
    if (rc < 0) {
        perror("Fork failed");
    } else if (rc == 0) {

        char fdStr[16]; // A buffer to hold the fd as a string
        sprintf(fdStr, "%d", fd); // integer value is going to get stored in fdstr
        char *myargs[3];
        myargs[0] = strdup("./child"); // executable file of child.c
        myargs[1] = strdup(fdStr); //argument: file to count
        myargs[2] = NULL; //end of array
        execvp(myargs[0], myargs);
        //printf("hello, I am child (pid:%d)\n", (int) getpid());
        if (execvp(myargs[0], myargs) == -1) {
            perror("execv() failed");
            exit(EXIT_FAILURE);
        } 
        // child process 
    } else {
        int rc_wait = wait(NULL);
        printf("hello, I am parent of %d (rc_wait:%d) (pid:%d)\n", rc, rc_wait, (int) getpid());
    }
    return 0;

}

