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



#define BLUE_BUFFER_SIZE 15
#define RED_BUFFER_SIZE 10
#define NUM_TO_READ_FOR_THREADL 25
#define NUM_TO_READ_FOR_THREADR 15


void assemblyManager(FILE *filePointer);
void *producerThreadL(void *args);
void *producerThreadR(void *args);
void *consumerX(void *args);
void *consumerY(void *args);
void putBlue(char *value);
void putRed(char *value);
char *getRed();
char *getBlue();
void printArrayBlueBuffer();
void printArrayRedBuffer();
void printArray(char *arr);
void printArrayForThreadR(char *arr);



char *bufferBlue[BLUE_BUFFER_SIZE];
char *bufferRed[RED_BUFFER_SIZE];
int blueCount = 0;
int redCount = 0;
int sequenceNumber = 1;
int shutDown = 0;



// struct to use 

pthread_mutex_t blueMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t redMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t shutdownMutex = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t blueIsEmpty = PTHREAD_COND_INITIALIZER;
pthread_cond_t blueIsFull = PTHREAD_COND_INITIALIZER;
pthread_cond_t redIsEmpty = PTHREAD_COND_INITIALIZER;
pthread_cond_t redIsFull = PTHREAD_COND_INITIALIZER;


/*
Here I receive the paramters of the myargs. 
I store the first argument which was the fd in string and convert it to int. 
Next, I create a FILE pointer to that fd.
And then I pass the FILE pointer to AssemblyManager.

*/

int main(int argc, char *argv[]) {
    int fd = atoi(argv[1]);
    FILE *filePointer = fdopen(fd, "r"); // file stream associated with the file descriptor
    if (filePointer == NULL) {
        perror("Error opening the file");
    }
    assemblyManager(filePointer);
    
}

/*
This function takes value from the file and puts it in blueBuffer and increases buffercount
*/

void putBlue(char *value) {
    //printf("%s\n", "inside putBlue()");

    bufferBlue[blueCount] = value;
    blueCount++;

}

/*
This function returns the firt value of the blueBuffer and shifts all the elements to the left.
Implementing FIFO

*/

char *getBlue() {

    char *value = bufferBlue[0];
    for (int i = 0; i < blueCount - 1; i++) {
        bufferBlue[i] = bufferBlue[i+1]; // shifting the values to left FIFO
    }
    blueCount = blueCount - 1;
    // pthread_cond_signal(&blueNotEmpty);
    // pthread_mutex_unlock(&blueMutex);

    return value;
}

/*
This function puts the value in the bufferRed and increases red count
*/

void putRed(char *value) {
    //printf("%s\n", "inside putRed()");

    bufferRed[redCount] = value;
    redCount++;


}

/*

This function returns the first value in bufferRed, and shifitng the elements to the left
Implementing FIFO
*/

char *getRed() {

    char *value = bufferRed[0];
    for (int i = 0; i < redCount - 1; i++) {
        bufferRed[i] = bufferRed[i+1]; // shifting the elements to the left one
    }
    redCount = redCount - 1;
    // pthread_cond_signal(&redNotEmpty);
    // pthread_mutex_unlock(&redMutex);

    return value;
}

/*
The Assembly manager contains:
2 worker/producer threads threadL, threadR
2 consumer threads (threadX, threadY)
Next, I use the pthread_create function to initialize the threads. 
The producerThreadL takes the filePointer as a function paramter and return NULL;
The producerThreadR takes the filePointer as a function paramter and returns NULL

*/

