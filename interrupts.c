#include "interrupts.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>  // For usleep()
#include <time.h>    // For random numbers


// The vector table will be hardcoded as an array of structs
VectorEntry vectorTable[VECTOR_TABLE_SIZE] = {
    {7,  0x0E, 0x0E},   // SYSCALL 7
    {12, 0x18, 0x18},   // SYSCALL 12
    {20, 0x28, 0x28},   // END_IO 20
    {22, 0x16, 0x16}    // END_IO 22
};

// Function to load the input trace data
int loadTrace(const char* filename, TraceEntry** traceEntries){
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("Error: Could not open trace file: %s\n", filename);
        return -1;
    }

    char line[50]; // Buffer
    int count = 0;

    // Count the number of entries in the file
    while (fgets(line, sizeof(line), file)) {
        count++;
    }

    // Allocate memory for the trace entries
    *traceEntries = malloc(count * sizeof(TraceEntry)); // Derefrencing here means we are accessing a pointer to the first TraceEntry struct in an array of them
    if (!*traceEntries) {
        printf("Error: Memory allocation failed\n");
        fclose(file);
        return -1;
    }

    rewind(file);  // Go back to the start of the file

    // Load the trace entries into memory
    int index = 0;
    while (fgets(line, sizeof(line), file)) { // The fgets fn here reads one line from the "file" and stores it in the line buffer. (If fgets does not successfully read a line (returns null) that ends the while loop)
        TraceEntry entry; // Temporary struct created to store the line entry from the trace input
        sscanf(line, "%[^,], %d", entry.activity, &entry.duration); // sscanf parses the formatted data of a string (line). It reads a string until it encounters a comma, then reads an integer.

        // If SYSCALL or END_IO, extract the position
        if (strncmp(entry.activity, "SYSCALL", 7) == 0 || strncmp(entry.activity, "END_IO", 6) == 0) {
            sscanf(line, "%s %d, %d", entry.activity, &entry.position, &entry.duration);
        } else {
            entry.position = -1;  // No position for CPU activities
        }

        (*traceEntries)[index++] = entry; // Store the trace entry in the traceEntries array
    }

    fclose(file);
    return count;
}

// Function to simulate CPU usage
void useCPU(int duration, FILE* logFile, int* currentTime) {
    fprintf(logFile, "%d, %d, CPU usage\n", *currentTime, duration);
    *currentTime += duration;
}

// Function to simulate a system call (SYSCALL)
void handleSysCall(int position, int duration, FILE* logFile, int* currentTime) {
    fprintf(logFile, "%d, %d, SYSCALL %d initiated\n", *currentTime, 0, position);
    simulateInterrupt(position, duration, logFile, currentTime);
}

// Function to simulate interrupt handling
void simulateInterrupt(int interruptNumber, int isrDuration, FILE* logFile, int* currentTime) {
    fprintf(logFile, "%d, 1, switch to kernel mode\n", *currentTime);
    *currentTime += 1;

    // Save context (random between 1-3 ms)
    int contextSaveDuration = rand() % 3 + 1; // rand() generates random int, %3 ensures between 0 and 2, + 1 ensures between 1-3
    fprintf(logFile, "%d, %d, context saved\n", *currentTime, contextSaveDuration);
    *currentTime += contextSaveDuration;

    // Find vector in memory
    fprintf(logFile, "%d, 1, find vector %d in memory\n", *currentTime, interruptNumber);
    *currentTime += 1;

    // Get ISR address from vector table
    fprintf(logFile, "%d, 1, obtain ISR address\n", *currentTime);
    *currentTime += 1;

    // Execute ISR (first determine total random duration of ISR between 100-400 ms)
    int isrActivityDuration = rand() % 301 + 100;

    // Now break down ISR execution into detailed steps
    // 1. Save information to the Process Control Block (PCB)
    int saveToPCBDuration = isrActivityDuration * 0.25; // 25% of the total ISR time
    fprintf(logFile, "%d, %d, save information to PCB\n", *currentTime, saveToPCBDuration);
    *currentTime += saveToPCBDuration;

    // 2. Call the scheduler
    int schedulerCallDuration = isrActivityDuration * 0.15; // 15% of the total ISR time
    fprintf(logFile, "%d, %d, call the scheduler\n", *currentTime, schedulerCallDuration);
    *currentTime += schedulerCallDuration;

    // 3. Execute the scheduler (decide which process to run next)
    int schedulerExecutionDuration = isrActivityDuration * 0.4; // 40% of the total ISR time
    fprintf(logFile, "%d, %d, execute the scheduler\n", *currentTime, schedulerExecutionDuration);
    *currentTime += schedulerExecutionDuration;

    // 4. Update the interrupt controller status
    int interruptControllerUpdateDuration = isrActivityDuration * 0.2; // 20% of the total ISR time
    fprintf(logFile, "%d, %d, update interrupt controller status\n", *currentTime, interruptControllerUpdateDuration);
    *currentTime += interruptControllerUpdateDuration;

    // Execute IRET (interrupt return)
    fprintf(logFile, "%d, 1, IRET\n", *currentTime);
    *currentTime += 1;

    // Restore context (random between 1-3 ms)
    int contextRestoreDuration = rand() % 3 + 1;
    fprintf(logFile, "%d, %d, context restored\n", *currentTime, contextRestoreDuration);
    *currentTime += contextRestoreDuration;

    // Switch back to user mode
    fprintf(logFile, "%d, 1, switch to user mode\n", *currentTime);
    *currentTime += 1;
}


// Function to handle END_IO interrupts
void handleEndIO(int position, int duration, FILE* logFile, int* currentTime) {
    fprintf(logFile, "%d, %d, END_IO %d\n", *currentTime, duration, position);
    *currentTime += duration;

    // Simulate interrupt for END_IO
    simulateInterrupt(position, duration, logFile, currentTime);
}

// argc (argument count) represents the number of command line arguments passed to the main function
// argv (argument vector) is an array of strings (array of char pointers) containing the actual command line arguments
int main (int argc, char *argv[]){
   srand(time(NULL));  // Essentially sets the starting point for the random num gen. to be the current time

    // Check if the correct number of arguments is passed (we expect the trace file as an argument, along with the name of the program)
    if (argc < 2) {
        printf("Error: No trace file specified.\n");
        return 1;
    }

    TraceEntry* traceEntries;
    // Load the trace file input from the path specified in the command line argument
    int traceCount = loadTrace(argv[1], &traceEntries); // argv[1] stores the path to the trace file as a string (pointer to first char in string)
    if (traceCount == -1) {
        return 1;  // Error loading trace
    }

    FILE* logFile = fopen("execution.txt", "w");
    if (!logFile) {
        printf("Error: Could not create execution.txt\n");
        return 1;
    }

    int currentTime = 0;  // Simulated time in milliseconds

    // For each trace entry, process them depending on their activity
    for (int i = 0; i < traceCount; i++) {
        if (strcmp(traceEntries[i].activity, "CPU") == 0) {
            useCPU(traceEntries[i].duration, logFile, &currentTime);
        } else if (strcmp(traceEntries[i].activity, "SYSCALL") == 0) {
            handleSysCall(traceEntries[i].position, traceEntries[i].duration, logFile, &currentTime);
        } else if (strcmp(traceEntries[i].activity, "END_IO") == 0) {
            handleEndIO(traceEntries[i].position, traceEntries[i].duration, logFile, &currentTime);
        }
    }

    fclose(logFile);
    free(traceEntries);
    return 0;
}