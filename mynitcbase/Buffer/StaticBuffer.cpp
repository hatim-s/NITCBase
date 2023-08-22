#include "StaticBuffer.h"

// the declarations for this class can be found at "StaticBuffer.h"

unsigned char StaticBuffer::blocks[BUFFER_CAPACITY][BLOCK_SIZE];
struct BufferMetaInfo StaticBuffer::metainfo[BUFFER_CAPACITY];

StaticBuffer::StaticBuffer(){
	// initialise all blocks as free
	for (int bufferIndex = 0; bufferIndex < BUFFER_CAPACITY; bufferIndex++) {
		metainfo[bufferIndex].free = true;
	}
}

/*
At this stage, we are not writing back from the buffer to the disk since we are
not modifying the buffer. So, we will define an empty destructor for now. In
subsequent stages, we will implement the write-back functionality here.
*/
StaticBuffer::~StaticBuffer() {}

int StaticBuffer::getFreeBuffer(int blockNum) {
	if (blockNum < 0 || blockNum > DISK_BLOCKS) return E_OUTOFBOUND;

	int allocatedBuffer = 0;

	// TODO: iterate through all the blocks in the StaticBuffer
	// TODO: find the first free block in the buffer (check metainfo)
	// TODO: assign allocatedBuffer = index of the free block
	for (; allocatedBuffer < BUFFER_CAPACITY; allocatedBuffer++)
		if (metainfo[allocatedBuffer].free) break;

	if (allocatedBuffer == BUFFER_CAPACITY) return FAILURE;

	metainfo[allocatedBuffer].free = false;
	metainfo[allocatedBuffer].blockNum = blockNum;

	return allocatedBuffer;
}

//* Get the buffer index where a particular block is stored or E_BLOCKNOTINBUFFER otherwise
int StaticBuffer::getBufferNum(int blockNum) {
	// Check if blockNum is valid (between zero and DISK_BLOCKS)
	// and return E_OUTOFBOUND if not valid.
	if (blockNum < 0 || blockNum >= DISK_BLOCKS) return E_OUTOFBOUND;

	// find and return the bufferIndex which corresponds to blockNum (check metainfo)
	for (int bufferBlock = 0; bufferBlock < BUFFER_CAPACITY; bufferBlock++){
		if (metainfo[bufferBlock].free == false 
			&& metainfo[bufferBlock].blockNum == blockNum) 
			return bufferBlock;
	}

	//! if block is not in the buffer
	return E_BLOCKNOTINBUFFER;
}