/*
 * pipes_processes1.c - Lab 3, Part 1: two-way communication with two pipes
 * Author: Sumayia Moore
 *
 * P1 (parent) reads a string from the user and sends it to P2 (child) on pipe 1.
 * P2 appends "howard.edu", prints it, prompts for a second string, appends
 * that, and sends the result back to P1 on pipe 2.
 * P1 appends "gobison.org" and prints the final string.
 * Prompts and output follow the assignment's sample format
 * ("Input : " and "Output : ").
 */
#include<stdio.h> 
#include<stdlib.h> 
#include<unistd.h> 
#include<sys/types.h> 
#include<string.h> 
#include<sys/wait.h> 
  
int main() 
{ 
    // Two pipes:
    //   fd1: parent (P1) -> child (P2), carries the first input string
    //   fd2: child (P2) -> parent (P1), carries the combined string back
    int fd1[2];  // Used to store two ends of first pipe 
    int fd2[2];  // Used to store two ends of second pipe 
  
    char fixed_str[] = "howard.edu";   // appended by P2
    char fixed_str2[] = "gobison.org"; // appended by P1
    char input_str[100]; 
    pid_t p; 
  
    if (pipe(fd1)==-1) 
    { 
        fprintf(stderr, "Pipe Failed" ); 
        return 1; 
    } 
    if (pipe(fd2)==-1) 
    { 
        fprintf(stderr, "Pipe Failed" ); 
        return 1; 
    } 
  
    // Sample-format output: show the other string, then prompt with "Input : "
    printf("Other string is: %s\n", fixed_str);
    printf("Input : ");
    fflush(stdout);
    scanf("%99s", input_str); 
    p = fork(); 
  
    if (p < 0) 
    { 
        fprintf(stderr, "fork Failed" ); 
        return 1; 
    } 
  
    // Parent process (P1)
    else if (p > 0) 
    { 
        close(fd1[0]);  // Parent only writes on pipe 1
        close(fd2[1]);  // Parent only reads on pipe 2
  
        // Send the input string to the child
        write(fd1[1], input_str, strlen(input_str)+1); 
        close(fd1[1]);
  
        // Read the string the child sends back
        char final_str[256]; 
        read(fd2[0], final_str, sizeof(final_str)); 
        close(fd2[0]);

        // Concatenate "gobison.org" and print in the sample format
        strcat(final_str, fixed_str2);
        printf("Output : %s\n", final_str);

        // Wait for child to finish
        wait(NULL); 
    } 
  
    // Child process (P2)
    else
    { 
        close(fd1[1]);  // Child only reads on pipe 1
        close(fd2[0]);  // Child only writes on pipe 2
      
        // Read the string from the parent
        char concat_str[256]; 
        read(fd1[0], concat_str, sizeof(concat_str)); 
        close(fd1[0]);
  
        // Concatenate "howard.edu" and print in the sample format
        strcat(concat_str, fixed_str);
        printf("Output : %s\n", concat_str);

        // Prompt for a second input and append it
        char second_str[100];
        printf("Input : ");
        fflush(stdout);
        scanf("%99s", second_str);
        strcat(concat_str, second_str);

        // Send the result back to the parent
        write(fd2[1], concat_str, strlen(concat_str)+1);
        close(fd2[1]);
  
        exit(0); 
    } 

    return 0;
}
