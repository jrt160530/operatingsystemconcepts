#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/*
 * stringtokenizer.c
 *
 *  Created on: Apr 28, 2019
 *      Author: Josh
 */

int main()
{
	char *arg[10];
	char filename[30] = "/file.txt";


	int i = 0;
	arg[i]=strtok(filename, "/");

	while(1){
		i++;
		if((arg[i]=strtok(NULL, "/")) == NULL)
			break;
	}

	int indexnumber = i;
	printf("\n index number: %d", indexnumber);
	i = 0;
	while(1){
		if(arg[i]==NULL)
			break;
		printf("\n%s", arg[i]);
		i++;
	}

	printf("\narg[indexnumber]: %s\narg[indexnumber-1]: %s", arg[indexnumber], arg[indexnumber-1]);

	printf("\n");
	exit(0);

}

