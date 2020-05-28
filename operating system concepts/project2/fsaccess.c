/*
 * fsaccess.c
 *
 *  Created on: Apr 17, 2019
 *      Author: Josh & Sri
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdbool.h> 

struct inode {
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

struct superblock {
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
struct superblock sb;

struct directoryEntry{
	unsigned short inodenum;
	char filename[14];
};

int mountFunction(char* fn);
int checkFileTypeFunction(int n, int fdPtr);
void fileSizeFunction(int n, int fdPtr);
void initFileSystem(int fdptr, unsigned short fsize, unsigned short isize);
void initDataBlocks(int fdptr);
void writeShortBlock(unsigned short blocknum, unsigned short *buf,int fdptr);
void writeCharBlock(unsigned short blocknum, char *buf,int fdptr);
void readShortBlock(unsigned short blocknum, unsigned short *buf, int fdptr);
int readCharBlock(unsigned short blocknum, char *buf, int fdptr);
void printiList(int fdptr);
void makeDirectory(char *filename, int fdptr);
void createRootDirectory(int fdptr);
void writeiNode(unsigned short inumber, struct inode node, int fdptr);
unsigned short allocateDataBlock(int fdptr);
unsigned short allocateInode(int fdptr);
void copyIn(char *srcFilename, char *dstFilename, int fdptr);
void copyOut(char *srcFilename, char *dstFilename, int fdptr);
void writeDirectoryEntry(struct directoryEntry d, unsigned short inumber, int fdptr);
char* tokenizeAbsolute(char *absolutepath);
void removefile(char *filename, int fdptr);
unsigned short changeDirectory(unsigned short inumber, char *filename, int fdptr);
unsigned short traverseDirectory(char *absolutepath, int fdptr);
void printDirectoryContent(int inodenum, int fdptr);
int checkDirectoryForDuplicate(char *filename, unsigned short inumber, int fdptr);

/*MAIN*/
int main() {

	//struct inode node;
	char buf[256];
	char *args[2];
	int fd;
	char command[32], argone[32], argtwo[32];

	char *prompt1 =
			"\n(a) mount filename\n(b) ckfiletype inode #\n(c) filesize inode #\n(d) initfs fsize isize \n(e) cpin filein newfilename "
			"\n(f) cpout fileout newfilename \n(g) mkdir newdirname \n(h) rm filename \n(i) printilist \n(j) q \n";

	printf("Type the command followed by the arguements.\nFor example: \nmount file.data \ninitfs 100 10 \ncpin filetocopyin copiedfilenewname\n\n\n\n");
	printf("%s\n$", prompt1);

	while (fgets(buf, sizeof buf, stdin) != NULL) {
		if (buf[0] != '\n' && buf[0] != ' ' && buf != NULL) {
			buf[strlen(buf) - 1] = '\0';
			int ndex;

			// parse command and arg
            args[0] = strtok(buf, " "); 	// grab command
			strcpy(command, args[0]);	// store command variable

            for(ndex=1; ndex<3; ndex++) 			// grab arguments
			{
                args[ndex] = strtok(NULL, " ");

				if(ndex == 1 && args[ndex] != NULL)
					strcpy(argone, args[ndex]);
				else if(ndex == 2 && args[ndex] != NULL)
					strcpy(argtwo, args[ndex]);
			}	
			

				// checks for 'q'
				if (!strcasecmp("q", command)) {
					printf("Quit was selected\n");
					exit(0);
				}

				/*mount file system*/
				else if (!strcasecmp("mount", command)) {
					fd = mountFunction(argone);
					if(fd == -1)
						printf("main: error mount function returned -1\n");
					else
					{
						//printf("main: mount function returned file descriptor\n");
					}
					
				}

				/*check file type*/
				else if (!strcasecmp("ckfiletype", command)) {
					char *ptr;
					int n = (int) strtol(argone, &ptr, 10);
					checkFileTypeFunction(n, fd);
				}

				/*check file size*/
				else if (!strcasecmp("filesize", command)) {
					char *ptr;
					int n = (int) strtol(argone, &ptr, 10);
					fileSizeFunction(n, fd);
				}

				/*make new directory in file system*/
				else if (!strcasecmp("mkdir", command)) {
				makeDirectory(argone, fd);
				}

				/*remove file from file system*/
				else if (!strcasecmp("rm", command)) {
				removefile(argone, fd);
				}

				/*initialize file system*/
				else if (!strcasecmp("initfs", command))
				{
					char *aptr;
					unsigned short fSize;
					unsigned short iSize;

					fSize = (unsigned short) strtol(argone, &aptr, 10);
					iSize = (unsigned short) strtol(argtwo, &aptr, 10);
					
					if(fSize > iSize)
						initFileSystem(fd, fSize, iSize);
					else
					{
						printf("initfs: fsize must be greater than isize\n");
					}
					

				}

				/*copies file from unix machine into file system*/
				else if (!strcasecmp("cpin", command)) {
					
					copyIn(argone, argtwo, fd);		
				}

				/*copies from file system into unix machine*/
				else if (!strcasecmp("cpout", command)){
					
					copyOut(argone, argtwo, fd);
				}

				/*check inode flag value*/
				else if (!strcasecmp("printilist", command)){
					printiList(fd);
				}

				else
					fprintf(stderr, "Invalid command\n");

				

		}
		else
			fprintf(stderr, "Invalid command: No input detected\n");

		printf("%s\n", prompt1);
		printf("\n$");
	}

	exit(0);

}/*EO MAIN*/


