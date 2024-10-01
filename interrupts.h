#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include <stdio.h>

#define VECTOR_TABLE_SIZE 4

// Structure to hold activity data from trace.txt
typedef struct {
    char activity[10];   // Activity type (CPU, SYSCALL, END_IO)
    int position;        // Position for SYSCALL or END_IO (if applicable)
    int duration;        // Duration of the activity in milliseconds
} TraceEntry;

// Structure to represent an entry in the vector table
typedef struct {
    int interruptNumber;   // Interrupt number
    int memoryAddress;     // Memory address where the ISR is located
    int isrAddress;        // ISR start address (is the memory address in our case for simplicity)
} VectorEntry;

// Function declarations
int loadTrace(const char* filename, TraceEntry** traceEntries); // Loads trace input data
void useCPU(int duration, FILE* logFile, int* currentTime); // Simulates CPU work
void handleSysCall(int position, int duration, FILE* logFile, int* currentTime); // Simulates the handling of a system call
void simulateInterrupt(int interruptNumber, int isrDuration, FILE* logFile, int* currentTime); // Simulates an interrupt
void handleEndIO(int position, int duration, FILE* logFile, int* currentTime); // Simulates the handling of and input/output process

#endif // INTERRUPTS_H