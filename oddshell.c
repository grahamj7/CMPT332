#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "unistd.h"

struct LinkedList{
    char **wordArray;
    int length;
    struct LinkedList *next;
};

void delete(struct LinkedList *cursor){
    if (cursor->next == NULL)
        free(cursor);
    else {
        delete(cursor->next);
        free(cursor);
    }
}

void execute_command_cursor(struct LinkedList *cursor, int pipes_count, int **pipes){
    int i=0, childPID;

    childPID = fork();
    if (childPID == -1){
        fprintf(stderr, "Error in forking.\n");
        exit(1);
    } else if (childPID == 0) {
        printf("(%d) Words: ", getpid());
        while (cursor->wordArray[i] != 0) {
            printf("%s, ", cursor->wordArray[i]);
            i++;
        }
        printf("\n");
        if(execvp(cursor->wordArray[0], cursor->wordArray) == -1){
            fprintf(stderr, "Execution failed: CMD: %s, Args: ", cursor->wordArray[0]);
            i = 1;
            while (cursor->wordArray[i] != 0) {
                fprintf(stderr, "%s, ", cursor->wordArray[i]);
                i++;
            }
            fprintf(stderr, "Child %d exited\n", getpid());
            exit(1);
        }
    } else {
        wait(NULL);
        /* Waits for child to process then calls the next command in line*/
        if (cursor->next != NULL)
            execute_command_cursor(cursor->next, pipes_count, pipes);

        /* Returns to above process call */
        /* Do nothing here */
    }
}

void execute_pipes(char **wordArray, int wordArray_length){
    int j, childPID;

    childPID = fork();
    if ( -1 == childPID ) {
        fprintf(stderr, "Error in forking.\n");
        exit(1);
    }
    else if ( 0 == childPID ) { /* Child process to execute */
        printf("Im a child! (PID: %d)\n", getpid());
        printf("CMD: %s, Args: ", wordArray[0]);
        
        for(j = 1; j < wordArray_length; j++ ){
            printf("%s, \n", wordArray[j]);
        }

        if(execvp(wordArray[0], wordArray) == -1){
            fprintf(stderr, "Execution failed: CMD: %s, Args: ", wordArray[0]);
            for(j = 1; j < wordArray_length -1; j++)
                fprintf(stderr, "%s, ", wordArray[j]);
            fprintf(stderr, "%s\n", wordArray[j]);
            fprintf(stderr, "Child %d exited\n", getpid());
            exit(1);
        }
    }
    else { /* Parent process waits for child to complete */
        wait(NULL);

        printf("I'm the parent! (PID: %d)\n", getpid());
    }
    
}

int odd_shell(){
    char *input_str, *pipe_args, *cmd_args;
    char **wordArray, **pipesArray;
    size_t max_str_length = 128; /* max length of command typed after osh prompt */
    ssize_t nchar_read, length;
    int i, childPID, cmd_index, pipes_count, wordArray_length = 0, max_num_args = 64; /* max number of words types after osh prompt */
    int **pipefdArray = NULL;
    /* This will be the unchanging first node */
    struct LinkedList *root;
    /* This will point to each node as it traverses the list */
    struct LinkedList *cursor;


    /* Continue prompting until user types "exit" */
    while(1){

        /* Now root points to a node struct */
        root = (struct LinkedList *) malloc( sizeof(struct LinkedList) );
        /* The node root points to has its next pointer equal to a null pointer set */
        root->length = 0;
        root->next = NULL;
        root->wordArray = NULL;
        cursor = root;

        printf("(PID: %d) osh> ", getpid());
        fflush(stdout);
    	/* Get command line input */
    	input_str = ( char* ) malloc( max_str_length * sizeof(char) );
    	nchar_read = getline( &input_str, &max_str_length, stdin );

    	/* Check for getline() error */
    	if( -1 == nchar_read ){
    		return EXIT_FAILURE;
    	}

    	/* Delete /n character at end of line */
    	length = (strlen(input_str) -1);
    	input_str[length] = '\0';

        /* get pipes */
        pipe_args = strtok (input_str, "|");
        pipesArray = (char **) malloc(max_num_args * sizeof(char));
        pipes_count = 0;

        while((pipe_args != NULL) && (pipes_count < max_num_args))
        {
            pipesArray[pipes_count] = malloc(strlen(pipe_args) + 1);
            strcpy(pipesArray[pipes_count], pipe_args);
            pipe_args = strtok(NULL, "|");
            pipes_count += 1;
        }
        pipesArray[pipes_count] = NULL; /* Make last entry in array null pointer */

        if (pipes_count == 0) /* handle blank line entered */
            continue;

        for (cmd_index = pipes_count-1; cmd_index >= 0; cmd_index--) {

            wordArray = (char **) malloc(max_num_args * 128 * sizeof(char));
            wordArray_length = 0;
            cmd_args = strtok (pipesArray[cmd_index], " ");

            while((cmd_args != NULL) && (wordArray_length < max_num_args)) {
                wordArray[wordArray_length] = malloc(strlen(cmd_args) + 1);
                strcpy(wordArray[wordArray_length], cmd_args);
                cmd_args = strtok(NULL, " ");
                wordArray_length += 1;
            }

            /* Check if command was an exit message */
            if(strcmp( wordArray[0], "exit" ) == 0){
                return EXIT_SUCCESS;
            }
            cursor->wordArray = wordArray;
            cursor->length = wordArray_length;
            /* Creates a node at the end of the list */
            if (cmd_index != 0) {
                cursor->next = malloc(sizeof(struct LinkedList));
                cursor = cursor->next;
            }
        }

        cursor = root;
        while (cursor != NULL){
            printf("CMD: %s\n", cursor->wordArray[0]);
            execute_pipes(cursor->wordArray, cursor->length);
            cursor=cursor->next;
        }

        /* todo stdout here */


    	/* Free allocated memory */
    	free (input_str);

        delete(root->next);

        /* Decrement wordArray_length to the highest index number */
        for( wordArray_length = wordArray_length - 1; wordArray_length > -1; wordArray_length--){
            free (wordArray[wordArray_length]);
        }
        wordArray_length = 0;
        for( pipes_count = pipes_count - 1; pipes_count > -1; pipes_count--){
                free (pipesArray[pipes_count]);
            }
        }

    /* Return because error has occurred if this
       code has been reached. */
    return EXIT_FAILURE;   
}

int main(int argc, char* argv[]) {
    return odd_shell();
}