/* Added access() to check if the file exist. 
This function could be cleaned up some, and made more eloquent.
*/
int mountFunction(char* fn) {
	//printf("mountfunction: begin\n");
	int fd = 0;
	if(access(fn, F_OK) != -1){
		//printf("mountfunction: file found\n");
		fd = open(fn, O_RDWR);
		if(fd != 1)
		{
			printf("mountfunction: open successful\n");
			if(lseek(fd, 512, SEEK_SET) == -1){
				perror("mountfunction: read super block error");
				return -1;
				}
			else{
				read(fd, &sb, 512);
				return fd;
			}
		}
		
		
	}
	else
	{
		printf("mountfunction: file not found\n");
		return -1;
	}
	
	
	return -1;

};/**/



int checkFileTypeFunction(int n, int fdPtr) {
	int fdp = fdPtr;
	struct inode node;

	if (!(n < 1)) {
		if(n > sb.isize*16)
					{
						printf("\ni-number stops at %d", sb.isize*16);
						return;
					}
		if (lseek(fdp, (512 * 2 + (n - 1) * 32), SEEK_SET) == -1)
			fprintf(stderr, "No file system mounted\n");

		else {
			read(fdp, &node, sizeof(node));
			unsigned short fm = 24576;
			unsigned short ft = (node.flags & fm);
			//printf("\ncheck file type: node.flags = %hu\n", node.flags);
			ft = (ft >> 13);

			switch (ft) {
			case 0:
				printf("inode %d: plain file\n", n);
				return 0;
				break;
			case 1:
				printf("inode %d: char special file\n", n);
				return 1;
				break;
			case 2:
				printf("inode %d: directory\n", n);
				return 2;
				break;
			case 3:
				printf("inode %d: block type special\n", n);
				return 3;
				break;
			}
		}
	} else
		perror("inode number must be greater than 0\n");
};/**/

void fileSizeFunction(int n, int fdPtr) {
	int fd = fdPtr;
	struct inode node;

	if (!(n < 1)) {
		if(n > sb.isize*16)
					{
						printf("\ni-number stops at %d", sb.isize*16);
						return;
					}
		if (lseek(fd, (512 * 2 + (n - 1) * 32), SEEK_SET) == -1)
			fprintf(stderr, "No file system mounted\n");
		else {
			read(fd, &node, sizeof(node));
			unsigned int fs = node.size1;
			printf("inode %d: %d bytes\n", n, fs);
		}

	} else
		perror("\ninode number must be greater than 0\n");
};/**/

/*fill with zero to ensure no residual data left*/
void initFileSystem(int fdptr, unsigned short fsize, unsigned short isize){
	int fd = fdptr;
	/*Initialize superblock*/
	sb.fsize = fsize;
	sb.isize = isize;
	sb.nfree = 0;
	sb.ninode = 100;
	sb.flock = 'f';
	sb.ilock = 'i';
	sb.fmod = 0;
	sb.time[2] = 0;

	int i;
	int n = 101;
	/*initialize free & inode array*/
	for (i = 0; i<100; i++)
	{
		sb.free[i] =  0;
		sb.inode[i] = n;
		n--;
	}

	 /*Initialize blocks for ilist*/
	char buf[512];
	for(i = 0; i < 512; i++)
		buf[i] = 0;
	for(i = 0; i < sb.isize; i++)
		write(fd, &buf, 512);

	/*Set data blocks.*/
	initDataBlocks(fd);

	/*First 100 data blocks added to free array
	 * May change to function*/
	for(i = 0; i < 100; i++){
		sb.free[sb.nfree] = (2 + i + sb.isize);
		sb.nfree++;
	}

	/*write superblock to block 1*/
		if (lseek(fd, 512, SEEK_SET) == -1)
					perror("No file system mounted\n");
		else
		{
					if((write(fd, &sb,sizeof(sb))) < sizeof(sb))
					{
						perror("superblock initialization error/n");
					}
		}

	createRootDirectory(fd);
	return;

};/**/

