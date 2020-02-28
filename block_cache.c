////////////////////////////////////////////////////////////////////////////////
//
//  File           : block_cache.c
//  Description    : This is the implementation of the cache for the BLOCK
//                   driver.
//
//  Author         : [Ajinkya Mukherjee]
//  Last Modified  : [August 1st 2019]
//

// Includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

// Project includes
#include <block_cache.h>
#include <cmpsc311_log.h>


struct cacheItem
{
    uint16_t frameIndex;
    uint8_t frame[BLOCK_FRAME_SIZE];
    uint32_t time;

};

//Helper functions
static int is_cacheFUll();
static struct cacheItem * get_emptyCacheFrame();
static struct cacheItem * get_oldestCacheFrame();
static struct cacheItem * get_cacheFrame(uint16_t index);

//Global Variables
uint32_t block_cache_max_items = DEFAULT_BLOCK_FRAME_CACHE_SIZE; // Maximum number of items in cache
static struct cacheItem * cache = NULL; // Cache List
static uint32_t cacheSize; // Cache Size
static uint32_t cacheTime; // Cache Time



//
// Functions

////////////////////////////////////////////////////////////////////////////////
//
// Function     : set_block_cache_size
// Description  : Set the size of the cache (must be called before init)
//
// Inputs       : max_frames - the maximum number of items your cache can hold
// Outputs      : 0 if successful, -1 if failure

