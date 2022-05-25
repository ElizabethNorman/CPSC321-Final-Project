#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "DiskSim.h"
#include <string.h>
#include <stdbool.h>

/* Here is a Disk Simulator.
 * Elizabeth Norman, 230139232
 */

struct Block
{
	unsigned char data [blockSize];

}block;

//do not shame me. or do. i deserve it.
struct Block disk[totalBlocks];

int partition() 
{
	printf("partitioning disk\n");
	createSuper(); //create our super
	initInodeMap(); //create and initalize all maps
	initDataMap();
	initIndirectMap();
	initDirectoryMap();
}

//create super block
int createSuper()
{
	struct Block super; 
	
	char temp[4];

	//magic, numdata/inodes values can be found in the h file
	//this is probably shameful but the conversion allows us to meet the requirements
	//of keeping this to 4 byte values :) 
	convert(magicNum, &temp);
	for (int i = 0; i < 4; i ++)
	{
		super.data[i] = temp[i];
	}
	convert(numDataBlocks, &temp);
	for (int i = 4; i < 8; i ++)
	{
		super.data[i] = temp[i];
	}
	convert(numInodes, &temp);
	for (int i = 8; i < 12; i++)
	{
		super.data[i] = temp[i];	
	}

	diskWrite(super, superBlockLoc); //placed onto disk
}

//create and initalize inode map
int initInodeMap()
{
	struct Block inodeMap;

	for (int i = 0; i < blockSize; i ++)
	{
		inodeMap.data[i] = 0x00;
			
	}
	diskWrite(inodeMap, inodeMapLoc);
}

//create data map 
int initDataMap()
{
	struct Block dataMap;
	
	for (int i = 0; i < blockSize; i++)
	{
		dataMap.data[i] = 0x00;
	}
	diskWrite(dataMap, dataMapLoc);
}

//create map of indirect blocks
int initIndirectMap()
{
	struct Block indirectMap;

	for (int i = 0; i < blockSize; i++)
	{
		indirectMap.data[i] = 0;
	}
	diskWrite(indirectMap, indirMapLoc);
}

//Looking at this list of functions as I add in this one, I absoutely see 
//that this can be refactored. May do if I have time, probably won't be able to.

//create directory map
int initDirectoryMap() 
{
	struct Block directoryMap;

	for (int i = 0; i < blockSize; i++)
	{
		directoryMap.data[i] = 0;
	}
	diskWrite(directoryMap, directMapLoc);
}

//we need to convert the ints to char arrays to be placed in data blocks
//this allows us to meet the requirements of things like magic being a 4 byte value :)
int convert(unsigned int x, char y[])
{
	for (int i = 3; i >= 0; i --)
	{	
		y[3-i]  = ((x >> (i*8)) & 0xFF);
	}
}

/** This little function ensures a block is all empty before it's used incase we are recycling a block
 * that has already been used to store a file!
 */
struct Block* clearBlock(struct Block *blk)
{
	for (int i = 0; i < blockSize; i++)
	{
		blk->data[i] = '\0';
	}
	return blk;
}

/** makeFile creates an inode block and affixes it to the disk
 */