/*initDataBlocks function*/
void initDataBlocks(int fdptr){
	/*Total number of data blocks are divided by 100.
	 * The first word of the first block is the
	 * number of available blocks in next set. (nfree)
	 * The next 100 words are the block numbers to be
	 * used. (free array)
	 * */
	int fd = fdptr;
	unsigned short datablocks = (sb.fsize-sb.isize-2);
	unsigned short numOfSets = (datablocks/100);
	unsigned short remainingBlocks = (datablocks%100);

	/*Array used to link 100 block addresses.*/
	unsigned short linkArray[256];
	/*Array used to fill data blocks with 0*/
	unsigned short initArray[256];

	int i;
	/*Initialize arrays with 0. Clear junk data*/
	for(i = 0; i < 256; i++){
		linkArray[i] = 0;
		initArray[i] = 0;
	}

	unsigned short setnum;
	/*Initialize each set of 100 blocks.*/
	for(setnum = 0; setnum < numOfSets; setnum++)
	{
		/*First word to be used as nfree*/
		linkArray[0] = 100;

		/*iteration to fill head block*/
		for(i = 0; i < 100; i++){
			/*The last set will point to remainder blocks, if any*/
			if(setnum == (numOfSets-1) && i==0)
				linkArray[0] = remainingBlocks;
			/*If there are no remaining blocks the last set points to zero*/
			if((setnum == (numOfSets - 1)) && (remainingBlocks == 0))
			{
				linkArray[i+1] = 0;
				continue;
			}
			if(setnum == (numOfSets -1) && i>=remainingBlocks)
				continue;
			/*Next 100 words filled with block numbers
			 *for free array*/
			linkArray[i+1] = 2+sb.isize+i+(100*(setnum+1));
		}
		/*write linkarray to block setnum * 100*/
		writeShortBlock(2+sb.isize+(100*setnum), linkArray, fd);

		/*For every set fill the next 99 blocks with 0s*/
		for(i=1; i < 100; i++)
			writeShortBlock(2+sb.isize+i+(100*setnum), initArray, fd);
	}

	/*Fill remaining blocks with 0*/
	if(remainingBlocks > 0){
		for(i=1; i <= remainingBlocks; i++){
			writeShortBlock(2+sb.isize + i+100*setnum, initArray, fd);
		}
	}
}/**/

/*writes unsigned short. checks for out of bound block numbers*/
void writeShortBlock(unsigned short blocknum, unsigned short *buf,int fdptr){

	int fd = fdptr;
	if(blocknum > (sb.fsize))
		perror("\nTrying to write to impossible block number\n");
	else{
		lseek(fd, blocknum*512,SEEK_SET);
		if(write(fd, buf, 512) > 512)
			perror("\nWrite short to block error.\n");
	}
}/**/

/*writes unsigned short. checks for out of bound block numbers
 * blocknum is the blocknumber to write to
 * fdptr is the file pointer of the v6 file system
 * buf is the buffer being written to fdptr*/
void writeCharBlock(unsigned short blocknum, char *buf,int fdptr){

	int fd = fdptr;
	if(blocknum > (sb.fsize))
		perror("\nTrying to write to impossible block number\n");
	else{
		lseek(fd, blocknum*512,SEEK_SET);
		if(write(fd, buf, 512) < 512)
			perror("\nWrite short to block error.\n");
	}
}/**/

void readShortBlock(unsigned short blocknum, unsigned short *buf, int fdptr){

	int fd = fdptr;
		if(blocknum > (sb.fsize))
			perror("\nTrying to write to impossible block number\n");
		else{
			lseek(fd, 512*blocknum, SEEK_SET);
			read(fd, buf, 512);
		}

}/**/

int readCharBlock(unsigned short blocknum, char *buf, int fdptr){

	int bytesread;
	int fd = fdptr;
		if(blocknum > (sb.fsize))
			perror("\nTrying to write to impossible block number\n");
		else{
			lseek(fd, 512*blocknum, SEEK_SET);
			bytesread = read(fd, buf, 512);
		}
		return bytesread;
}/**/



void printiList(int fdptr){
	int fd = fdptr;
	struct inode node;
	unsigned short filetyp;

	if(lseek(fd,1024,SEEK_SET) == -1){
		perror("\nNo FS mounted");
		fprintf(stderr, "\n");
	}

	unsigned short i = 0;
	for(i=1; i <= (sb.isize*16); i++){
		read(fd, &node, 32);
		//printf("\n%d: ", i);
		
		filetyp = checkFileTypeFunction(i, fd);
		
		//unsigned int ft = (node.flags && 32768);
		unsigned short flg = node.flags;
		//printf("\nprintilist: node.flags = %hu\n", flg);
		//ft = (ft && 1000000000000000);
		if((flg && 32768) == 1)
		{
			printf("is allocated\n\n");	
		}
		else
		{
			printf("unallocated\n");
		}
			
		if(filetyp == 2)
		{
			printDirectoryContent(i, fd);
			//Need to lseek to bring file descriptor back to correct position 
			if(lseek(fd,1024+(i)*32,SEEK_SET) == -1)
				perror("\nprintilist: error seeking\n");
		}


	}
	
	return;

};/**/


