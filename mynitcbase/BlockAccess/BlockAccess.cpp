#include "BlockAccess.h"

#include <cstring>

inline bool operator == (RecId lhs, RecId rhs) {
	return (lhs.block == rhs.block && lhs.slot == rhs.slot);
}

inline bool operator != (RecId lhs, RecId rhs) {
	return (lhs.block != rhs.block || lhs.slot != rhs.slot);
}

RecId BlockAccess::linearSearch(int relId, char attrName[ATTR_SIZE], union Attribute attrVal, int op)
{
	// get the previous search index of the relation relId from the relation cache
	// (use RelCacheTable::getSearchIndex() function)
	RecId prevRecId;
	RelCacheTable::getSearchIndex(relId, &prevRecId);

	// let block and slot denote the record id of the record being currently checked
	int block = -1, slot = -1;

	// if the current search index record is invalid(i.e. both block and slot = -1)
	if (prevRecId.block == -1 && prevRecId.slot == -1)
	{
		//* no hits from previous search; 
		//* search should start from the first record itself

		// get the first record block of the relation from the relation cache
		// (use RelCacheTable::getRelCatEntry() function of Cache Layer)
		RelCatEntry relCatBuffer;
		RelCacheTable::getRelCatEntry(relId, &relCatBuffer);

		// block = first block of the relation,
		// slot = 0 (start at the first slot)
		block = relCatBuffer.firstBlk, slot = 0;
	}
	else
	{
		//* there is a hit from previous search; search should start from
		//* the record next to the search index record

		// block = search index's block
		// slot = search index's slot + 1
		block = prevRecId.block, slot = prevRecId.slot + 1;
	}

	/* The following code searches for the next record in the relation
	   that satisfies the given condition:
		* "We start from the record id (block, slot) and iterate over the remaining
		* records of the relation"
	*/

	RelCatEntry relCatBuffer;
	RelCacheTable::getRelCatEntry(relId, &relCatBuffer);
	while (block != -1)
	{
		// TODO: create a RecBuffer object for block (use RecBuffer Constructor for existing block)
		RecBuffer blockBuffer(block);

		// TODO: get header of the block using RecBuffer::getHeader() function
		HeadInfo blockHeader;
		blockBuffer.getHeader(&blockHeader);

		// TODO: get slot map of the block using RecBuffer::getSlotMap() function
		unsigned char slotMap[blockHeader.numSlots];
		blockBuffer.getSlotMap(slotMap);

		// If slot >= the number of slots per block(i.e. no more slots in this block)
		if (slot >= relCatBuffer.numSlotsPerBlk)
		{
			// TODO: update block = right block of block, update slot = 0
			block = blockHeader.rblock, slot = 0;
			continue; // continue to the beginning of this while loop
		}

		// if slot is free skip the loop
		// (i.e. check if slot'th entry in slot map of block contains SLOT_UNOCCUPIED)
		if (slotMap[slot] == SLOT_UNOCCUPIED)
		{
			slot++;
			continue;
		}

		// TODO: get the record with id (block, slot) using RecBuffer::getRecord()
		Attribute record[blockHeader.numAttrs];
		blockBuffer.getRecord(record, slot);

		// TODO: compare record's attribute value to the the given attrVal as below:
		//* firstly get the attribute offset for the attrName attribute
		//* from the attribute cache entry of the relation using
		//* AttrCacheTable::getAttrCatEntry()

		AttrCatEntry attrCatBuffer;
		AttrCacheTable::getAttrCatEntry(relId, attrName, &attrCatBuffer);

		// use the attribute offset to get the value of the attribute from current record
		int attrOffset = attrCatBuffer.offset;

		// will store the difference between the attributes 
		// set cmpVal using compareAttrs()
		int cmpVal = compareAttrs(record[attrOffset], attrVal, attrCatBuffer.attrType); 

		/* 
		TODO: check whether this record satisfies the given condition.
		* It is determined based on the output of previous comparison and the op value received.
		* The following code sets the cond variable if the condition is satisfied.
		*/
		if (
			(op == NE && cmpVal != 0) || // if op is "not equal to"
			(op == LT && cmpVal < 0) ||	 // if op is "less than"
			(op == LE && cmpVal <= 0) || // if op is "less than or equal to"
			(op == EQ && cmpVal == 0) || // if op is "equal to"
			(op == GT && cmpVal > 0) ||	 // if op is "greater than"
			(op == GE && cmpVal >= 0)	 // if op is "greater than or equal to"
		)
		{
			// TODO: set the search index in the relation cache as
			// TODO: the record id of the record that satisfies the given condition
			// (use RelCacheTable::setSearchIndex function)
			RecId newRecId = {block, slot};
			RelCacheTable::setSearchIndex(relId, &newRecId);

			return RecId{block, slot};
		}

		slot++;
	}

	//! no record in the relation with Id relid satisfies the given condition
	return RecId{-1, -1};
}

