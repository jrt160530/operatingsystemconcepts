#include <stdio.h>
#include <string.h>
//This program will accept an input stream
//It tokenizes each word
//It checks each token and outputs its command line format
//exit to terminate
int main()
{
	char str[30];		//holds char[] for input stream
	char c;			//holds char to insert into char[]
	char *t;		//Pointer to hold token
	int i = 0;		//i for an iterator

    printf("Enter Command \n");
    while(c != '\n')   		// will character is not newline(pressing enter)
    {
        c = getchar();		//gets char and assigns it to c
        str[i] = c;		//outs c into str[i]. i=iterator
        i++;			//iterates i
    }
    str[i] = '\0';       //insert null character. used to find EOL


i = 0;			//using to find arg[0]

//strtok("STRING, DELIMITER)
//assigns token to t
t = strtok (str," ");	
  
//while the token variable, t, is not NULL
while (t != NULL)
  {if(strcmp("exit\n", str) == 0){return(0);}	//if str = exit. stop program
    	
	//arg[0]. command in this example
	if(i == 0){
		printf("\ncommand: %s\n", t);
		t = strtok (NULL, " ");
		i++;
		}
	
	if(t == NULL){main();}//calls main to loop program
	
	//switch statement. checks for cases of:
	//options, file direction, pipe, and arguments
	switch(t[0]){
	case '-':{
		printf("Options: %s\n",t);
		t = strtok (NULL, " ");
		if(t != NULL)
			printf("Arguments: %s\n",t);
		break;}
	case '<':{
		printf("File Redirection: %s\n",t);
		t = strtok (NULL, " ");
		if(t != NULL)
			printf("File: %s\n",t);
		break;}

	case '>':{
		printf("File Redirection: %s\n",t);
		t = strtok (NULL, " ");
		if(t != NULL)
			printf("File: %s\n",t);
		break;}
	
	case '|':{
		printf("PIPE\n");
		t = strtok (NULL, " ");
		if(t != NULL)
			printf("Command: %s\n",t);
		break;}
	
	default:{
		printf("Arguments: %s\n",t);
		break;}
		
}//end of switch
	
	
	i++;
    	t = strtok (NULL, " ");
  }//end of while
main();
return(0);
}