/*filename is an absolute pathname. if location of new directory
 * is a directory, write inode, get data block, write directory entry
 * */
void makeDirectory(char *filename, int fdptr) {
	int fd = fdptr;
	unsigned short parentdirectoryinum;

	struct directoryEntry newdir;
	char fname[14];
	char tempfilename[14];
	strcpy(tempfilename, filename);
	strcpy(fname, tokenizeAbsolute(tempfilename));
	strncpy(newdir.filename, fname, 14);


	if ((parentdirectoryinum = traverseDirectory(filename,fd)) == 0) {
		perror("\nmake directory: directory writing to not found");
		return;
	}

	;
	newdir.inodenum = allocateInode(fd);

	struct inode dirnode;
	dirnode.flags = (32768 | 16384 | 504);
	dirnode.nlinks = 2;
	dirnode.uid = 0;
	dirnode.gid = 0;
	dirnode.size0 = 0;
	dirnode.size1 = 32;
	dirnode.addr[0] = allocateDataBlock(fd);

	//printf("\nmake directory: dirnode.addr[0] = %hu\n", dirnode.addr[0]);

	int i;
	for (i = 1; i < 8; i++)
		dirnode.addr[i] = 0;
	dirnode.actime[0] = 0;
	dirnode.actime[1] = 0;
	dirnode.modtime[0] = 0;
	dirnode.modtime[1] = 0;

	writeiNode(newdir.inodenum, dirnode, fd);
	writeDirectoryEntry(newdir, parentdirectoryinum, fd);

	lseek(fd, 512 * dirnode.addr[0], SEEK_SET);

	struct directoryEntry firstTwoEntries;
	/*first entry '.' points to self*/
	firstTwoEntries.inodenum = newdir.inodenum;
	firstTwoEntries.filename[0] = '.';
	firstTwoEntries.filename[1] = '\0';
	lseek(fd, 512 * dirnode.addr[0], SEEK_SET);
	if (write(fd, &firstTwoEntries, 16) < 16)
		perror("\nmake directoy: fail to write .");

	/*second entry "..". points to parent directory*/
	firstTwoEntries.inodenum = parentdirectoryinum;
	firstTwoEntries.filename[1] = '.';
	firstTwoEntries.filename[2] = '\0';
	if (write(fd, &firstTwoEntries, 16) < 16)
		perror("\nmake directory: fail to write ..");

}
;
/**/

/*Used by initfs to create root node.*/
void createRootDirectory(int fdptr){
	int fd = fdptr;
	struct inode rootnode;
	/*32768 = file allocated, 16384 = directoryEntry, 504 = RWX (user,group)*/
	rootnode.flags = 32768 | 16384 | 504;
	rootnode.nlinks = 2;
	rootnode.uid = 0;
	rootnode.gid = 0;
	rootnode.size0 = 0;
	rootnode.size1 = 32;
	rootnode.addr[0] = allocateDataBlock(fd);
	//printf("\ncreate root directroy: rootnode.addr[0] = %hu\n", rootnode.addr[0]);
	int i;
	for(i = 1; i<8; i++)
		rootnode.actime[i] = 0;
	rootnode.actime[0] = 0;
	rootnode.actime[1] = 0;
	rootnode.modtime[0] = 0;
	rootnode.modtime[1] = 0;

	//printf("\ncreate root directroy: checking rootnode size %hu\n", sizeof(rootnode));
	writeiNode(1, rootnode, fd);

	struct directoryEntry rootdirectory;
	/*first entry '.' points to self*/
	rootdirectory.inodenum = 1 ;
	rootdirectory.filename[0] = '.';
	rootdirectory.filename[1] = '\0';
	lseek(fd, 512*rootnode.addr[0], SEEK_SET);
	if(write(fd, &rootdirectory, 16) != 16)
		perror("\nFailed to write '.'  root entry");

	/*second entry "..". For root, points to self*/
	rootdirectory.filename[1] = '.';
	rootdirectory.filename[2] = '\0';
	if(write(fd, &rootdirectory, 16) != 16)
			perror("\nFailed to write '..'  root entry");

	return;

}/**/