int BlockAccess::renameRelation(char oldName[ATTR_SIZE], char newName[ATTR_SIZE]){
    // TODO: reset the searchIndex of the relation catalog using RelCacheTable::resetSearchIndex() 
	RelCacheTable::resetSearchIndex(RELCAT_RELID);

	// TODO: set newRelationName with newName
    Attribute newRelationName;    
	strcpy(newRelationName.sVal, newName);

    // TODO: search the relation catalog for an entry with "RelName" = newRelationName
	RecId searchIndex = BlockAccess::linearSearch(RELCAT_RELID, RELCAT_ATTR_RELNAME, newRelationName, EQ);

    //! If relation with name newName already exists (result of linearSearch is not {-1, -1})
	if (searchIndex != RecId{-1, -1})
       return E_RELEXIST;


    // reset the searchIndex of the relation catalog using RelCacheTable::resetSearchIndex)
	RelCacheTable::resetSearchIndex(RELCAT_RELID);

	// set oldRelationName with oldName
    Attribute oldRelationName;
	strcpy(oldRelationName.sVal, oldName);

    // search the relation catalog for an entry with "RelName" = oldRelationName
	searchIndex = BlockAccess::linearSearch(RELCAT_RELID, RELCAT_ATTR_RELNAME, oldRelationName, EQ);

    //! If relation with name oldName does not exist (result of linearSearch is {-1, -1})
	if (searchIndex == RecId{-1, -1})
       return E_RELNOTEXIST;

    // TODO: get the relation catalog record of the relation to rename using a RecBuffer
    // TODO: on the relation catalog [RELCAT_BLOCK] and RecBuffer.getRecord function
	RecBuffer relCatBlock (RELCAT_BLOCK);
	
	Attribute relCatRecord [RELCAT_NO_ATTRS];
	relCatBlock.getRecord(relCatRecord, searchIndex.slot);

    // TODO: update the relation name attribute in the record with newName.
	strcpy(relCatRecord[RELCAT_REL_NAME_INDEX].sVal, newName);

    // TODO: set back the record value using RecBuffer.setRecord
	relCatBlock.setRecord(relCatRecord, searchIndex.slot);

	// TODO: update all the attribute catalog entries in the attribute catalog corresponding
	// TODO: to the relation with relation name oldName to the relation name newName

    // reset the searchIndex of the attribute catalog using RelCacheTable::resetSearchIndex()
	RelCacheTable::resetSearchIndex(ATTRCAT_RELID);

    //for i = 0 to numberOfAttributes :
	for (int attrIndex = 0; attrIndex < relCatRecord[RELCAT_NO_ATTRIBUTES_INDEX].nVal; attrIndex++) {
		//    linearSearch on the attribute catalog for relName = oldRelationName
		//    get the record using RecBuffer.getRecord
		searchIndex = BlockAccess::linearSearch(ATTRCAT_RELID, ATTRCAT_ATTR_RELNAME, oldRelationName, EQ);
		RecBuffer attrCatBlock (searchIndex.block);

		Attribute attrCatRecord [ATTRCAT_NO_ATTRS];
		attrCatBlock.getRecord(attrCatRecord, searchIndex.slot);

		//    update the relName field in the record to newName
		//    set back the record using RecBuffer.setRecord

		strcpy(attrCatRecord[ATTRCAT_REL_NAME_INDEX].sVal, newName);
		attrCatBlock.setRecord(attrCatRecord, searchIndex.slot);
	}

    return SUCCESS;
}

