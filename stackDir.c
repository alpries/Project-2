#include "Llibc.h"
#include "Implt.h"

#define MAX_NAME_LENGTH 256
#define STACK_SIZE 128

typedef struct {
    int inode; // Inode number, if you need it for filesystem interaction
    char name[MAX_NAME_LENGTH]; // Directory name
} DirectoryEntry;

typedef struct {
    DirectoryEntry entries[STACK_SIZE];
    int top;
} DirectoryStack;

// Push a directory onto the stack
void push(DirectoryStack *stack, DirectoryEntry dir) {
    // checks if stack is full
    if (stack->top < STACK_SIZE - 1) {
        stack->top++;
        stack->entries[stack->top] = dir;
    } else {
        //Do something when the stack is full
    }
}

// Pop a directory from the stack
DirectoryEntry pop(DirectoryStack *stack) {
    if (stack->top >= 0) {
        DirectoryEntry dir = stack->entries[stack->top];
        stack->top--;
        return dir;
    } else {
        // Return something else when stack is empty
        return (DirectoryEntry){0, ""}; // Return an empty directory entry
    }
}

// Peek at the top directory on the stack without popping it
DirectoryEntry peek(const DirectoryStack *stack) {
    if (stack->top >= 0) {
        return stack->entries[stack->top];
    } else {
        // Return something else when a stack is empty
        return (DirectoryEntry){0, ""}; // Return an empty directory entry
    }
}

// print the current stack for pwd command
void printStack(const DirectoryStack *stack) {
    for (int i = 0; i <= stack->top; i++) {
        if (i<2){
            Lprintf("%s", stack->entries[i].name);
        }
        else{
            Lprintf("/%s", stack->entries[i].name);
        }

    }
    printf("\n");
}