void writeiNode(unsigned short inumber, struct inode node, int fdptr){
	int fd = fdptr;
	unsigned short inum = inumber;
	//printf("\nwriteinode: writing inode for inum = %hu\n", inum);
	if(inum < 1)
		{
			printf("\nwriteinode: i num can not be less than 1. inum = %d\n", inum);
			return;
		}
	if(lseek(fd, 1024 +(inum-1)*32 , SEEK_SET) == -1){
		perror("writeiNode:seek");
	}

	// Re work this test. Testing the same thing twice.
	if(sizeof(node) != 32)
		printf("\nwriteinode: Testing for node size. error node size does not equal 32.\n");
	else{
	int nodesize = write(fd, &node, 32);
	//printf("\nwriteinode: node size = %d", nodesize);
	if(nodesize < 32 )
			perror("Failed to write inode.\n");
	//printf("\nwrite inode number: %u updated\n", inum);
	}
	
}/**/

/*Gets a free block from free array*/
unsigned short allocateDataBlock(int fdptr){
	int fd = fdptr;
	unsigned short blocknum;
	sb.nfree--;

	while(sb.free[sb.nfree] > sb.fsize)
		sb.nfree--;

	blocknum = sb.free[sb.nfree];
	//printf("\nallocate data block:block number %d", blocknum);
	sb.free[sb.nfree] = 0;

	if(sb.nfree == 0){
		unsigned short linkArray[256];
		readShortBlock(blocknum, linkArray, fd);
		sb.nfree = linkArray[0];
		int i;
		for(i=0; i<100; i++)
			sb.free[i] = linkArray[i+1];
	}
	lseek(fd, 512, SEEK_SET);
	write(fd, &sb, sizeof(sb));
	return blocknum;
}/**/

void addBlockToFreeArray(unsigned short blocknum,int fdptr){
	int fd = fdptr;
	if(sb.nfree == 100){
		unsigned short linkArray[256];
		linkArray[0] = 100;
		sb.nfree = 0;
		int i;
		for(i=1; i<101; i++){
			linkArray[i] = sb.free[sb.nfree];
			sb.nfree++;
		}
		writeShortBlock(blocknum, linkArray, fd);
		}
	sb.free[sb.nfree] = blocknum;
	sb.nfree++;

	lseek(fd, 512, SEEK_SET);
	write(fd, &sb, sizeof(sb));
}/**/

unsigned short allocateInode(int fdptr)
{
int fd = fdptr;
unsigned short inum;
unsigned short i = 0;

sb.ninode--;
//printf("\nallocate inode: sb.ninode = %d", sb.ninode);
while(sb.inode[sb.ninode] > sb.isize*16)
	sb.ninode--;
inum = sb.inode[sb.ninode];
//printf("\nallocate inode: sb.inode[sb.ninode] = %d\n", sb.inode[sb.ninode]);

lseek(fd, 1024+(inum-1)*32, SEEK_SET);
struct inode node;
read(fd, &node, sizeof(node));
unsigned short ft = node.flags;

//printf("\nallocate inode: node.flags = %d", ft);
//ft = (ft >> 15);
if((ft && 32768) == 1)
	{
	//printf("\nallocate inode: inode was previously allocated.\n");
	inum = allocateInode(fd);
	}

if(inum == 0){
	/*i list is empty. refill i list with available inodenum*/
}
lseek(fd, 512, SEEK_SET);
write(fd, &sb, sizeof(sb));
return inum;
};/**/


void copyIn(char *srcFilename, char *dstFilename, int fdptr)
{
	int fd = fdptr;
	int srcfd;
	int dstfd;
	char readbuf[512]={0};
	int bytesread;
	char fname[16];

	if((srcfd = open(srcFilename, O_RDWR)) == -1)
	{
		perror("copy in error: opening source file ");
		return;
	}
	else
	{
		//printf("\ncopy in: opened source file");
		strcpy(fname,dstFilename);
	}

	struct inode node;
	unsigned short length = lseek(srcfd, 0, SEEK_END);
	node.size1=length;
	lseek(srcfd, 0L, SEEK_SET);

	unsigned short directoryinumber;
	directoryinumber = traverseDirectory(dstFilename, fd);
	if(directoryinumber < 1)
	{
		printf("\ncopy in: error invlaid directory i number \n");
		return;
	}
	/* I implemented a different way to check for duplicate filenames and
	modify the file name if there is a duplicate. Maybe revise this
	to instead just cancel the copy , but see write directory entry
	for implementation.
	if((checkDirectoryForDuplicate(dstFilename, directoryinumber, fd)) == -1)
		{
			printf("\ncopyin: duplicate file found\n");
			return;
		}*/
	struct directoryEntry d;
	d.inodenum = allocateInode(fd);

	if(d.inodenum < 1 || d.inodenum > sb.isize*16)
	{
		perror("copy in error: inode allocation");
		return;
	}

	
	
	strncpy(d.filename, tokenizeAbsolute(fname), 14);


	node.flags = (32768 | 0) ;
	node.nlinks = 1;
	node.uid = '0';
	node.gid = '0';
	node.size0 = '0';
	node.actime[0] = 0;
	node.actime[1] = 0;
	node.modtime[0] = 0;
	node.modtime[1] = 0;
	int j;
	for(j = 0; j < 7; j++)
	{
		node.addr[j] = 0;
	}

	unsigned short filesize = 0;
	int i = 0;
	unsigned short newblocknum;

	while (1) {
		memset(readbuf, '\0', 512);
		bytesread = read(srcfd, readbuf, 512);
		if (bytesread > 0) {
			newblocknum = allocateDataBlock(fd);
			writeCharBlock(newblocknum, readbuf, fd);
			node.addr[i] = newblocknum;
			i++;
			filesize += bytesread;
			if (bytesread < 512) {
				break;
			}
		}
		else
		{
			break;
		}
		

	}


	//printf("\ncopy in: filesize: %u", filesize);

	//printf("\ncopy in: directory inumber = %u", directoryinumber);
	writeiNode(d.inodenum, node, fd);
	writeDirectoryEntry(d, directoryinumber, fd);
	return;



};/**/