int makeFile(int dirIndex)
{
	struct Block *dirM = diskRead(directMapLoc); //get directory map

	if (dirM->data[dirIndex] != 1) //validate user input
	{
		printf("directory does not exist.");
		return -1;
	}

	struct Block *dir = diskRead(dirIndex + directoryStart); //if directory exists, bring it in	
	
	if (strlen(dir->data) + 4 > blockSize) //if the directory would be over its capacity with this inode, cancel process 
	{
		printf("directory is full!\n");
		return -1;
	}
		
	int seek = 0;
	int locale = 0;
	bool spot = false;

	struct Block *imap = diskRead(inodeMapLoc); //bring in the map

	int inodeID = 0;

	for(int i = 0; i < numInodes/8; i ++) //find an unallocated inode in the map
	{
		if (imap->data[i] < 255) 
		{	
			spot = true;
			for (int j = 0; j < 8; j ++)
			{
				if (getBit(imap->data[i], j) == 0)//find the location
				{
					locale = j;
					inodeID = i*8+j;
					setBit1(&imap->data[i], j); //since there is a spot, mark it as 1 on map
					printf("file created in directory %d. inode num is: %d\n", dirIndex,inodeID );	
					break;
				}			
			}
			if (spot) //if we have a place for a new inode, create new inodes
			{
				struct Block tempi;
				clearBlock(&tempi);	
				int inodeIndex = i*8 + locale + inodeStart;
								
				diskWrite(tempi, inodeIndex);//write inode to disk
						
				char dirData[200]; //affix inode address to directory
				sprintf(dirData, "%s%d ", dir->data, inodeID+inodeStart);
				memcpy(dir->data, dirData, 200);
				printf("updated directory data: %s\n", dir->data);
				
				break;
			}
		}
	}
	if (!spot) //if a spot was not found, then no more inodes exist
	{
		printf("no more inodes exist :( sorry\n");
		return -1;
	}		
}

/** writeFile writes data to a particular inode
 */
int writeFile(char *input, int inodeLoc)
{
	
	struct Block *iMap = diskRead(inodeMapLoc); //acquire necessary blocks 
	struct Block *dMap = diskRead(dataMapLoc);

	//first we will validate the user's input
	if (inodeLoc < 0 || inodeLoc > 127)
	{
		printf("invalid inode location.\n");
		return -1;
	}
	else if (getBit(iMap->data[inodeLoc/8], inodeLoc % 8) != 1)
	{
		printf("file has not been created yet at this location\n");
		return -1;
	}
	else //if everything is ok, we proceed
	{
		printf("writing to file!\n"); //inform user that input was valid

		int reqBlocks  = strlen(input) / blockSize + 1;	
		
		if (reqBlocks > maxFileSize) //if required 
		{
			printf("nevermind. too large of a file\n");
			return -1;
		}

		struct Block *inode = diskRead(inodeLoc + inodeStart); //aquire the inode for this file	
		
		//embrace the jank
	 	char *size = malloc(sizeof(char*));
		sprintf(size, "%d", reqBlocks * blockSize);	
		memcpy(inode->data, size, sizeof(char*));
		free(size);
		
		//prepare for indirect block need
		bool indirectNeed = false;
		int indirIndex = 0;
		struct Block indirBlock; //for writing earlier, can't be more inner scoped than this
			
		if (reqBlocks > 4)
		{			
			clearBlock(&indirBlock); //clear block incase its from an old deleted file

			indirectNeed = true;
			struct Block *indirMap = &disk[indirMapLoc]; 
			for (int i = 0; i < blockSize; i++) //find a free block
			{
				if (indirMap->data[i] == 0)
				{
					indirIndex = i + indirectBStart;
					break;	
				}
			}		
		}

		//this isn't CPSC200, right?
		for (int k = 0; k < reqBlocks; k++)//first loop: go through all needed blocks
		{
			int bitLocale;
			bool foundb = false;

			for (int i = 0; i < blockSize; i ++)//second loop: find a free data block
			{
				if (dMap->data[i] < 255)//does a byte have a spare bit? 
				{
					foundb = true;
					for (int j = 0; j < 8; j ++)//third loop: find a bit
					{
						if (getBit(dMap->data[i], j) == 0)//find the location
						{	
							bitLocale = j;
							setBit1(&dMap->data[i], j);
							break;
						}	
					}
				}
				
				if (foundb) //if a free bit has been found, proceed with file write
				{	
					int blockIndex = i*8 + bitLocale + databStart; //locate the index on disk				
					struct Block temp; //create a temp block to be added to the disk	
					clearBlock(&temp); //clear block 					
					char ctemp[blockSize]; //temporary array to affix necessary data to blocl
					strncpy(ctemp, input + blockSize*k, blockSize);	//the + allows us to trim input as needed
					memcpy(temp.data, ctemp, blockSize); //copy trimmed data to block
					diskWrite(temp, blockIndex); //write block's data to disk
					
					//inode time
					if (k < 4) //direct pointer time
					{		
						char temps[400]; //temp array
						sprintf(temps, "%s %d", inode->data, blockIndex); //copy index to array
						memcpy(inode->data, temps, 128*4); //copy array to inode				
					}
					else //if we have reached our max of four direct pointers, we work on indirect pointers
					{
						if (indirectNeed == true) //validate that we are accessing the else ONLY in this case
						{
							char indirtemp[400]; //again, temp.
							sprintf(indirtemp,"%s %d", indirBlock.data, blockIndex); //same process as above
							memcpy(indirBlock.data, indirtemp, 128*4);
						}	
					}
					break;
				}						
			}
			if (!foundb) //if a bit was not found, then all data blocks are used up
			{
				printf("looks like we are out of room. delete something!\n");
			}
		}
		if (indirectNeed == true) //handle indirect pointer
		{	
			diskWrite(indirBlock, indirIndex); //write indirect pointer to disk
			char indirForInode[400]; //affix indirect pointer's address to the inode
			sprintf(indirForInode, "%s %d", inode->data, indirIndex);
			memcpy(inode->data, indirForInode, 128*4);
		}	
	}
}

