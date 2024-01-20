# Project-2-F23  
## Project 2 Exploring Synchronization
 
1. Execution instructions:

If you put the make command in the terminal it will create an executable file main for you. 
After that all you need to run is ./main, and the program would run for you. 

Design:

2. main.c

My code starts of in the main, where I use fork() and exec() commands. I used fork() to split
the program into the child and parent processes. Furthermore, I used the execvp() command to
create a different running program. In the main I am using a file descriptor, and I am also converting the fd into string and passing it into my other funciton which is the child.c.
So the paramters which go into the execvp are "./child" which is the executable file for child.c 
and the string copy of the fd. 

child.c 
I have a couple of global variables defined. Some of them are int bufferBlue[], int bufferRed[], int blueCount, int redCount, int sequenceNumber. Furthermore, I am also using two lock variable redMutex, blueMutex, and 4 conditional variables blueIfFull, blueIsEmpty, redIsFull, and redIsEmpty. In the main function which takes in the parameter of the string of fd. I convert that into an integer
and then set a FILE pointer to the fd by using the fdopen function for reading. Next, I send that FILE pointer to thw function assemblyManager() which initializes all my threads using the pthread_create function, and then I wait for those threads to complete by using the pthread_join function. There are two producer threads (threadL and threadR), and two consumer threads(threadX and threadY). The produceThreadL, and produceThreadR takes parameter of the FILE pointer. I start the lock over here. Next, I loop through the file until EOF reached. Every part which is taken from the file I create a new string value of part;sequencenNumber as a way to keep track of sequenceNumber. Next, Depending on the value of the part I use the putBlue(value), and putRed(value) functions which also sleeps for 0.20 seconds. After it reaches putBlue() it puts the value in the bufferBlue and then increments the blueCount. Similarly, after it reaches putRed() it puts the value in bufferRed and then incremenets the redCount. If the redCount == redBufferSize or blueCount == blueBufferSize I put the threads to sleep. After collecting 25 numbers for threadL I put the thread to sleep, similarly after producing 15 numbers from threadR I put the thread to sleep. I also release the lock() and singal full before the end of the function. 

For the consumers, I start by putting a lock on them. Next, I am checking. If the blueCount or redCount == 0. In that case, I put the threads to sleep and wait for producers. Next, I use the getBlue() function for producerX() which returns the first value in the bufferBlue array and shift all the elements in the bufferBlue to the left one. After retreving the value from the getBlue or getRed function I am next splitting the value. The value is in the format of part;sequenceNumber. So I split the part and the sequenceNumber and then open the RED_DELIVERY or BLUE_DELIVERY file depending on wheather it is consumerY or consumerX and then append the split values into the file using the fprintf function. I use the sleep function and put the thread to sleep after every get function for 0.2 seconds. 

3. Testing

After writing the smallest piece of code I would test my code. There can be many errors which can come across threads like memory leakage, deadlocks, race condition etc. I was repeatedly testing my code in the server. 