void copyOut(char *srcFilename, char *dstFilename, int fdptr) {
	int fd = fdptr;
	int srcptr, dstptr;
	unsigned short inumber;
	char readbuffer[512] = { '\0' };

	struct inode directorynode;
	unsigned short dirinum;

	char fname[64];
	char sourcefn[64];
	strcpy(sourcefn, srcFilename);
	strcpy(fname, tokenizeAbsolute(sourcefn));

	dirinum = traverseDirectory(srcFilename, fd);
	if(dirinum < 1)
	{
		printf("\ncopy out: error invlaide directory i number\n");
	}
	//printf("\ncopy out: parent directory i number %u\n", dirinum);
	lseek(fd, 1024+(dirinum-1)*32, SEEK_SET);
	read(fd, &directorynode, 32);

	struct directoryEntry d;
	int k, j;
	int found = 0;


	/*checks directory for file*/
	for (k = 0; k < 7; k++) {
		lseek(fd, 512 * directorynode.addr[k], SEEK_SET);
		for (j = 1; j <= 32; j++) {
			read(fd, &d, 16);

			if (strcmp(d.filename, fname) == 0) {

				inumber = d.inodenum;
				//printf("\ncopy out: inode# %u", inumber);
				//printf("\ncopy out: succesfully found file %s\n", srcFilename);
				found = 1;
				break;
			}
		}
		if (found == 1)
			break;
	}

	if(found == 0)
	{
		printf("\ncopy out: file not found\n");
			return;
	}
	memset(readbuffer, '\0', 512);
	int bytesread = 0;

	if((dstptr = open(dstFilename, O_RDWR | O_CREAT, 0600)) == -1)
	{
		printf("\nerror opening file: %s\n", dstFilename);
		return;
	}

	for (k = 0; k < 8; k++) {
		lseek(fd, 1024 + (inumber - 1) * 32, SEEK_SET);

		struct inode n;
		read(fd, &n, 32);

		if ((n.addr[k] != 0)) {
			int  j = readCharBlock(n.addr[k], readbuffer, fd);
			write(dstptr, readbuffer, j);
		}
		else
			break;
	}

}

/**/

/*writes an entry into directory file.*/
void writeDirectoryEntry(struct directoryEntry d,unsigned short inumber,int fdptr)
{
	int fd = fdptr;
	int inumofDirectory = inumber;
	struct inode nodeofDirectory;
	struct directoryEntry checkdir;

	if(inumofDirectory < 1)
	{
		printf("\nwrite directory entry: i number must be greater than 0\n");
	}
	lseek(fd, 1024+(inumofDirectory-1)*32, SEEK_SET);
	read(fd, &nodeofDirectory, 32);

	int k, j;

	int dupfixer = 1;
	int fnlngth = 0;
	fnlngth = strlen(d.filename);
	// Checking for duplicate file and appending to end of file name to correct duplicate name.
	// file name has a max size of 14, so a bit iffey.
	while(checkDirectoryForDuplicate(d.filename, inumofDirectory, fd) == -1)
	{

		if(fnlngth > 10)
		{
			d.filename[11] = '(';
			d.filename[12] = dupfixer + '0';
			d.filename[13] = ')';
		}
		else
		{
			d.filename[fnlngth] = '(';
			d.filename[fnlngth+1] = dupfixer + '0';
			d.filename[fnlngth+2] = ')';
		}
		dupfixer++;
	}
	//printf("\nwrite directory entry: no duplicate file name found\n");

	nodeofDirectory.size1+=16;
	lseek(fd, 1024+(inumofDirectory-1)*32, SEEK_SET);
	write(fd, &nodeofDirectory, 32);


		for (j = 0; j < 7; j++) {
			lseek(fd, 512 * nodeofDirectory.addr[j], SEEK_SET);
			for (k = 0; k < 32; k++) {
				read(fd, &checkdir, 16);
				if(checkdir.inodenum == 0)
				{
					lseek(fd, -16, SEEK_CUR);
					write(fd, &d, 16);
					return;
				}
			}
		}


};

