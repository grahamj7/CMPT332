#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>


#define READ_END 0
#define WRITE_END 1

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

void execute_pipes(struct LinkedList *cmd_list, int num_pipes){

	int status, i, j, cmd_num;
	pid_t childPID;
	const int num_commands = num_pipes + 1;
	
	/* Create enough pipes to go between each command */
	int fd_arr[num_pipes][2];
	for( i = 0; i < num_pipes; i++ ){
		if( pipe(fd_arr[num_pipes]) < 0 ){
        	fprintf(stderr, "Error in creating pipes.\n");
        	exit(1);	
        }
    }
    
    for( cmd_num = 0; cmd_num < num_commands; cmd_num++ ){
    
    	childPID = fork();
    	
    	if ( childPID < 0 ) {
        	fprintf(stderr, "Error in forking.\n");
        	exit(1);
    	}		
    	
		if ( 0 == childPID ) { /* Child process to execute */
			/* If there was a previous command, read from pipe: */
			if( cmd_num > 0 ){
			    close( fd_arr[cmd_num -1][WRITE_END] );
				if( dup2(fd_arr[cmd_num - 1][READ_END], 0) < 0 ){
					fprintf(stderr, "Error with dup2.\n");
        			exit(1);
        		}
        		close( fd_arr[cmd_num -1][READ_END] );
			}
			
			/* If there is another command following, write to pipe ahead */
			if( cmd_num < num_pipes ){
				close( fd_arr[cmd_num][READ_END] );
				if( dup2(fd_arr[cmd_num][WRITE_END], 1) < 0 ){
					fprintf(stderr, "Error with dup2.\n");
        			exit(1);
        		}
        		close( fd_arr[cmd_num][WRITE_END] );
			}
			
			/* Execute current command */
			if(-1 == execvp(cmd_list->wordArray[0], cmd_list->wordArray) ){
            	fprintf(stderr, "Execution failed: CMD: %s, Args: ", cmd_list->wordArray[0]);
            	for(j = 1; j < cmd_list->length -1; j++)
                	fprintf(stderr, "%s, ", cmd_list->wordArray[j]);
            	fprintf(stderr, "%s\n", cmd_list->wordArray[j]);
            	fprintf(stderr, "Child %d exited\n", getpid());
            	exit(1);
        	}			
		}
		
		else { /* Parent */
			/* If there was a previous command, close fds: */
			if( cmd_num > 0 ){
				waitpid( childPID, &status, 0 );
				close( fd_arr[cmd_num-1][0] );
				close( fd_arr[cmd_num-1][1] );
			}
		}
		cmd_list = cmd_list->next;
	}
	waitpid( childPID, &status, 0 );
	for( i = 0; i < num_pipes; i++ ){
		close( fd_arr[i][0] );
		close( fd_arr[i][1] );
    }
}

/* inserts into subject[] at position pos */
char* append(char *subject, const char insert[], int pos) {
    ssize_t len;
    char *buf;
    buf = malloc(100 * sizeof(char)); /* 100 so that it's big enough.*/

    strncpy(buf, subject, pos); /* copy at most first pos characters */
    len = strlen(buf);
    strcpy(buf+len, insert); /* copy all of insert[] at the end */
    len += strlen(insert);  /* increase the length by length of insert[] */
    strcpy(buf+len, subject+pos); /* copy the rest */
    return buf;
}