int BlockAccess::renameAttribute(char relName[ATTR_SIZE], char oldName[ATTR_SIZE], char newName[ATTR_SIZE]) {
    // reset the searchIndex of the relation catalog using RelCacheTable::resetSearchIndex()
	RelCacheTable::resetSearchIndex(RELCAT_RELID);

	// set relNameAttr to relName
    Attribute relNameAttr;
	strcpy(relNameAttr.sVal, relName);

	// Search for the relation with name relName in relation catalog using linearSearch()
	RecId searchIndex = BlockAccess::linearSearch(RELCAT_RELID, RELCAT_ATTR_RELNAME, relNameAttr, EQ);
    
	//! If relation with name relName does not exist (search returns {-1,-1})
	if (searchIndex == RecId{-1, -1})
       return E_RELNOTEXIST;
	
    // reset the searchIndex of the attribute catalog using RelCacheTable::resetSearchIndex()
	RelCacheTable::resetSearchIndex(ATTRCAT_RELID);

    // declare variable attrToRenameRecId used to store the attr-cat recId of the attribute to rename
    RecId attrToRenameRecId{-1, -1};
    // Attribute attrCatEntryRecord[ATTRCAT_NO_ATTRS];

    // TODO: iterate over all Attribute Catalog Entry record corresponding to the
    // TODO: relation to find the required attribute
    while (true) {
        // linear search on the attribute catalog for RelName = relNameAttr
		searchIndex = BlockAccess::linearSearch(ATTRCAT_RELID, ATTRCAT_ATTR_RELNAME, relNameAttr, EQ);

        // if there are no more attributes left to check (linearSearch returned {-1,-1})
		if (searchIndex == RecId{-1, -1}) break;

        // TODO: Get the record from the attribute catalog using 
		// TODO: RecBuffer.getRecord into attrCatEntryRecord
		RecBuffer attrCatBlock (searchIndex.block);

		Attribute attrCatRecord [ATTRCAT_NO_ATTRS];
		attrCatBlock.getRecord(attrCatRecord, searchIndex.slot);

        // if attrCatEntryRecord.attrName = oldName
		if (strcmp(attrCatRecord[ATTRCAT_ATTR_NAME_INDEX].sVal, oldName) == 0){
			attrToRenameRecId = searchIndex;
			break;
		}

        //! if attrCatEntryRecord.attrName = newName
		if (strcmp(attrCatRecord[ATTRCAT_ATTR_NAME_INDEX].sVal, newName) == 0)
            return E_ATTREXIST;
    }

	// if attribute with the old name does not exist
    if (attrToRenameRecId == RecId{-1, -1})
        return E_ATTRNOTEXIST;

    // Update the entry corresponding to the attribute in the Attribute Catalog Relation.
    /*   declare a RecBuffer for attrToRenameRecId.block and get the record at
         attrToRenameRecId.slot */
    //   update the AttrName of the record with newName
    //   set back the record with RecBuffer.setRecord

	RecBuffer attrCatBlock (attrToRenameRecId.block);
	Attribute attrCatRecord [ATTRCAT_NO_ATTRS];
	attrCatBlock.getRecord(attrCatRecord, attrToRenameRecId.slot);
	
	strcpy(attrCatRecord[ATTRCAT_ATTR_NAME_INDEX].sVal,newName );
	attrCatBlock.setRecord(attrCatRecord, attrToRenameRecId.slot);

    return SUCCESS;
}