/*absolute path name is passed. Function traverses absolute path name using inode 1 as the hook
 * returns block number of directory where file is located.
 * The absolute path name is tokenized. The "i th" element of array is the last entry of absolute pathname
 * For purposes of project "i th" element of absolute pathname is the filename.
 * The "i - 1" element is the directory we are writing filename to.
 * */
unsigned short traverseDirectory(char *absolutepath, int fdptr) {
	int fd = fdptr;
	/*arg holds array of filenames from absolute path*/
	char *arg[10];
	/*i to hold index of arg*/
	int i = 0;
	/*int n to hold inumber of directory we are searching*/
	unsigned short n;

	arg[i] = strtok(absolutepath, "/");
	//printf("\ntraverse directory:%s", arg[0]);
	i++;
	while (1) {
		if ((arg[i] = strtok(NULL, "/")) == NULL)
			break;
		//printf("\ntraverse: index %d %s", i, arg[i]);
		i++;

	}
	//printf("\ntraverse: %d elements in path name array.", i);

	int indexcounter = (i-1);

	i=0;
	int counter2 = 1;
	/*i to hold index of arg*/

	n = 1;
	int j;
	struct inode inodeofDirectory;
	unsigned short blocknum = 0;

		/*open root*/
		lseek(fd, 1024+(n-1)*32,SEEK_SET);
		read(fd, &inodeofDirectory, 32);
		//printf("\nTraverse directory: opening root node. i number %d", n);

		while(indexcounter != i)
		{
		//printf("\ntraverse: attempting change directory %u, %s", n, arg[i]);
		n=changeDirectory(n, arg[i], fd);
		i++;
		}
		return(n);

}
;
/**/

char* tokenizeAbsolute(char *absolutepath)
{
	char *arg[10];

		//printf("\ntokenizing %s", absolutepath);
		int i = 0;
		arg[i]=strtok(absolutepath, "/");

		//printf("\ntokenize arg[0] %s", arg[i]);

		while(1){
			i++;
			if((arg[i]=strtok(NULL, "/")) == NULL)
			{
				break;
			}
		}
		
		//printf("\n tokenize absolute: filename %s", arg[i]);
		i--;
		//printf("\ntokenize: arg[%d] %s", i, arg[i]);

		int indexnumber = i;
		//printf("\n tokenize absolute: filename %s", arg[indexnumber]);
		return arg[indexnumber];
};/**/


void removefile(char *filename, int fdptr){
	int fd = fdptr;
	bool found = false;

	unsigned short dirinum;
	struct inode parentinode;
	char fn[64];
	char tempfn[64];
	strcpy(tempfn, filename);
	strcpy(fn,tokenizeAbsolute(tempfn));
	dirinum = traverseDirectory(filename, fd);
	//printf("\nremove file: parent directory i number %u\n", dirinum);

	lseek(fd, 1024+(dirinum-1)*32,SEEK_SET);
	read(fd, &parentinode, 32);

	struct directoryEntry d;
	int k, j;
	unsigned short inumdelete;

		/*checks directory for file*/ 
		/*Changed from j < 7 to j <= 7. I think this is correct, but if errors 
		related to remove file occur, possibly it was this change */
		for (j = 0; j < 8; j++) {
			//printf("remove file: checking directory page %d for file\n", j);
			lseek(fd, 512 * parentinode.addr[j], SEEK_SET);
			for (k = 0; k < 32; k++) {
				//printf("remove file: checking addr[%d], entry - %d\n", j, k);
				read(fd, &d, 16);
				if (strcmp(fn, d.filename) == 0) {
					//printf("\nremove file: success found file\n");
					found = true;
					inumdelete= d.inodenum;
					d.inodenum = 0;
					int z;
					for(z=0; z<14; z++)
					{
						d.filename[z]='\0';
					}
					lseek(fd, -16, SEEK_CUR);
					write(fd, &d, 16);
					//printf("\nremove file: directory entry set to %u %s", d.inodenum, d.filename);
					j=8;
					break;

				}
			}
		}
		if(found == true){
		lseek(fd, 1024+(inumdelete-1)*32,SEEK_SET);
		struct inode deleteinode;

		read(fd, &deleteinode, 32);

		char emptybuffer[512];
		memset(emptybuffer, '\0', 512);
		deleteinode.flags = (deleteinode.flags & 000000000000000);
		//printf("\nremove file: i node %u flags set to %u\n", inumdelete, deleteinode.flags);
		for(j=0;j<7;j++){
			if(deleteinode.addr[j] != 0)
			{
			//printf("remove file: filling block number %d with zero", deleteinode.addr[j]);
				/*write empty buffer to blocknumber*/
				unsigned short tempblocknum = deleteinode.addr[j];
				addBlockToFreeArray(tempblocknum,fd);
				writeCharBlock(tempblocknum, emptybuffer, fd);
				deleteinode.addr[j]=0;
			}
		}
		lseek(fd, 1024+(inumdelete-1)*32,SEEK_SET);
		write(fd, &deleteinode, 32);
		}
		else
		{
			printf("\nremove file: file not found\n");
		}

return;
};/**/