int main(int argc, char* argv[]) {
    char *input_str, *pipe_args, *cmd_args, *file_name, *direction_parse, *stdout_str, *c;
    char **wordArray, **pipesArray;
    size_t max_str_length = 128; /* max length of command typed after osh prompt */
    ssize_t nchar_read, length;
    int should_redirect, cmd_index, pipes_count, position, wordArray_length = 0, max_num_args = 64; /* max number of words types after osh prompt */
    /* This will be the unchanging first node */
    struct LinkedList *root;
    /* This will point to each node as it traverses the list */
    struct LinkedList *cursor;
    int copy_stdout, out_file;
    copy_stdout = dup(1); /* save copy of stdout to switch output back to on each loop */


    /* Continue prompting until user types "exit" */
    while(1){
    	dup2( copy_stdout, 1 );

        /* Now root points to a node struct */
        root = (struct LinkedList *) malloc( sizeof(struct LinkedList) );
        /* The node root points to has its next pointer equal to a null pointer set */
        root->length = 0;
        root->next = NULL;
        root->wordArray = NULL;
        cursor = root;

        fflush(stdout);
        printf("(PID: %d) osh> ", getpid());
        
    	/* Get command line input */
    	input_str = ( char* ) malloc( max_str_length * sizeof(char) );
    	nchar_read = getline( &input_str, &max_str_length, stdin );

    	/* Check for getline() error */
    	if( -1 == nchar_read ){
    		close( copy_stdout );
    		return EXIT_FAILURE;
    	}
    	
    	/* Check for redirection */
    	direction_parse = strtok (input_str, "<");
    	file_name = direction_parse;
    	should_redirect = 0;
    	direction_parse = strtok (NULL, "<");
    	if(direction_parse != NULL){
    		input_str = direction_parse;
    		should_redirect = 1;
    	}
    	if( 0 == should_redirect ){ /* If no redirection */
    		input_str = file_name;
    	}
    	
    	/* Delete /n character at end of line */
    	length = (strlen(input_str) -1);
    	input_str[length] = '\0';

        /* Break command line entry into pipe delineated chunks in pipesArray */
        pipe_args = strtok (input_str, "|");
        pipesArray = (char **) malloc(max_num_args * sizeof(char));
        pipes_count = 0;

        while((pipe_args != NULL) && (pipes_count < max_num_args)){
            pipesArray[pipes_count] = malloc(strlen(pipe_args) + 1);
            strcpy(pipesArray[pipes_count], pipe_args);
            pipe_args = strtok(NULL, "|");
            pipes_count += 1;
        }
        pipesArray[pipes_count] = NULL; /* Make last entry in array null pointer */

        if (pipes_count == 0) /* handle blank line entered */
            continue;

		/* Put data from each pipe command into a linked list */
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
            	close( copy_stdout );
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

		/* Move pointer to current node in the command list to the root node */
        cursor = root;
        
        /* Set stdout to the file specified */
        if(1 == should_redirect){
		out_file = open( file_name , O_WRONLY |O_TRUNC| O_CREAT, S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR | S_IWGRP | S_IWOTH);
        	dup2( out_file, 1 );
    	}
        /* Execute the commands saved in the linked list */
        /* pipes_count-1 gives the number of "|"s in original command */
        execute_pipes(cursor, pipes_count-1);
        
        close( out_file );
/*
        stdout_str = malloc(max_str_length * sizeof(char));

   	char ch, *c;
      	FILE *fp;
       
	fp = fopen("stdout","r"); // read mode
	
	position = 0;
	while( ( ch = fgetc(fp) ) != EOF )
	        c = &ch;
		stdout_str = append(stdout_str, c, strlen(stdout_str));
	fclose(fp);


	printf("Before: %s\n", stdout_str);
        for (position = 0; position < strlen(stdout_str); position++)
            switch(stdout_str[position]) {
                case 'c':
                    c = "c";
                    stdout_str = append(stdout_str, c, position);
                    position++;
                    break;
                case 'm':
                    c = "m";
                    stdout_str = append(stdout_str, c, position);
                    position++;
                    break;
                case 'p':
                    c = "p";
                    stdout_str = append(stdout_str, c, position);
                    position++;
                    break;
                case 't':
                    c = "t";
                    stdout_str = append(stdout_str, c, position);
                    position++;
                    break;
                default:
                    break;
            }
        printf("After: %s\n", stdout_str);

        if( 1 == should_redirect  ){
            out_file = open( file_name , O_WRONLY |O_TRUNC| O_CREAT, S_IRUSR | S_IRGRP | S_IROTH );
            dup2( out_file, 1 );
            printf("%s\n", stdout_str);
            close( out_file );
        } else {
            printf("%s\n", stdout_str);
        }
*/
    	/* Free allocated memory */
    	//free (input_str);

        delete(root);

        /* Decrement wordArray_length to the highest index number */
        for( wordArray_length = wordArray_length - 1; wordArray_length > -1; wordArray_length--){
            free (wordArray[wordArray_length]);
        }
        wordArray_length = 0;
        for( pipes_count = pipes_count - 1; pipes_count > -1; pipes_count--){
                free (pipesArray[pipes_count]);
        }
        fflush(stdout);
    }

    /* Return because error has occurred if this code has been reached. */
    close( copy_stdout );
    return EXIT_FAILURE;   
}