void assemblyManager(FILE *filePointer) {
   
    pthread_t threadL, threadR, threadX, threadY;
    //int sequenceNumber = 0; //this variable keeps track of the 
    //amount of parts the worker threads read from the data file.
    //int shutdown = 0; // count of the completion of threads 
    if (pthread_create(&threadL, NULL, producerThreadL, filePointer) != 0) {
        perror("Error creating thread");
        exit(EXIT_FAILURE);
    }
    if (pthread_create(&threadR, NULL, producerThreadR, filePointer) != 0) {
        perror("Error creating thread");
        exit(EXIT_FAILURE);
    }

    /*
    
    BLUE_delivery.txt - this file will be the output of threadX (a consumer thread)
    RED_delivery.txt - this file will be the output of threadY (another consumer thread)
    
    */
    if (pthread_create(&threadX, NULL, consumerX, NULL) != 0) {
        perror("Error creating thread");
        exit(EXIT_FAILURE);
    }
    if (pthread_create(&threadY, NULL, consumerY, NULL) != 0) {
        perror("Error creating thread");
        exit(EXIT_FAILURE);
    }

    if (pthread_join(threadL, NULL) == 0) {
        printf("The ThreadL has been completed succesfully\n");
        printf("threadL shutdown = %d\n", shutDown);
        //exit(EXIT_SUCCESS);
    }
     if (pthread_join(threadR, NULL) == 0) {
        printf("ThreadR The call has been completed succesfully\n");
        printf("threadR shutdown = %d\n", shutDown);
    }
    // if (shutDown == 2) {
    //     exit(EXIT_SUCCESS);
    // }
    if (pthread_join(threadX, NULL) == 0) {
        printf("ThreadX has been completed succesfully\n");
        //exit(EXIT_SUCCESS);
    }
   
    if (pthread_join(threadY, NULL) == 0) {
        printf("ThreadY The call has been completed succesfully\n");
    }
}

/*
The producerThreadL convert the null pointer argument to a file pointer and returns NULL
I lock the blueMutex
Next I loop through the file storing the value in part, and checking if not EOF has reached.
Next I use snprintf to create a newValue which combines part andd sequenceNumber
Next I am checking if blueCount == BLUE_BUFFER_SIZE of so then put the thread to sleep.
Next I am checking if part belongs in red or blue buffer and putting them accordingly, and increasing sequencenumber.
The totalNumbers check how many numbers have been read from the file. If 25 numbers have reached
then I put the thread to sleep.
Lastly, I provide the signal that blueIsFull and I unlock the blueMutex;
*/

void *producerThreadL(void *args) {
    //printf("%s\n", "Inside Producer ThreadL");
    FILE *file = (FILE*)args;
    int part;
    //int numbers[NUM_TO_READ_FOR_THREADL];
    int totalNumbers = 0;
    pthread_mutex_lock(&blueMutex);
    while (fscanf(file, "%d", &part) == 1) { // reading one integer from the file
        //printf("ThreadL: totalNumbers = %d, part = %d\n", totalNumbers, part);
        printf("Blue part = %d\n", part);
        char newValue[20];
        snprintf(newValue, sizeof(newValue), "%d;%d", part, sequenceNumber);
        char *copiedValue = strdup(newValue);
        //pthread_mutex_lock(&blueMutex);
        //printf("Acquired Red Lock\n");

        while (blueCount == BLUE_BUFFER_SIZE) {
            printf("%s\n", "Going inside blue sleep, blueCount == BLUE_BUFFER_SIZE");
            pthread_cond_wait(&blueIsEmpty, &blueMutex);
        }

        if (part >= 1 && part <= 12) {
            putBlue(copiedValue);
            sequenceNumber++;
            usleep(200000);
        } 
        if (part >= 13 && part <= 25) {
            putRed(copiedValue);
            sequenceNumber++;
            usleep(200000);
        } 
        // pthread_mutex_lock(&LOCK);
        // sequenceNumber++;
        // pthread_mutex_unlock(&LOCK);
        totalNumbers++;
        if ((totalNumbers% NUM_TO_READ_FOR_THREADL) == 0) { // 25 numbers read put sleep. 
            //printf("ThreadL totalNumbers = %d\n", totalNumbers);
            usleep(250000);
        }
        pthread_cond_signal(&blueIsFull);
        pthread_mutex_unlock(&blueMutex);

        // printArrayBlueBuffer();
        // printArrayRedBuffer();
        usleep(50000); // sleep for 0.25 seconds after each delivery
    }
    if (feof(file)) {
        pthread_mutex_lock(&shutdownMutex);
        printf("ThreadL: Incrementing shutdown\n");
        shutDown++;
        pthread_mutex_unlock(&shutdownMutex);
    }
    return NULL;
}