unsigned short changeDirectory(unsigned short inumber, char *filename, int fdptr)
{
	int fd = fdptr;
	unsigned short n = inumber;
	unsigned short newinumber;
	unsigned short blocknum;
	struct inode inodeofParentDir;
	lseek(fd, 1024+(n-1)*32,SEEK_SET);
	read(fd, &inodeofParentDir, 32);
	//printf("\nchange directory: opening inode number %d", n);

	unsigned short ft = 24576;
	if ((inodeofParentDir.flags & ft) == 16384) {
		//printf("\nchange directory: %s is a directory", filename);
	}
	else {
		//printf("\nchange directory: %s is not a directory", filename);
		return (0);
	}

			int j;
			/*j to be index of addr[j]. opens directory page*/
			for(j=0;j < 8;j++)
			{
				//printf("\nchange directory: checking directory file at addr[%d]", j);
				blocknum = inodeofParentDir.addr[j];
				if((blocknum) == 0)
					break;

				struct directoryEntry dEntry;
				lseek(fd, 512*blocknum,SEEK_SET);

				int thirtytwo;
				for(thirtytwo = 0; thirtytwo < 32 ;thirtytwo++)
				{
					read(fd,&dEntry,16);
					//printf("\nchange directory: checking %u %s", dEntry.inodenum , dEntry.filename);

					if(strcmp(dEntry.filename, filename)==0)
					{
						//printf("\nchange directory: %s was found", filename);

						struct inode tempnode;
						lseek(fd, 1024+(dEntry.inodenum-1)*32,SEEK_SET);
						read(fd,&tempnode,32);
						unsigned short ft = 24576;
						if((tempnode.flags & ft)==16384)
						{
							//printf("\nchange directory: %s is a directory", filename);
							newinumber = dEntry.inodenum;
							return(newinumber);

						}
						else
						{
							//printf("\nchange directory: %s is not a directory", filename);
							return(0);
						}

					}


				}


			}
			printf("\nchange directory: %s was not found in inode# %u", filename, n);
			return(0);

};/**/

/*
This method will check all entries of a directory given by inode.
The inode number and name of entry and file type will be displayed to console.
*/
void printDirectoryContent(int inumptr, int fdptr){
	int fd = fdptr;
	unsigned short dirinum = inumptr;
	struct inode dirInode;
	struct directoryEntry dirEntry;

	lseek(fd, 1024+(dirinum-1)*32,SEEK_SET);
	read(fd, &dirInode, 32);
	int k, j;

	printf("Directory Entries for inode number # %d\n", dirinum);
	for (j = 0; j < 8; j++) {
		lseek(fd, 512 * dirInode.addr[j], SEEK_SET);
		for (k = 0; k < 32; k++) {
			read(fd, &dirEntry, 16);
			
			if (strcmp("", dirEntry.filename) != 0) {

				printf("\tInode #: %d ", dirEntry.inodenum);
				printf("\tFile name: %s\n", dirEntry.filename);
				int tempfd = fd;
				int tempinum = dirEntry.inodenum;
				//checkFileTypeFunction(tempinum, tempfd);
				

			}
		}
	}
	
	printf("\n");
};/**/

int checkDirectoryForDuplicate(char *filename, unsigned short inumber, int fdptr){
	int fd = fdptr;
	int inumofDirectory = inumber;
	char fname[16];
	struct inode nodeofDirectory;
	struct directoryEntry checkdir;

	strcpy(fname, filename);

		if(inumofDirectory < 1)
		{
			perror("\ncheckdirectoryforduplicate: i number must be greater than 0\n");
		}
		
		lseek(fd, 1024+(inumofDirectory-1)*32, SEEK_SET);
		read(fd, &nodeofDirectory, 32);

		int k, j;

		for (j = 0; j < 8; j++) {
			lseek(fd, 512 * nodeofDirectory.addr[j], SEEK_SET);
		
			for (k = 0; k < 32; k++) {
				read(fd, &checkdir, 16);
			
				if (strcmp(filename, checkdir.filename) == 0) {
					printf("\ncheckdirectoryforduplicate: duplicate file found in directory\n");
					return -1;
				}
			}
		}
	
	return 1;
};/**/