/** DeleteFile simply deallocates an inode from the map and the corresponding data blocks unallocated from their map
 * This is computationally stupid, but it works
 */
int deleteFile(int inodeLoc)
{	
	if (inodeLoc < 0 || inodeLoc > 127) //first we will validate input from the user
	{
		printf("input a real inode location\n");
		return -1;
	}
	
	int iindex = inodeLoc/8;
	int ibitloc = inodeLoc%8;
	struct Block *imap = diskRead(inodeMapLoc);

	if(getBit(imap->data[iindex], ibitloc) != 1)
	{
		printf("file does not exist, cannot be deleted\n");
		return -1;
	}
	struct Block *inode = diskRead(inodeLoc+inodeStart); //get necessary blocks for processing
	struct Block *dmap = diskRead(dataMapLoc);	

	char *indexes = strtok(inode->data, " ");
	
	int count = 0;
	int index = 0;
	int mapIndex = 0;
	int bitLocale = 0;
	while(indexes != NULL) //lets deallocate some data blocks
	{
		index = atoi(indexes); //this converts a string to a number!
		if (count > 0 && count < 5)//we want to skip over first thing since that is a size
		{
			mapIndex = (index-databStart)/8;
			bitLocale = ((index-databStart)%8);			
			setBit0(dmap[mapIndex].data, bitLocale);	
		}		
		indexes = strtok(NULL, " ");
		count ++;
	}
	if (count > 4) //I am very unhappy with this. Will make less repetitive if time permits
	{
		struct Block *indmap = diskRead(indirMapLoc);
		indmap->data[index - indirectBStart] = 0;
		struct Block *indir = diskRead(index);		
		char *indirindexes = strtok(indir->data, " ");
		while(indirindexes != NULL)
		{
			index = atoi(indirindexes);
			mapIndex = (index-databStart)/8;
			bitLocale = (index-databStart)%8;
			setBit0(dmap[mapIndex].data, bitLocale);
			indirindexes = strtok(NULL, " ");			
		}
	}

	setBit0(imap[iindex].data, ibitloc);
	printf("file succesfully deleted. inode at location: %d is %d (should read zero)\n", inodeLoc, getBit(imap->data[iindex], ibitloc));
	//i confirm to user that the inode is deallocated
}

/** readFile was not listed in the assignment description but I added it out of spite from it not being listed
 * It is just an edited version of the same code from deleteFile. 
 */
