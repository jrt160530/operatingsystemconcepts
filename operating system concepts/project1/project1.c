#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


/*This is a simple shell program.
*/

int main(int argc, char **argv){

char buf[1024];
char *args[16];		
int counter = 0;
int p;

//prompt for command
printf("$$:");
while(fgets(buf, sizeof buf, stdin) != NULL){
	buf[strlen(buf) -1] ='\0';
	int i;
	counter++;
	
	//if input is not NULL parse command line
	if(buf != NULL) {
		args[0] = strtok(buf, " ");
		
		for(i=1; i<16; i++) 			
                args[i] = strtok(NULL, " ");

            // checks for 'exit' 
		if(!strcmp("exit", args[0])) {
                printf("Total number of commands entered: %d\n", counter);
				exit(0);
            }
			
			// check for 'cd' -> "change directory"
		else if(!strcmp("cd", args[0])) {
             chdir(args[1]);
			}
            
			// handles other commands
		else {
			p = fork();
		
			if(p < 0){
				printf("Error\n");
				}
		
			else if(p == 0){
				execvp(args[0], args);
				exit(0);
				}
			else{
				wait(1);
				}
			}	
	
	}
	//prompt for next command line
	printf("$$:");
}
	exit(0);
}