/*
The producerThreadR convert the null pointer argument to a file pointer and returns NULL
I lock the redMutex
Next I loop through the file storing the value in part, and checking if not EOF has reached.
Next I use snprintf to create a newValue which combines part andd sequenceNumber
Next I am checking if redCount == RED_BUFFER_SIZE of so then put the thread to sleep.
Next I am checking if part belongs in red or blue buffer and putting them accordingly, and increasing
sequence Number.
The totalNumbers check how many numbers have been read from the file. If 15 numbers have reached
then I put the thread to sleep.
Lastly, I provide the signal that blueIsFull and I unlock the blueMutex;
*/

void *producerThreadR(void *args) {
    //printf("%s\n", "Inside Producer ThreadR");
    FILE *file = (FILE*)args;
    int part;
    int totalNumbers = 0;
    pthread_mutex_lock(&redMutex);
    while (fscanf(file, "%d", &part) == 1) {
        //printf("ThreadR: totalNumbers = %d, part = %d\n", totalNumbers, part);
        printf("Red part = %d\n", part);
        char newValue[20];
        snprintf(newValue, sizeof(newValue), "%d;%d", part, sequenceNumber);
        char *copiedValue = strdup(newValue);
        //printf("Acquired Red Lock\n");

        while (redCount == RED_BUFFER_SIZE) {
            printf("%s\n", "Going inside blue sleep, blueCount == BLUE_BUFFER_SIZE");
            pthread_cond_wait(&redIsEmpty, &redMutex);
        }

        if (part >= 1 && part <= 12) {
            putBlue(copiedValue);
            sequenceNumber++;
            usleep(200000);
        } 
        if (part >= 13 && part <= 25) {
            putRed(copiedValue);
            sequenceNumber++;
            usleep(200000);
        } 
        
        totalNumbers++;
        if((totalNumbers% NUM_TO_READ_FOR_THREADR) == 0) { // read 15 numbers and then sleep.
            //printf("ThreadR totalNumbers = %d\n", totalNumbers);
            usleep(50000);
        }
        pthread_cond_signal(&redIsFull);
        pthread_mutex_unlock(&redMutex);

        // printArrayBlueBuffer();
        // printArrayRedBuffer();
    }
    if (feof(file)) {
        pthread_mutex_lock(&shutdownMutex);
        printf("ThreadR: Incrementing shutdown\n");
        shutDown++;
        pthread_mutex_unlock(&shutdownMutex);
    }
    return NULL;
}

/*
The consumerX functions takes void ponter as an argument and returns NULL
I lock the blueMutex
If the blueCount == 0, I put the thread to sleep and wait for producer.
Next I provide the signal of empty, and unlock the blueMutex
After using the lock I put the thread to sleep.
Next, I open the BLUE_DELIVERY.txt file and append the value to the file. 
I break down the value using sscanf and then append the value to the file.
*/


