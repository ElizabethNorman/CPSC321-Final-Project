/** just a nice litte h file for the assignment :)
 * Elizabeth Norman 230139232
 */

#ifndef DISKH
#define DISKH

//the following is some useful numbers! 
#define magicNum 0xf0f03410
#define numDataBlocks 1024
#define numInodes 128
//super+map+map + inode+ data +indir
#define totalBlocks 1411
#define blockSize 128

#define indirectBlockNum 128

#define maxFileSize 36 //this is in blocks

//the following are the hardcoded locations in the disk
#define superBlockLoc 0
#define inodeMapLoc 1
#define dataMapLoc 2
#define inodeStart 3
#define databStart 131
#define indirMapLoc 1155
#define indirectBStart 1156


#define directoryStart 1284
#define directMapLoc 1283

int deleteFile();
int writeFile(char *input, int inodeLoc);
int makeFile(int dir);
int partition();
int createSuper();
int convert();
struct Block* diskRead(int index);
int diskWrite(struct Block writeb, int index);

int initInodeMap();
int initDataMap();
int initIndirectMap();
struct Block* clearBlock(struct Block *blk);
int initDirectoryMap();
int createDirectory(int parent);
int getBit(char bits, int position);
int setBit1(char* bits, int position);
int setBit0(char *bits, int position);
int readFile(int ind);


#endif