int readFile(int inodeLoc)
{
	struct Block *imap = diskRead(inodeMapLoc);

	if (inodeLoc < 0 || inodeLoc > 127)
	{
		printf("input a real inode location\n");
		return -1;
	}
	int iindex = inodeLoc/8;
	int ibitloc = inodeLoc%8;
	struct Block *inode = diskRead(inodeLoc+inodeStart);

	if(getBit(imap->data[iindex], ibitloc) != 1)
	{
		printf("file does not exist, cannot be deleted\n");
		return -1;
	}
	char *indexes = strtok(inode->data, " ");
	
	int count = 0;
	int index = 0;
	int mapIndex = 0;
	int bitLocale = 0;
	while(indexes != NULL) //looping through indexes for printing
	{
		index = atoi(indexes); //this converts a string to a number!
		if (count > 0 && count < 5)//we want to skip over first thing since that is a size
		{
			struct Block *temp = diskRead(index);
			printf("%s", temp->data);
		}		
		indexes = strtok(NULL, " ");
		count ++;
	}
	if (count > 4) //This code is just copied and edited from deleteFile, I am still unhappy with it
	{
		struct Block *indmap = diskRead(indirMapLoc);
		struct Block *indir = diskRead(index);		
		char *indirindexes = strtok(indir->data, " ");
		while(indirindexes != NULL)
		{
			index = atoi(indexes);
			struct Block *temp = diskRead(index);
			printf("%s", temp->data);
			indirindexes = strtok(indir->data, " ");
		}
	}
	printf("\n");
}

/** createDirectory allows us to make directories based on the location of a parent directory
 */
int createDirectory(int parent)
{
	struct Block *directoryMap = diskRead(directMapLoc);
	struct Block directory;

	if (directoryMap->data[0] == 0)//special case, root will be created first
	{
		printf("creating root directory. ID is 0\n");
		directoryMap->data[0] = 1;
		char id[blockSize];
		sprintf(id, "%d ", -1);
		memcpy(directory.data, id, blockSize);
		diskWrite(directory, directoryStart);

		return 0;
	}	
	
	if (parent < 0 || parent > 127) //validate user input before proceeding
	{
		printf("invalid directory ID\n");
		return -1;
	}
	if (directoryMap->data[parent] != 1)
	{
		printf("parent directory does not exist yet!\n");
		return -1;
	}
	
	int dirIndex = 0;

	for (int i = 1; i < blockSize; i++) //find empty spot for directory
	{
		if (directoryMap->data[i] == 0)
		{
			dirIndex = i + directoryStart;
			printf("making directory. ID is: %d Parent is: %d\n", i, parent);
			directoryMap->data[i] = 1;		
			char id[blockSize];
			sprintf(id, "%d ", parent);
			memcpy(directory.data, id, blockSize);
			diskWrite(directory, dirIndex);
			break;
		}	
	}

	if (dirIndex == 0) //if an index cannot be found, max directories have been reached
	{
		printf("no more directories can be made!\n");
		return -1;
	}	
}

/* diskWrite simply writes the block struct to the overall array of block structs
 * The block's content is determined elsewhere 
 */
int diskWrite(struct Block writeb, int index)
{
	if (index < 0 || index > totalBlocks)
	{
		printf("a block has failed to write\n");
		return -1;
	}
	else
	{
		printf("writing to index %d\n", index);
		disk[index] = writeb;
	}
}

/*  diskRead returns a pointer to a specified block on the disk
 *  With both read and write, there are certainly many edge cases the user could break
 *  the system with. the user is not able to access these functions at all, however
 */
struct Block* diskRead(int index)
{
	if (index < 0 || index > totalBlocks)
	{
		printf("disk reading has failed\n");
		return NULL;
	}
	else
	{
		return &disk[index];
	}
}


/* The following three functions allow us to work with the data bitmap to find
 * specific bits and set them accordingly
 */
int getBit(char bits, int position)
{
	return (bits >> position) & 0x01;
}

int setBit1(char* bits, int position)
{
	*bits |= 1 << position;
}

int setBit0(char* bits, int position)
{
	*bits &= ~(1 << position);
}