void *consumerX(void *args) {
    //printf("%s\n", "Inside consumer X");
    char *value;
    while(true) {
        //printf("%s\n", "Blue part of the consumer");
        pthread_mutex_lock(&blueMutex);
        while(blueCount == 0) {
            pthread_cond_wait(&blueIsFull, &blueMutex);
        }
        value = getBlue();        // create delay

        pthread_cond_signal(&blueIsEmpty);
        pthread_mutex_unlock(&blueMutex);

        usleep(200000);
        printf("BlueNumberFromGet = %s\n",value);
        // pthread_cond_signal(&blueIsFull);
        // pthread_mutex_unlock(&blueMutex);
        
        FILE *file = fopen("BLUE_DELIVERY.txt", "a");
        //printf("Filename in consumer = BLUE_DELIVERY.txt");        
        if (file == NULL) {
            perror("Error opening the file");
            EXIT_FAILURE;
        }
        int part, seq;
        if (sscanf(value, "%d;%d", &part, &seq) == 2) {
            // if (value == NULL) {
            //     pthread_mutex_lock(&shutdownMutex);
            //     shutDown++;
            //     pthread_mutex_unlock(&shutdownMutex);
            // }
            fprintf(file, "Number = %d, sequence number = %d\n", part, seq);
        } else {
            printf("Error parsing value: %s\n", value);
            EXIT_FAILURE;
        }
        fclose(file);
        pthread_mutex_lock(&shutdownMutex);
        if (shutDown > 0) {
            pthread_mutex_unlock(&shutdownMutex);
            break;  // Exit the loop and thread
        }
        pthread_mutex_unlock(&shutdownMutex);
    }
    return NULL;
}

/*
The consumerY functions takes void ponter as an argument and returns NULL
I lock the redMutex
If the redCount == 0, I put the thread to sleep and wait for producer.
Next I provide the signal of empty, and unlock the redMutex
After using the lock I put the thread to sleep.
Next, I open the RED_DELIVERY.txt file and append the value to the file. 
I break down the value using sscanf and then append the value to the file.
*/

void *consumerY(void *args) {
    //printf("%s\n", "Inside consumerY");
    char *value;
    while(true) {
        //printf("%s\n", "red part of the consumer");
        pthread_mutex_lock(&redMutex);
        while(redCount == 0) {
            pthread_cond_wait(&redIsFull, &redMutex);
        }
        value = getRed();
        // create delay

        pthread_cond_signal(&redIsFull);
        pthread_mutex_unlock(&redMutex); 
        usleep(200000);
        printf("redNumberFromGet = %s\n",value);
        // pthread_cond_signal(&redIsFull);
        // pthread_mutex_unlock(&redMutex);
        
        FILE *file = fopen("RED_DELIVERY.txt", "a");
        //printf("Filename in consumer = RED_DELIVERY.txt");        
        if (file == NULL) {
            perror("Error opening the file");
            EXIT_FAILURE;
        }
        int part, seq;
        if (sscanf(value, "%d;%d", &part, &seq) == 2) {
            // if (value == NULL) {
            //     // printf("%s\n", "EOF has been reached");
            //     // break;
            //     pthread_mutex_lock(&shutdownMutex);
            //     shutDown++;
            //     pthread_mutex_unlock(&shutdownMutex);
            // }
            fprintf(file, "Number = %d, sequence number = %d\n", part, seq);
        } else {
            printf("Error parsing value: %s\n", value);
            EXIT_FAILURE;
        }
        fclose(file);
        pthread_mutex_lock(&shutdownMutex);
        if (shutDown > 0) {
            pthread_mutex_unlock(&shutdownMutex);
            break;  // Exiting the loop and thread
        }
        pthread_mutex_unlock(&shutdownMutex);
    }
    return NULL;
}

/*
The next 4 functions were created just to debug the code. 

*/


void printArrayBlueBuffer() {
    for(int i = 0; i < BLUE_BUFFER_SIZE; i++) {
        printf("Blue: i %d ith position = %s\n", i, bufferBlue[i]);
    }   
}

void printArrayRedBuffer() {
    for(int i = 0; i < RED_BUFFER_SIZE; i++) {
        printf("Red: i = %d at ith position = %s\n", i, bufferRed[i]);
    }   
}

void printArray(char *arr) {
    for (int i = 0; i < NUM_TO_READ_FOR_THREADL; i++) {
        printf("%d ", arr[i]);
    }
}

void printArrayForThreadR(char *arr) {
    for (int i = 0; i < NUM_TO_READ_FOR_THREADR; i++) {
        printf("%d ", arr[i]);
    }
}