int set_block_cache_size(uint32_t max_frames)
{
    // int i;
     //int j;
     //int iterate;
     //for(i = 0; i < block_cache_max_items;i++)
     //{
        // max_frames = block_cache_max_items;
        block_cache_max_items = max_frames;
      //}

     
    return (0);
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : init_block_cache
// Description  : Initialize the cache and note maximum frames
//
// Inputs       : none
// Outputs      : 0 if successful, -1 if failure

int init_block_cache(void)
{
   // uint32_t init = (uint32_t) malloc(block_cache_max_items);
    cache = (struct cacheItem*) malloc(sizeof(struct cacheItem)* block_cache_max_items);// Alocating cache array

    if(cache == NULL)
    {
        return -1;
    }
    cacheSize = 0;
    cacheTime = 0;
   // int i;
    //for(i = 0; i < block_cache_max_items ;i++)
   // {
   //     cache[i].isUsed =0;
   // }
    return (0);
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : close_block_cache
// Description  : Clear all of the contents of the cache, cleanup
//
// Inputs       : none
// Outputs      : o if successful, -1 if failure

int close_block_cache(void)
{
    free(cache);
    return (0);
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : put_block_cache
// Description  : Put an object into the frame cache
//
// Inputs       : block - the block number of the frame to cache
//                frm - the frame number of the frame to cache
//                buf - the buffer to insert into the cache
// Outputs      : 0 if successful, -1 if failure

int put_block_cache(BlockIndex block, BlockFrameIndex frm, void* buf)
{
    struct cacheItem * item;
    
    item = get_cacheFrame(frm); // Searching for the frame
    
    if(item == NULL)// Frame not found
    {
        if(is_cacheFUll())// If cache is full
        {
            item = get_oldestCacheFrame(); // Replace it with the oldes frame
        }
        else // If cache is not full 
        {
            item = get_emptyCacheFrame(); // add the frame to the end of the cache
        }
        
    }

    if(item == NULL)
    {
        return -1;
    }
    item->time = cacheTime++;
    //item->frame = frm;
    item->frameIndex = frm;
    memcpy(item->frame, buf, BLOCK_FRAME_SIZE);

    return (0);
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : get_block_cache
// Description  : Get an frame from the cache (and return it)
//
// Inputs       : block - the block number of the block to find
//                frm - the  number of the frame to find
// Outputs      : pointer to cached frame or NULL if not found

void* get_block_cache(BlockIndex block, BlockFrameIndex frm)
{
    struct cacheItem * item;

    item = get_cacheFrame(frm); //Getting cache item from frame index frm

    if(item == NULL) //MISS
    {
        return NULL;
    }

    item->time = cacheTime++; //If frame found update the time

    //return (NULL);
    return item->frame;
}


 struct blockCacheTest
{
    uint16_t frameIndex;
    uint8_t frame[BLOCK_FRAME_SIZE];
};


//
// Unit test

////////////////////////////////////////////////////////////////////////////////
//
// Function     : blockCacheUnitTest
// Description  : Run a UNIT test checking the cache implementation
//
// Inputs       : none
// Outputs      : 0 if successful, -1 if failure

int blockCacheUnitTest(void)
{
     struct blockCacheTest *blockTest;
    int testIndex = 0;
    int operation;
    int frameIdx;
    int error;
    uint8_t *cacheFrame;
    uint8_t *testFrame;

   
    srand(time(NULL));//Initializing Random

    
    blockTest = (struct blockCacheTest *) malloc(10000 * sizeof(struct blockCacheTest));//Initializing Helper array
    if(blockTest == NULL) 
    {
        logMessage(LOG_ERROR_LEVEL, "Unable to initialize helper array.");
        return -1;
    }


    
    if(init_block_cache() == -1) // INitialiazing Cache
    {
        logMessage(LOG_ERROR_LEVEL, "Unable to initialized the cache.");
        return -1;
    }

    

    for(int i = 0; i < 10000; i++)// Test Cache implementation with 10,000 operation
     {

        
        operation = rand() % 2;//Pick random operations 0 is get and 1 is put
        frameIdx = rand() % BLOCK_BLOCK_SIZE;//Picking random frame

      
        
        if(operation == 0) // Do the operations
        {
            // Getting operation 

            
            cacheFrame = get_block_cache(0, frameIdx);// Doing cahce get

            
            testFrame = NULL;
            for(int i = testIndex -1; i >= 0; i--)//Search for the last frame in the array with a matching index 
            {
                if(blockTest[i].frameIndex == frameIdx) 
                {
                    testFrame = blockTest[i].frame;
                    break;
                }
            }

            if(cacheFrame != NULL) 
            {
                if(testFrame == NULL) 
                {
                    //Error->got a cache frame not added by the code
                    logMessage(LOG_ERROR_LEVEL, "Get block cache retrived frame not added.");
                    return -1;
                }


                //Checking if the actual data is the same 
                if(memcmp(testFrame, cacheFrame, BLOCK_FRAME_SIZE) != 0) 
                {
                    logMessage(LOG_ERROR_LEVEL, "Get block cache data is diferent from expected.");
                    return -1;
                }
            }            

        } 
        else//put Operation
         {
            

            // Saving frame to the array 
            blockTest[testIndex].frameIndex = frameIdx;
            memset(blockTest[testIndex].frame, 0, BLOCK_FRAME_SIZE);
            sprintf((char *) blockTest[testIndex].frame, "TEST %d", frameIdx);

            // Do cache put 
            error = put_block_cache(0, frameIdx, blockTest[testIndex].frame);
            if(error)
             {
                logMessage(LOG_ERROR_LEVEL, "Put block cache error.");
                return -1;
            }

            testIndex++;
        }
    }

    // Closing cache
    if(close_block_cache() == -1) 
    {
        logMessage(LOG_ERROR_LEVEL, "Unable to close the cache.");
        return -1;
    }

    // Return successfully
    logMessage(LOG_OUTPUT_LEVEL, "Cache unit test completed successfully.");
    return (0);
}


//Helper functions
static int is_cacheFUll()// Checking if cache is full
{
    if(cacheSize != block_cache_max_items)
    {
        return 0;
    }
    return  1;
}
static struct cacheItem * get_emptyCacheFrame()// Getting the first empty cache frame
{
    //struct cacheItem *item = 0;
    struct cacheItem *item;
    if(is_cacheFUll())//Checking for caches that are already full
    {
        return NULL;
    }

    item = &cache[cacheSize]; //Gettign the last item 
    
    cacheSize++;//Incrementing the cache size

    return item;
}
static struct cacheItem * get_oldestCacheFrame()//Gett oldest cache frame
{
    //struct cacheItem *item = 0;
    struct cacheItem *item = NULL;//current oldes item

    if(cacheSize == 0) // Checking for empty cache
    {
        return NULL;
    }
    int i;
    for(i = 0; i<cacheSize; i++)// Searching for the oldes cache(Oldest cahce based on time)
    {
        if(item == NULL || item->time > cache[i].time)//Already oldest at the start || check time
        {
            item = &cache[i];
        }
    }

    return item;

    

}
static struct cacheItem * get_cacheFrame(uint16_t index)// Funding frame using frame index, Null for miss and cache for Hits
{
    struct cacheItem *item; // Item we are looking for
    int i ;
    //while(i< 1024)

    for(i = 0; i <cacheSize ; i++)
    {
        if(cache[i].frameIndex == index)
        {
            item = &cache[i];
            //break;
            return item;
        }
    }
    
    return NULL;


}
