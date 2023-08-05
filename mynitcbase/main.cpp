#include "Buffer/StaticBuffer.h"
#include "Cache/OpenRelTable.h"
#include "Disk_Class/Disk.h"
#include "FrontendInterface/FrontendInterface.h"

// local headers
#include <iostream>

void printBuffer(unsigned char buffer[], int size)
{
	for (int i = 0; i < size; i++)
	{
		std::cout << (int)buffer[i] << " ";
		if (i % 64 == 63)
			std::cout << "\n";
	}
	std::cout << "\n";
}

int main(int argc, char *argv[])
{
	Disk disk_run;

	// create objects for the relation catalog and attribute catalog
	RecBuffer relCatBuffer(RELCAT_BLOCK);
	RecBuffer attrCatBuffer(ATTRCAT_BLOCK);

	// creating headers for relation catalog and attribute catalog
	HeadInfo relCatHeader;
	HeadInfo attrCatHeader;

	// load the headers of both the blocks into relCatHeader and attrCatHeader.
	// (we will implement these functions later)
	relCatBuffer.getHeader(&relCatHeader);
	attrCatBuffer.getHeader(&attrCatHeader);

	// attrCatBaseSlot stores the base of the slot, 
	// from which offset should be taken
	for (int i = 0, attrCatBaseSlot = 0; i < relCatHeader.numEntries; i++)
	{
		// will store the record from the relation catalog
		Attribute relCatRecord[RELCAT_NO_ATTRS]; 
		relCatBuffer.getRecord(relCatRecord, i);

		printf("Relation: %s\n", relCatRecord[RELCAT_REL_NAME_INDEX].sVal);

		int j = 0;
		for (; j < relCatHeader.numAttrs; j++)
		{
			// declare attrCatRecord and load the attribute catalog entry into it
			Attribute attrCatRecord[ATTRCAT_NO_ATTRS];
			attrCatBuffer.getRecord(attrCatRecord, attrCatBaseSlot + j);

			// if the current attribute belongs to the current relation
			// then we print it, which is checked by comparing names
			if (strcmp(attrCatRecord[ATTRCAT_REL_NAME_INDEX].sVal,
					   relCatRecord[RELCAT_REL_NAME_INDEX].sVal) == 0)
			{
				const char *attrType = attrCatRecord[ATTRCAT_ATTR_TYPE_INDEX].nVal == NUMBER
										   ? "NUM" : "STR";
				printf("  %s: %s\n", attrCatRecord[ATTRCAT_ATTR_NAME_INDEX].sVal, attrType);
			}
		}
		attrCatBaseSlot += j;

		printf("\n");
	}

	return 0;
}