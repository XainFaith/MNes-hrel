#include <stddef.h>
#include <stdint-gcc.h>

#define MB 1024 * 1024

extern uint32_t __kernel_end;

struct MemChunk
{
	uint32_t pageCount;
	struct MemChunk * prev;
	struct MemChunk * next;
};

 struct MemChunk * rootMemNode;
 
uint32_t MemFree;
uint32_t MemUsed;

int32_t init_pmm()
{
	uint32_t kstart = (uint32_t)& __kernel_end;
	
	uint32_t avalMem = SYS_MEM ;
	uint32_t sysMem = ((avalMem) * MB) - kstart; //TODO Figure out a decent way to account for space used by kernel and initramdisk
	 MemFree = sysMem;
	 MemUsed = 0;
	 
	 //The root node is set at the 8MB Address 
	 rootMemNode = (struct MemChunk * )(kstart);
	 rootMemNode->prev = NULL;
	 rootMemNode->next = NULL;
	 rootMemNode->pageCount = MemFree / 4096; 	//Each page is 4kb 
	 return 0;	 
}

void MergeChunks(struct MemChunk* prior,struct MemChunk* freed,struct MemChunk* post)
{
	//If there is a chunk prior
    if(prior != NULL)
    {
        //check to see if its boundarys line up with the freed chunk if they do then they need to be merged
        if((((uint32_t)prior) + (prior->pageCount * 4096)) == (uint32_t)freed)
        {
            prior->pageCount += freed->pageCount;
            prior->next = post;

            //Set the freed chunk to be the prior chunk since they have merged
            freed = prior;
        }
    }

    //If there is a chunk post the freed chunk
    if(post != NULL)
    {
        //check to see if its boundarys line up with the freed chunk if they do then they need to be merged
        if((((uint32_t)freed) + freed->pageCount * 4096) == (uint32_t)post)
        {
            freed->pageCount += post->pageCount;
            freed->next = post->next;
        }
	}
}

void * allocPage(uint32_t numOfPages)
{
	if(rootMemNode != NULL)
    {
        //Setup a pointer for iterating over the list
        struct MemChunk * memChunkPtr = rootMemNode;

        //Loop while the pointer for iteration is not null
        while(memChunkPtr != NULL)
        {
            //Is the chunk larger or equal too the number of pages requested
           if(memChunkPtr->pageCount >= numOfPages)
           {
                //is the chunk exactly the correct size
                if(memChunkPtr->pageCount== numOfPages)
                {
                    //If there was a chunk prior to the allocated chunk
                    if(memChunkPtr->prev != NULL)
                    {
                        //Is there a chunk after the allocated chunk
                        if(memChunkPtr->next != NULL)
                        {
                            //Set the prior chunk to point to the next one beyond the allocated chunk
                            memChunkPtr->prev->next = memChunkPtr->next;
                        }
                        else //If there is no chunk beyond the allocated chunk set the prior chunks next link to NULL
                        {
                            memChunkPtr->prev->next = NULL;
                        }
                    }
                    else //If there is no prior chunk this must be the root node
                    {
                        //Is there a chunk after the allocated chunk
                        if(memChunkPtr->next != NULL)
                        {
                            //Set the root chunk to be the chunk beyond the allocated chunnk
                            rootMemNode = memChunkPtr->next;
                        }
                        else
                        {
                            //We are not out of memory
                            rootMemNode = NULL;
							//TODO Return a no mem value from ErrorNo
                        }
                        
                    }
                    
                    MemUsed += numOfPages * 4096;
                    //Return the allocated chunk
                    return (void*)memChunkPtr;
                }
                
                //If the chunk is larger
                if(memChunkPtr->pageCount > numOfPages)
                {
                    //Calculate the address of the break
                    uint32_t baseAddr = (uint32_t)memChunkPtr;
                    uint32_t breakAddr = baseAddr + (numOfPages * 4096);

                    //Create the new chunk data
                    struct MemChunk * newChunkPtr = (struct MemChunk*)(breakAddr);
                    newChunkPtr->next = memChunkPtr->next;
                    newChunkPtr->prev = memChunkPtr->prev;
                    newChunkPtr->pageCount = memChunkPtr->pageCount - numOfPages;
                    
                    //If there is a chunk prior to the allocated one set its next pointer to the new chunk
                    if(memChunkPtr->prev != NULL)
                    {
                        memChunkPtr->prev->next = newChunkPtr;
                    }
                    else //If there is no chunk prior to the allocated on set the new chunk as the root chunk
                    {
                        rootMemNode = newChunkPtr;
                    }
                    
                    MemUsed += numOfPages * 4096;
                    //Return the allocated chunk
                    return (void*)memChunkPtr;
                }

                //Iterate to the next chunk to see if the next one has space for the allocation
                memChunkPtr = memChunkPtr->prev;
           }

        }
    }
    
    return NULL;
}

void freePage(void * addr, uint32_t numOfPages)
{
	    //Sainity check on the values passed in
    if(addr != NULL && numOfPages != 0)
    {
        //Check to see if there is a root chunk
        if(rootMemNode!= NULL)
        {
            uint32_t freeAddr = (uint32_t)addr;

            //Iterate over the list of free chunks 
            struct MemChunk * memChunkPtr = rootMemNode;
            while(memChunkPtr != NULL)
            {
                //If the chunk being returned has an address less the the chunk we are checking its place belongs before that chunk
                uint32_t chunkAddr = (uint32_t)memChunkPtr;
                if(freeAddr < chunkAddr)
                {
                    struct MemChunk * freedPtr = (struct MemChunk*)addr;
                    freedPtr->pageCount = numOfPages;
                    freedPtr->next = memChunkPtr;
                    freedPtr->prev = memChunkPtr->prev;

                    MergeChunks(memChunkPtr->prev, freedPtr, memChunkPtr);
                    break;
                }
            }
        }
        else
        {
            //We ran out of memory and this is the first free call since then, So just set the freed pages chunk as the root chunk
            rootMemNode = (struct MemChunk*)addr;
            rootMemNode->pageCount = numOfPages;
            rootMemNode->prev = NULL;
            rootMemNode->next = NULL;
        }

        //adjust for the freed memory
        MemUsed -= numOfPages * 4096;        
    }
}


