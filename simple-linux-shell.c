/**
 * Omar Al-Sadi (B00895876)
 * Assignment2: A Simple Shell for Linux
 * CSCI3120
 */
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_LINE 80 /* The maximum length command */
#define HISTORY_SIZE 10

//function declarations
bool allSpaces(const char *str);
void print_history(char **cmd_buffer, int *id_buffer, pid_t *pid_buffer, int current_size);
static void update_history(char **cmd_buffer, int *id_buffer, pid_t *pid_buffer, char *value, pid_t, int *size);
void insertionSort(int *id_buffer, pid_t *pid_buffer, char **cmd_buffer, int n);
int getRecentIndex(int *id_buffer);
int getByIdIndex(int *id_buffer, int id);


int main(void) {
    char *args[MAX_LINE / 2 + 1]; /* an array of character strings */
    int should_run = 1; /* flag to determine when to exit program */
    //declare a buffer to read user input
    char *inputBuffer;
    //allocate memory for history implementation
    int size = 0;
    char *fullCommand = (char *) calloc(80, sizeof(char));
    char **history_commnd = (char **) calloc(10, sizeof(char *));
    int *history_ID = (int *) calloc(10, sizeof(int));
    pid_t *history_PID = (pid_t *) calloc(10, sizeof(pid_t));

    for (int i = 0; i < HISTORY_SIZE; i++) {
        history_commnd[i] = (char *) calloc(MAX_LINE + 1, sizeof(char));
    }

    while (should_run) {
        printf("CSCI320>");
        fflush(stdout);
        inputBuffer = (char *) calloc(80, sizeof(char));
        fgets(inputBuffer, MAX_LINE + 1, stdin);

        //get rid of the new line character
        size_t len = strlen(inputBuffer);
        if (len > 0 && inputBuffer[len - 1] == '\n') {
            inputBuffer[len - 1] = '\0';
        }
        //continue if input is empty
        if (inputBuffer[0] == '\0' || allSpaces(inputBuffer)) {
            free(inputBuffer);
            continue; // Skip to the next iteration of the loop
        }
        //get the commands from history if input is !! or !N
        if (strcmp(inputBuffer, "!!") == 0) {
            if (size == 0) {
                printf("No commands in history!\n");
                continue;
            }
            // Fetch the most recent command from the history
            int recentIndex = getRecentIndex(history_ID);
            // Make sure there's a recent command
            if (recentIndex != -1) {
                strcpy(inputBuffer, history_commnd[recentIndex]);
                // Display the command being executed
                printf("%s\n", inputBuffer);
            }
        } else if (inputBuffer[0] == '!' && isdigit(inputBuffer[1])) {
            int commandNumber = atoi(&inputBuffer[1]);
            int index = getByIdIndex(history_ID,commandNumber);
            //Make sure command exist
            if (index == -1) {
                printf("Such command is NOT in history!\n");
                fflush(stdout);
                continue;
            }
            // Fetch the command from the history
            strcpy(inputBuffer, history_commnd[index]);
            // Display the command being executed
            printf("%s\n", inputBuffer);
            fflush(stdout);
        }
        //copy the full command to be passed to update_history method
        strcpy(fullCommand, inputBuffer);

        //seperate string into tokens for execvp() function
        int n = 0;
        char *token = strtok(inputBuffer, " ");
        while (token != NULL) {
            args[n] = (char *) malloc(strlen(token) + 1);
            strcpy(args[n++], token);
            token = strtok(NULL, " ");
        }
        args[n++] = NULL;

        //if command is "history" print the history
        if (strcmp(args[0], "history") == 0) {
            print_history(history_commnd, history_ID, history_PID, size);
            // Skip the rest of the loop after printing history
            continue;
        }

        //variable declaration for process creation and execution
        int execvpStatus;
        int waitStatus;
        pid_t pid;
        pid = fork();
        //check if process is forked
        if (pid < 0) {
            printf("fork failed\n");
            fflush(stdout);
            return 1;
        }
        //child process
        else if (pid == 0) {
            execvpStatus = execvp(args[0], args);
            if (execvpStatus == -1) {
                printf("Invalid command!\n");
                fflush(stdout);
                exit(1);
            }
        }
        //parent process
        else {
            wait(&waitStatus);
            update_history(history_commnd, history_ID, history_PID, fullCommand, pid, &size);
        }

        //free allocated memory
        for (int i = 0; i < n; i++) {
            free(args[i]);
        }
        free(inputBuffer);
    }

    for (int i = 0; i < HISTORY_SIZE; i++) {
        free(history_commnd[i]);
    }
    free(history_commnd);

    // Free history_ID and history_PID arrays
    free(history_ID);
    free(history_PID);
    return 0;
}
// Define function for updating command history, including managing memory and IDs for history records
static void update_history(char **cmd_buffer, int *id_buffer, pid_t *pid_buffer, char *value, pid_t pid, int *current_size) {
    int size = *current_size;
    if (size < HISTORY_SIZE) {
        pid_buffer[size] = pid;
        id_buffer[size] = 1;
        strcpy(cmd_buffer[size], value);
        (*current_size)++;
        for (int i = 0; i < size; ++i) {
            id_buffer[i] += 1;
        }
    } else {
        for (int i = 0; i < size; i++) {
            if (id_buffer[i] == size) {
                id_buffer[i] = 1;
                pid_buffer[i] = pid;
                strcpy(cmd_buffer[i], value);
                continue;
            }
            id_buffer[i] += 1;
        }
    }
}
// Define function for printing the command history, sorted by ID
void print_history(char **cmd_buffer, int *id_buffer, pid_t *pid_buffer, int current_size) {
    if (current_size == 0) {
        printf("No commands in history!\n");
        return;
    }
    insertionSort(id_buffer, pid_buffer, cmd_buffer, current_size);
    printf("ID PID Command\n");
    for (int i = 0; i < current_size; i++) {
        printf("%d %d %s\n", id_buffer[i], pid_buffer[i], cmd_buffer[i]);
    }
}
//Sorting function to sort the history records using insertion sort
//The code is adapted from: https://www.geeksforgeeks.org/insertion-sort/
void insertionSort(int *id_buffer, pid_t *pid_buffer, char **cmd_buffer, int n) {
    int i, j;
    int id_key;
    pid_t pid_key;
    char *cmd_key;

    for (i = 1; i < n; i++) {
        id_key = id_buffer[i];
        pid_key = pid_buffer[i];
        cmd_key = cmd_buffer[i];
        j = i - 1;

        while (j >= 0 && id_buffer[j] > id_key) {
            id_buffer[j + 1] = id_buffer[j];
            pid_buffer[j + 1] = pid_buffer[j];
            cmd_buffer[j + 1] = cmd_buffer[j];
            j = j - 1;
        }
        id_buffer[j + 1] = id_key;
        pid_buffer[j + 1] = pid_key;
        cmd_buffer[j + 1] = cmd_key;
    }
}
// Define function to get the index of the most recent command or a specific command by ID
int getByIdIndex(int *id_buffer, int id) {
    for (int i = 0; i < HISTORY_SIZE; i++) {
        if (id_buffer[i] == id) {
            // Return the index where the ID matches
            return i;
        }
    }
    // Indicate not found
    return -1;
}
int getRecentIndex(int *id_buffer) {
    for (int i = 0; i < HISTORY_SIZE; i++) {
        if (id_buffer[i] == 1) {
            // Return the index of the most recent command
            return i;
        }
    }
    // Return -1 if not found, indicating no history is present
    return -1;
}
// Define a utility function to check if a string contains only spaces
bool allSpaces(const char *str) {
    for (int i = 0; str[i]; i++) {
        if (!isspace((unsigned char) str[i])) {
            return false;
        }
    }
    return true;
}