int BlockAccess::insert(int relId, Attribute *record) {
    // get the relation catalog entry from relation cache
    // ( use RelCacheTable::getRelCatEntry() of Cache Layer)
	RelCatEntry relCatEntry;
	RelCacheTable::getRelCatEntry(relId, &relCatEntry);

    int blockNum = relCatEntry.firstBlk;

    // rec_id will be used to store where the new record will be inserted
    RecId rec_id = {-1, -1};

    int numOfSlots = relCatEntry.numSlotsPerBlk;
    int numOfAttributes = relCatEntry.numAttrs;

	// block number of the last element in the linked list = -1 
    int prevBlockNum = -1;

	// Traversing the linked list of existing record blocks of the relation
	// until a free slot is found OR until the end of the list is reached

    while (blockNum != -1) {
        // create a RecBuffer object for blockNum (using appropriate constructor!)
		RecBuffer blockBuffer (blockNum);

        // get header of block(blockNum) using RecBuffer::getHeader() function
		HeadInfo blockHeader;
		blockBuffer.getHeader(&blockHeader);

        // get slot map of block(blockNum) using RecBuffer::getSlotMap() function
		int numSlots = blockHeader.numSlots;
		unsigned char slotMap [numSlots];
		blockBuffer.getSlotMap(slotMap);

        // search for free slot in the block 'blockNum' and store it's rec-id in rec_id
        // (Free slot can be found by iterating over the slot map of the block)
		int slotIndex = 0;
		for (; slotIndex < numSlots; slotIndex++) {
        	// if a free slot is found, set rec_id and discontinue the traversal
           	// of the linked list of record blocks (break from the loop) 
			//* slot map stores SLOT_UNOCCUPIED if slot is free and SLOT_OCCUPIED if slot is occupied
			if (slotMap[slotIndex] == SLOT_UNOCCUPIED) {
				rec_id = RecId{blockNum, slotIndex};
				break;
			}
		}

		if (rec_id != RecId{-1, -1}) break;

        /* otherwise, continue to check the next block by updating the
           block numbers as follows:
              update prevBlockNum = blockNum
              update blockNum = header.rblock (next element in the linked list of record blocks)
        */
	   prevBlockNum = blockNum;
	   blockNum = blockHeader.rblock;
    }

    //  if no free slot is found in existing record blocks (rec_id = {-1, -1})
	if (rec_id == RecId{-1, -1})
    {
        // if relation is RELCAT, do not allocate any more blocks
        //     return E_MAXRELATIONS;
		if (relId == RELCAT_RELID) return E_MAXRELATIONS;

        // Otherwise,
        // get a new record block (using the appropriate RecBuffer constructor!)
		RecBuffer blockBuffer;

        // get the block number of the newly allocated block
        // (use BlockBuffer::getBlockNum() function)
        blockNum = blockBuffer.getBlockNum();
		
		// let ret be the return value of getBlockNum() function call
        if (blockNum == E_DISKFULL) return E_DISKFULL;

        // Assign rec_id.block = new block number(i.e. ret) and rec_id.slot = 0
		rec_id = RecId {blockNum, 0};

		// TODO: set the header of the new record block such that it links with
		// TODO: existing record blocks of the relation
		// TODO: set the block's header as follows:
		// blockType: REC, pblock: -1
		// lblock = -1 (if linked list of existing record blocks was empty
		// 				i.e this is the first insertion into the relation)
		// 		= prevBlockNum (otherwise),
		// rblock: -1, numEntries: 0,
		// numSlots: numOfSlots, numAttrs: numOfAttributes
		// (use BlockBuffer::setHeader() function)
        
		HeadInfo blockHeader;
		blockHeader.blockType = REC;
		blockHeader.lblock = prevBlockNum, blockHeader.rblock = blockHeader.pblock = -1;
		blockHeader.numAttrs = numOfAttributes, blockHeader.numSlots = numOfSlots, blockHeader.numEntries = 0;

		blockBuffer.setHeader(&blockHeader);
        /*
            set block's slot map with all slots marked as free
            (i.e. store SLOT_UNOCCUPIED for all the entries)
            (use RecBuffer::setSlotMap() function)
        */
	   	unsigned char slotMap [numOfSlots];
		for (int slotIndex = 0; slotIndex < numOfSlots; slotIndex++)
			slotMap[slotIndex] = SLOT_UNOCCUPIED;

		blockBuffer.setSlotMap(slotMap);

        // if prevBlockNum != -1
		if (prevBlockNum != -1)
        {
            // TODO: create a RecBuffer object for prevBlockNum
			RecBuffer prevBlockBuffer (prevBlockNum);

            // TODO: get the header of the block prevBlockNum and
			HeadInfo prevBlockHeader;
			prevBlockBuffer.getHeader(&prevBlockHeader);

            // TODO: update the rblock field of the header to the new block
			prevBlockHeader.rblock = blockNum;
            // number i.e. rec_id.block
            // (use BlockBuffer::setHeader() function)
			blockBuffer.setHeader(&prevBlockHeader);
        }
        else
        {
            // update first block field in the relation catalog entry to the
            // new block (using RelCacheTable::setRelCatEntry() function)
			relCatEntry.firstBlk = blockNum;
			RelCacheTable::setRelCatEntry(relId, &relCatEntry);
        }

        // update last block field in the relation catalog entry to the
        // new block (using RelCacheTable::setRelCatEntry() function)
		relCatEntry.lastBlk = blockNum;
		RelCacheTable::setRelCatEntry(relId, &relCatEntry);
    }

    // create a RecBuffer object for rec_id.block
    RecBuffer blockBuffer (rec_id.block);

	// insert the record into rec_id'th slot using RecBuffer.setRecord())
	blockBuffer.setRecord(record, rec_id.slot);

    /* update the slot map of the block by marking entry of the slot to
       which record was inserted as occupied) */
    // (ie store SLOT_OCCUPIED in free_slot'th entry of slot map)
    // (use RecBuffer::getSlotMap() and RecBuffer::setSlotMap() functions)
	unsigned char slotmap [numOfSlots];
	blockBuffer.getSlotMap(slotmap);

	slotmap[rec_id.slot] = SLOT_OCCUPIED;
	blockBuffer.setSlotMap(slotmap);

    // increment the numEntries field in the header of the block to
    // which record was inserted
    // (use BlockBuffer::getHeader() and BlockBuffer::setHeader() functions)
	HeadInfo blockHeader;
	blockBuffer.getHeader(&blockHeader);

	blockHeader.numEntries++;
	blockBuffer.setHeader(&blockHeader);

    // Increment the number of records field in the relation cache entry for
    // the relation. (use RelCacheTable::setRelCatEntry function)
	relCatEntry.numRecs++;
	RelCacheTable::setRelCatEntry(relId, &relCatEntry);

    return SUCCESS;
}