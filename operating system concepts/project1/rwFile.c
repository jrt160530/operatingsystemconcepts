#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>


struct inode{
unsigned short flags;
char nlinks;
char uid;
char gid;
char size0;
unsigned short size1;
unsigned short addr[8];
unsigned short actime[2];
unsigned short modtime[2];
};

struct superblock{
unsigned short isize; //number of blocks devoted to the i-list(starts in block 2)
unsigned short fsize;
unsigned short nfree;
unsigned short free[100];
unsigned short ninode;
unsigned short inode[100];
char flock;
char ilock;
char fmod;
unsigned short time[2];
};



main(){

struct 	inode node;
char buf[32];
char *args[1];	
int fd;
unsigned int fm = 0x6000;

char *prompt1 = "(a) mount filename\n(b) ckfiletype inode #\n(c) filesize inode #\n(d) q \n";

printf("%s\n", prompt1);

while(fgets(buf, sizeof buf, stdin) != NULL){
	buf[strlen(buf) -1] ='\0';
	int i;
	
	//if input is not NULL parse command line
	if(buf != NULL) {
		args[0] = strtok(buf, " ");	
        args[1] = strtok(NULL, " ");

            // checks for 'q' 
  if(!strcasecmp("q", args[0])) {
                printf("Quit was selected\n");
				exit(0);
            }
		
		else if(!strcasecmp("mount", args[0])) {
            fd = open(args[1],O_RDONLY);
            if(fd == -1)
              printf("Mount unsuccessful\n");
            else
              printf("Mount successful\n");
			         
              
			}  
            
			// handles other commands
		else if(!strcasecmp("ckfiletype", args[0])){
		
			char *ptr;
			int n = (int)strtol(args[1], &ptr, 10); 
			if(lseek(fd, (512*2 + (n-1)*32), SEEK_SET) == -1)
				printf("Error. filesystem may not be mounted\n");
				
				else{
					read(fd, &node, sizeof(node));
					unsigned int fm = 0x6000;
					unsigned int ft = (node.flags & fm);
					ft = (ft >> 13);
				
					switch(ft){
						case 0: printf("plain file\n");
							break;
						case 1: printf("char special file\n");
							break;
						case 2: printf("directory\n");
							break;
						case 3: printf("block type special\n");
							break;
							  }
					  }
				  }//end of elseif	
		
		else if(!strcasecmp("filesize", args[0])){
			
      char *ptr2;
			int n = (int)strtol(args[1], &ptr2, 10); 
			if(lseek(fd, (512*2 + (n-1)*32), SEEK_SET) == -1)
				printf("Error. filesystem may not be mounted\n");
        
      else{
				read(fd, &node, sizeof(node));
    
        printf("%d\n", node.size0);
 	      printf("%d\n", node.size1);
              
        int fs = (node.size0 & node.size1);
		}
   }
   
   else
     printf("invalid command entered\n");
	
	}
	//prompt for next command line
	printf("%s:\n", prompt1);
}


exit(0); 


}
