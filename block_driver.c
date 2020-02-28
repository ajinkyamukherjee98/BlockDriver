////////////////////////////////////////////////////////////////////////////////
//
//  File           : block_driver.c
//  Description    : This is the implementation of the standardized IO functions
//                   for used to access the BLOCK storage system.
//
//  Author         : [Ajinkya]
//  Last Modified  : [26th July]
//

// Includes
#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include <string.h>
// Project Includes
#include <block_controller.h>
#include <block_driver.h>
#include <cmpsc311_util.h>
#include <block_cache.h>

#define USED_FRAMES_ARRAY_SIZE BLOCK_BLOCK_SIZE / 8

struct data // File Descriptor Table
{
    uint32_t size;
    uint32_t position;
    uint16_t *frames;
    uint16_t framesAmount;
    uint8_t isOpen;
    char path[BLOCK_MAX_PATH_LENGTH + 1];
};

struct data files[BLOCK_MAX_TOTAL_FILES];// eMPTY fiLES DATA sTRUCTURE

uint8_t usedFrames[USED_FRAMES_ARRAY_SIZE];// opEN fILES DATA STRUCTURES AS A BIT ARRAY.




// Helper Functions for assignment 2
static void initialize_FilesTable();
static void initialize_UsedFramesTable();
static int is_usedFrames();
static int16_t get_FilebyPath(const char *path);
static struct data * get_Filebyfd(int16_t fd);
static int16_t create_newFile(const char *path);
static int add_FrameFile(int16_t fd);
static int is_FrameEmpty(int frame);
static int get_FreeFrame();
static void mark_UsedFrame(uint16_t frame);
static BlockXferRegister create_Block_opcode(uint8_t key, uint16_t frame, uint32_t checksum);
static uint8_t responseFromRegisters(BlockXferRegister reg);
static uint32_t cheksumFromRegisters(BlockXferRegister reg);
static uint8_t init_block();
static uint8_t zero_block();
static uint8_t read_BlockFrame(uint16_t frame, void *buf);
static uint8_t write_BlockFrame(uint16_t frame, void *buf);
static uint8_t deinit_block();
static int compute_frame_checksum(void *frame, uint32_t *checksum);
static int min(int a, int b);

// Helper functions for assign 3
// Saving Data
static void * serialize_Data(struct data *file , int *size);
static int save_UsedFrames();
static int save_FilesArray();
static int save_DataStructure();
//Loading data
//static void * deserialize_Data();
static struct data * deserialize_Data(void *newData , int *size);
static int load_UsedFrames();
static int load_FilesArray();
static int load_DataStructure();
static int data_size();





//
// Implementation

////////////////////////////////////////////////////////////////////////////////
//
// Function     : block_poweron
// Description  : Startup up the BLOCK interface, initialize filesystem
//
// Inputs       : none
// Outputs      : 0 if successful, -1 if failure

int32_t block_poweron(void)
{
    //printf("TestFunctionPowerOn\n");
    if(init_block() != BLOCK_RET_SUCCESS)
    {
        return -1;
    }

     if(init_block_cache() == -1)// initializing cache
    {
        return -1;
    }

    //printf("testLoad\n");
    load_DataStructure();// Loading data structres from the first frame
    //return -1; 
    //printf("Loading Successful\n");
  if(!is_usedFrames())// If used frames not set after loading clean everything
   {
        if(zero_block() != BLOCK_RET_SUCCESS)
        {
            //printf("Printing error\n");
            return -1;
        }
    //}
    initialize_FilesTable();
    initialize_UsedFramesTable();
   }
     ////printf("TestFunctionPowerOnEnd\n");


    
    /*int32_t fd;
	BlockXferRegister InitOP =(BlockXferRegister) BLOCK_OP_INITMS << 56;
	BlockXferRegister value = block_io_bus(InitOP, NULL);// Interacting with the controller 0 if succesful and -1 if unsuccesful
	if((value&0xff) == 0)
	{
		BlockXferRegister BZero =(BlockXferRegister)BLOCK_OP_BZERO << 56;
		BlockXferRegister val = block_io_bus(BZero , NULL);
		if((val&0x11)==0)
		{
			newData = (data*)calloc(BLOCK_MAX_TOTAL_FILES,sizeof(data));
			newData->position = 0;
		//	newData->objectIndex = 0;
			newData->length = 0;
			fd = 0;
		}
	}
		else
		{
			fd = -1;

		}

		if(val == 0)
		{
			fcall = (BlockXferRegister*)calloc(Block_MAX,sizeof(BlockXferRegister));
			BlockRegisters.BLOCK_REG_KY1 = 0;
			BlockRegisters.BLOCK_REG_FM1 = 0;
			BlockRegisters.BLOCK_REG_CS1 = 0;
			BlockRegisters.BLOCK_REG_RT1 = 0;
			BlockRegisters.BLOCK_REG_MAXVAL = 0;
		}*/
	 

    // Return successfully
    //printf("ENd Test Poweron\n");
    return (0);
}
////////////////////////////////////////////////////////////////////////////////
//
// Function     : block_poweroff
// Description  : Shut down the BLOCK interface, close all files
//
// Inputs       : none
// Outputs      : 0 if successful, -1 if failure

int32_t block_poweroff(void)
{
     //printf("Blockoff Start\n");


    if(!save_DataStructure())// Saving data strutures to the block
    {
        return -1;
    }

    if(deinit_block() != BLOCK_RET_SUCCESS)// Swithc off the block
    {
        return -1;
    }
    int i;
    for(i = 0; i<BLOCK_MAX_TOTAL_FILES ;i++)// Deinitializinf each frame in the file
    {
        free(files[i].frames);
    }

    close_block_cache();
     //printf("Blockoff end");


    /*BlockXferRegister  OffOP = BLOCK_OP_POWOFF<<56;
	BlockXferRegister value = block_io_bus(OffOP,NULL );

	if(value == 0)
	{
		close(new_data);
		free(new_data);

	}
	else
	{
		return -1;
	}
 */

    // Return successfully
   // printf("Blockoff Start\n");
    return (0);
    
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : block_open
// Description  : This function opens the file and returns a file handle
//
// Inputs       : path - filename of the file to open
// Outputs      : file handle if successful, -1 if failure

int16_t block_open(char* path)
{
    int16_t fd;//File handler
    struct data *file;
     
    fd = get_FilebyPath(path);//Get File Descriptor/Handler from using Path
    //printf("Blockopen Start");
    if(fd<0)//If file DNE
    {
        fd = create_newFile(path);// Create a new file
    }

    if(fd<0)// If file not created or not opening
    {
        return -1;
    }

    file = get_Filebyfd(fd);// Get a file using file Descripor/handler

    if(file == NULL)// File DNE
    {
        return -1;
    }

    file->isOpen = 1;// Marking file to be open
    file->position = 0; // Setting position to 0;
     //printf("Blockopen end");
    //printf("Path = %s\n",path);
    /*
	char* checkfile;
	int count = 0;
	char * file = fopen(path,'r');
	if(checkfile != file)// if file does not exist
	{
		char * newPath;
		fopen(newPath,'w+');//Creating a new file and setting its position ot the first byte.
		//fseek(newPath, 0 ,SEEK_SET);
		//newPath.size() = 0;

		int s = fseek(newPath,0,SEEK_END);
		sizeof(newPath) = 0;

	}
	else
	{
		fopen(path,'w+');
	}

	
	char *checkFile;
	if(checkFile != fopen(path,'r'))
	{
		
	}
    */
   
    // THIS SHOULD RETURN A FILE HANDLE
    //printf("end BLOck open\n");
    return (fd);
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : block_close
// Description  : This function closes the file
//
// Inputs       : fd - the file descriptor
// Outputs      : 0 if successful, -1 if failure

int16_t block_close(int16_t fd)
{
    struct data * file;
   
    file = get_Filebyfd(fd);// Getting file usin Fiel Descriptor
    //printf("Blockclose Start");
    if(file == NULL)// File DNE
    {
        return -1;
    }

    if(!files->isOpen) // If file is not open
    {
        return -1;
    }

    files->isOpen = 0;// close the file
    //files->position= 0;
    // Return successfully
     //printf("Blockclose end");
     
    return (0);
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : block_read
// Description  : Reads "count" bytes from the file handle "fh" into the
//                buffer "buf"
//
// Inputs       : fd - filename of the file to read from
//                buf - pointer to buffer to read into
//                count - number of bytes to read
// Outputs      : bytes read if successful, -1 if failure

int32_t block_read(int16_t fd, void* buf, int32_t count)
{
    struct data * file;
    int currentFrame;
    int bytesRead = 0;
    int start;
    int amount;
    int newCurrentFrame;
    int temp;
    int minimum;
    char frame[BLOCK_FRAME_SIZE];
    //printf("BlockRead Start\n");
    file = get_Filebyfd(fd);
    //printf("start read FIle open is %d\n",file->isOpen);

    if(file == NULL)// File DNE
    {
        return -1;
    }

    if(!file->isOpen) // If file is not open
    {
        return -1;
    }

    while (bytesRead < count)// Keep Reading till you reach count
    {
        
        currentFrame = (file->position) / BLOCK_FRAME_SIZE; // CUrrent Position of the Frame

        if(currentFrame >= file->framesAmount)// Need to read only with the amount given
        {
            //printf("%d\n",file->framesAmount);
            //printf("Break\n");
            break;
        }

        if(read_BlockFrame(file->frames[currentFrame],frame)!= BLOCK_RET_SUCCESS)
        {
            return -1;
        }

        start = (file->position) % BLOCK_FRAME_SIZE;
        newCurrentFrame = (currentFrame + 1)* BLOCK_FRAME_SIZE;
        temp = (count - bytesRead) + file->position;
        minimum = min(file->size , newCurrentFrame) ;
        amount = min(minimum , temp) - file->position;
        //printf("%d\n",start);
        //printf("%d\n",newCurrentFrame);
        //printf("%d\n",temp);
        //printf("%d\n",minimum);
        //printf("%d\n",amount);

        memcpy(buf + bytesRead , frame + start, amount);// Copying Frame to buf
        bytesRead = bytesRead + amount;// updating bytesrad
        file->position = file->position + amount;// Updating the position 
        
    }
    
    //printf("BlockRead end\n");
    // Return successfully
    //printf("end read FIle open is %d\n",file->isOpen);
    return (bytesRead);
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : block_write
// Description  : Writes "count" bytes to the file handle "fh" from the
//                buffer  "buf"
//
// Inputs       : fd - filename of the file to write to
//                buf - pointer to buffer to write from
//                count - number of bytes to write
// Outputs      : bytes written if successful, -1 if failure

int32_t block_write(int16_t fd, void* buf, int32_t count)
{
    struct data * file;
    int currentFrame;
    int bytesWritten = 0;
    int start;
    int amount;
    int newCurrentFrame;
    int temp;
    //int minimum;
    char frame[BLOCK_FRAME_SIZE] = {0};
    //printf("TestStart Block Write\n");
    file = get_Filebyfd(fd);// Get file usnig FD

     //printf("Start write FIle open is %d\n",file->isOpen);
    if(file == NULL)// File DNE
    {
       // printf("return 1\n");
        return -1;
    }
   // printf("file open is ' %d '\n",file->isOpen);
    if(!file->isOpen) // If file is not open
    {
       // printf("return 2\n");
        return -1;
    }
   // printf("Is it entering While?\n");
    while (bytesWritten < count)// Keep Writing till you reach count
    {
       // printf("Inside While?\n");
        
        currentFrame = (file->position) / BLOCK_FRAME_SIZE; // CUrrent Position of the Frame
       // printf("%d\n",currentFrame);
        if(currentFrame >= file->framesAmount)// Need to read only with the amount given
        {
         //   printf("break?\n");
            //break;
            if(add_FrameFile(fd)!= 0)// Adding a new Frame if current frame is larger than the size.
            {
              //  printf("return 3\n");
                return -1;
            }
            // printf("Check\n");
        }
        else
        {
            //printf("else\n");
            //printf("%d\n",file->framesAmount);
            //printf("%d\n",currentFrame);
            ////printf("%c\n",frame[100]);
            //printf("%d\n",file->frames[currentFrame]);
            //read_BlockFrame(file->frames[currentFrame],frame);
            //printf("Testing IF\n");
            if(read_BlockFrame(file->frames[currentFrame],frame)!= BLOCK_RET_SUCCESS)
            {
               // printf("return 4\n");
                return -1;
            }
            //printf("ignored?\n");
        }
        //Setting new Bytes to the frame
        start = (file->position) % BLOCK_FRAME_SIZE;
        newCurrentFrame = (currentFrame + 1)* BLOCK_FRAME_SIZE;
        temp = (count - bytesWritten) + file->position;
        //minimum = min(file->size , newCurrentFrame) ;
        amount = min(temp , newCurrentFrame) - file->position;
        

        //memcpy(buf + bytesRead , frame + start, amount);// Copying Frame to buf
        memcpy(frame + start , buf + bytesWritten, amount);// Copying buf to frame

        if((write_BlockFrame(file->frames[currentFrame],frame)) != BLOCK_RET_SUCCESS)// If writing  not successful
        {
            //printf("return 5\n");
            return -1;
        }

        bytesWritten = bytesWritten + amount;// updating bytesrad
        file->position = file->position + amount;// Updating the position 

        if(file->size < file->position)
        {
            file->size = file->size + amount;
        }
    }
    

   // printf("TestEnd BLOCK write\n");
    // Return successfully
     //printf("end write FIle open is %d\n",file->isOpen);
    return (bytesWritten);
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : block_read
// Description  : Seek to specific point in the file
//
// Inputs       : fd - filename of the file to write to
//                loc - offfset of file in relation to beginning of file
// Outputs      : 0 if successful, -1 if failure

int32_t block_seek(int16_t fd, uint32_t loc)
{
    struct data * file;


    file = get_Filebyfd(fd);// Get File Using File Descriptor
    //printf("Start seek FIle open is %d\n",file->isOpen);
   // printf("Start File Seek\n");
    
    if(file == NULL)// File DNE
    {
        return -1;
    }

    if(!file->isOpen) // If file is not open
    {
        return -1;
    }

    if(file->size < loc)// If file length less than LOC return error
    {
        return -1;
    }

    file->position = loc;
    
    // Return successfully
    //printf("End File Seek\n");
    //printf("end seek FIle open is %d\n",file->isOpen);
    return (0);
}



// Helper Functions for assignment 2
static void initialize_FilesTable()// Initializing Files table
{
    int i;
    for(i = 0; i< BLOCK_MAX_TOTAL_FILES; i++)
    {
        files[i].size = 0;
        files[i].frames = NULL;
        files[i].isOpen = 0;
        files[i].framesAmount = 0;
        files[i].position = 0;
        //files[i].path = '';
        memset(files[i].path,0,BLOCK_MAX_PATH_LENGTH + 1);
    }

}
static void initialize_UsedFramesTable()// Initializing usedFrames Table
{
    memset(usedFrames , 0 , USED_FRAMES_ARRAY_SIZE);
    int i;
    for(i = 0; i<69;i++)
    {
        mark_UsedFrame(i);
    }
}

static int is_usedFrames()// Checking if frame is used or not
{
    int i;
    for(i = 0; i< 69; i++)
    {
        if(is_FrameEmpty(i))// If frame is empty it is unused
        {
            return 0;
        }
    }
    return 1;
}
static int16_t get_FilebyPath(const char *path)// Get file by path
{
    int i;
    for(i = 0; i<BLOCK_MAX_TOTAL_FILES; i++)
    {
        if(strncmp(path,files[i].path,BLOCK_MAX_PATH_LENGTH) == 0)
        {
            return i;
        }

    }
    return -1;
}
//static int get_filebyfd(int16_t)

static struct data * get_Filebyfd(int16_t fd)// Get file using File descriptor or handler
{
    if(fd < 0 || fd > BLOCK_MAX_TOTAL_FILES)
    {
        //return -1;
        return NULL;
    }
    return files + fd;
}
static int16_t create_newFile(const char *path)// Creating a new File
{
    int i = 0;
    while(i<BLOCK_MAX_TOTAL_FILES)
    {
        if(strlen(files[i].path) == 0)
        {
            strncpy(files[i].path , path , BLOCK_MAX_PATH_LENGTH);
            return i;
        }
        i++;
    }
    return -1;
}
static int add_FrameFile(int16_t fd)// Adding an extra frame to file
{
    struct data *file;
    uint16_t *temp;

    file = get_Filebyfd(fd);// Get file using FD
    if(file == NULL)// if file Does not Exist (DNE)
    {
        return -1;
    }
    //Incrementing the list
    if(file->framesAmount == 0)
    {
        file->frames = (uint16_t*) malloc(sizeof(uint16_t));
        if(file->frames == NULL)
        {
            return -1;
        }
    }
    else
    {
        temp = (uint16_t*) realloc(file->frames,sizeof(uint16_t)*(file->framesAmount + 1));
        if(temp == NULL)
        {
            return -1;
        }
        file->frames = temp;
    }
    file->frames[file->framesAmount] = get_FreeFrame();// Add a new empty frame to the list

    if( file->frames[file->framesAmount] < 0)
    {
        return -1;
    }
    mark_UsedFrame( file->frames[file->framesAmount]); // Setting ti as a used frame now
    file->framesAmount = file->framesAmount + 1;
    return 0;
}
static int is_FrameEmpty(int frame)// Checking if frame is empty or not
{
    int index;
    int offset;
    unsigned int mask;
    index = frame/8;
    offset = frame%8;
    mask = 1 << offset;

    return !(usedFrames[index] & mask);

}
static int get_FreeFrame()// Get an empty frame
{
    int i;
    for(i=0; i < BLOCK_BLOCK_SIZE; i++)
    {
        if(is_FrameEmpty(i))
        {
            return i;
        }
    }
    return -1;

}
//static void mark_FrameEmpty // Mark frame as empty
//{
//}
static void mark_UsedFrame(uint16_t frame)// Mark Frame as a used frame
{
    int index;
    int offset;
    unsigned int mask;
    index = frame/8;
    offset = frame%8;
    mask = 1 << offset;

    usedFrames[index] = usedFrames[index] | mask;
}

static BlockXferRegister create_Block_opcode(uint8_t key, uint16_t frame, uint32_t checksum)// Build Register
{
    uint64_t keyReg = (uint64_t) key << 56;
    uint64_t frameReg = (uint64_t) frame << 40;
    uint64_t checksumReg = (uint64_t) checksum << 8;
    //uint64_t returnValue = keyReg & frameReg & checksumReg;
    uint64_t returnValue = keyReg | frameReg | checksumReg;
    return returnValue;
    

}
static uint8_t responseFromRegisters(BlockXferRegister reg)// Getting register Response
{
    uint8_t temp = (uint8_t) (reg & 0xff);
    return temp;
}
static uint32_t cheksumFromRegisters(BlockXferRegister reg)// Getting checksu Response
{
     uint32_t temp = (uint32_t) ((reg >> 8) & 0xffffffff);
    return temp;

}
// Block Function
static uint8_t init_block()// Initialising the block, return BLOCK_RET_SUCCESS on Success and BLOCK_RET_ERROR other wise
{
    BlockXferRegister reg = create_Block_opcode(BLOCK_OP_INITMS , 0 ,0);
    reg = block_io_bus(reg,NULL);
    uint8_t temp = responseFromRegisters(reg);
    return temp;


}
static uint8_t zero_block()//Zeroing the block
{ 
    BlockXferRegister reg = create_Block_opcode(BLOCK_OP_BZERO , 0 ,0);
    reg = block_io_bus(reg,NULL);
    uint8_t temp = responseFromRegisters(reg);
    return temp;

}
static uint8_t read_BlockFrame(uint16_t frame, void *buf)//Reading a frame from the block
{   //printf("WE\n");
    BlockXferRegister sentReg;
    BlockXferRegister returnReg;
    uint32_t frameChecksum;
    uint32_t regChecksum;

    uint8_t *cacheFrame;
    //printf("BlockRead sub\n");
    cacheFrame = get_block_cache(0,frame);//first check if the frame is in the cache
    if(cacheFrame != NULL)
    {
        memcpy(buf, cacheFrame , BLOCK_FRAME_SIZE);
        return BLOCK_RET_SUCCESS;
    }
    sentReg = (create_Block_opcode(BLOCK_OP_RDFRME,frame,0)); // BUILDING A REGISTER
    //compute_frame_checksum();
    do // Write frame util there is a check sum error
    {
        //printf("BlockRead do sub\n");
        returnReg = block_io_bus(sentReg,buf);

        regChecksum = cheksumFromRegisters(returnReg); // get checksum
        compute_frame_checksum(buf, &frameChecksum);
    } while (regChecksum != frameChecksum);

    
    return responseFromRegisters(returnReg);

}
static uint8_t write_BlockFrame(uint16_t frame, void *buf) //Writing a frame from the block
{
    //printf("Test");
    BlockXferRegister sentReg;
    BlockXferRegister returnReg;
    //uint32_t frameChecksum;
    uint8_t returnCode;
    uint32_t regChecksum;

    if(compute_frame_checksum(buf,&regChecksum))
    {
        return BLOCK_RET_ERROR;
    }
    //printf("if #1\n");

    sentReg = (create_Block_opcode(BLOCK_OP_WRFRME,frame,regChecksum)); //Generating checksum from the block
    //compute_frame_checksum();
    
    do // Write frame until there is a check sum error
    {
        returnReg = block_io_bus(sentReg,buf);
        //printf("Do WHILE\n");
        returnCode = responseFromRegisters(returnReg); // get checksum
        //printf("%d",returnCode);
        //printf("\n");
        //compute_frame_checksum(buf, &frameChecksum);
    } while (returnCode == BLOCK_RET_CHECKSUM_ERROR);


     if(put_block_cache(0 , frame ,buf) != 0)//Updating cache
    {
        return BLOCK_RET_ERROR;
    }
    
   // uint8_t temp = responseFromRegisters(returnReg);
   // printf(returnCode);
    return returnCode;


}
static uint8_t deinit_block()// Deinitializing block
{
    BlockXferRegister reg = create_Block_opcode(BLOCK_OP_POWOFF, 0 ,0);
    reg = block_io_bus(reg,NULL);
    uint8_t temp = responseFromRegisters(reg);
    return temp;


}
static int compute_frame_checksum(void *frame, uint32_t *checksum)
{
    uint32_t sigsz = sizeof(uint32_t);
    *checksum = 0;
    if(generate_md5_signature(frame, BLOCK_FRAME_SIZE, (char *) checksum, &sigsz)) 
    {
        return -1;
    }
    return 0;

}
static int min(int a, int b)// calculating the minimum between two values
{
    if(a<b)
    {
        return a;
    }
    return b;
}


// Helper functions for assign 3


/* The data structure will be saved in the first 69 frames
the first two frames will be occupied by USed Frames and 67 by FilesArray


 */


// Saving Data
// Serializing the data
//size -> 4bytes
// position -> 4 bytes
// framesAmount -> 2 bytes
// list of frames -> 2bytes * framesAmount
// isOPen -> 1 byte
// path -> BLOCK_MAX_PATH_LENGTH + 1byte

static void * serialize_Data(struct data * file , int *size)
{
    //printf("Serialized Data\n");
    uint8_t *newData;
    int pos = 0;

    
    int dataSize = 11 + BLOCK_MAX_PATH_LENGTH + 2 * file->framesAmount;// calculating the data strcture size
    
    
    newData = malloc(dataSize);//Allocating data
    if(newData == NULL) 
    {
        return NULL;
    }

    //Copying data
    memcpy(newData + pos, &file->size, sizeof(file->size));//size
    pos = pos + sizeof(file->size);
    //printf("Position 1 %d\n",pos);
    memcpy(newData + pos, &file->position, sizeof(file->position));//position
    pos = pos + sizeof(file->position);
    //printf("Position 2 %d\n",pos);
    memcpy(newData + pos, &file->framesAmount, sizeof(file->framesAmount));//framesAmount
    pos = pos + sizeof(file->framesAmount);
    //printf("Position 3 %d\n",pos);
    //printf("%d\n",file->framesAmount);
    memcpy(newData + pos, file->frames, sizeof(*file->frames) * file->framesAmount);//frames
    pos = pos + sizeof(*file->frames) * file->framesAmount;
    //printf("Position 4 %d\n",pos);
    // memcpy(data + pos, &file->isOpen, sizeof(file->isOpen));
    // pos += sizeof(file->isOpen);
    memcpy(newData + pos, file->path, BLOCK_MAX_PATH_LENGTH + 1);//path
   pos = pos + BLOCK_MAX_PATH_LENGTH + 1;
   //printf("Position 5 %d\n",pos);

    *size = pos;

    return newData;

}
static int save_UsedFrames()// Saving the used frames into the data
{
    //0 is the first frame and 1 is the second
    uint8_t frame[BLOCK_FRAME_SIZE];
    int answer;
    

    memcpy(frame , usedFrames , BLOCK_FRAME_SIZE);// Saving the first 4096 bytes in Frame 1
    answer = write_BlockFrame(0,frame);
    //if(answer != NULL)
    if(answer != BLOCK_RET_SUCCESS)
    {
        return answer;
    }


    memcpy(frame , usedFrames + BLOCK_FRAME_SIZE, BLOCK_FRAME_SIZE);// Saving the second 4096 bytes in Frame 2
    

    return write_BlockFrame(1,frame);
    
}
static int save_FilesArray()
{
    int currentFrame = 2;// Start from the 3rd frame
    int framePosition = 0;
    int byteWritten = 0;
    int newDataSize ;
    int answer;
    uint8_t frame[BLOCK_FRAME_SIZE];
    uint8_t *newData;
    int i = 0;
    int bytesToCopy;
    
    
    for(i = 0; i< BLOCK_MAX_TOTAL_FILES ; i++)// iTerating over files array
    {
        newData = (uint8_t *)serialize_Data(files + i,&newDataSize);// Serilaing the data

        if(newData == NULL)
        {
            return BLOCK_RET_ERROR;
        }
        byteWritten = 0;
        //printf("Error test");
        while (byteWritten < newDataSize)// COintuing till bytes written is less than File entry size
        {
            //saving data to the frame
            bytesToCopy = (newDataSize - byteWritten);
           
            
            if(bytesToCopy > (BLOCK_FRAME_SIZE - framePosition))
            {
                bytesToCopy = BLOCK_FRAME_SIZE - framePosition;
                //printf(bytesTocopy);
            }
            memcpy(frame + framePosition, newData +byteWritten , bytesToCopy);
            byteWritten = byteWritten + bytesToCopy;
            framePosition = framePosition + bytesToCopy;

            if(framePosition == BLOCK_FRAME_SIZE)// Writing frame when its full
            {
                answer = write_BlockFrame(currentFrame,frame);
                if(answer != BLOCK_RET_SUCCESS)
                {
                    return answer;
                }
                currentFrame++;
                framePosition = 0;

            }
            
        }
        free(newData);
        


    }

    //return newData;
    return BLOCK_RET_SUCCESS;


}
static int save_DataStructure()// Saving the current state of data structure, rerun 1 on success and 0 on failure
{
    int answer;

    answer = save_UsedFrames();// saving used frames

    if(answer != BLOCK_RET_SUCCESS)
    {
        return 0;
    }

    answer = save_FilesArray();// saving files array using frames 3-69


    if(answer != BLOCK_RET_SUCCESS)
    {
        return 0;
    }

    return 1;
}


//Loading data
//static void * deserialize_Data();
static struct data * deserialize_Data(void* newData , int *size)// Deserialinzing the data structure
{
    //printf("Deserialized Data\n");
     // uint8_t fileData = (uint8_t *) data;
    struct data *file;
    int pos = 0;

    if(*size < data_size()) {
        return NULL;
    }

 
    file = (struct data *) malloc(sizeof(struct data));
    if(file == NULL) {
        return NULL;
    }

    // copying data
    memcpy(&file->size, newData + pos, sizeof(file->size));//size
    pos = pos + sizeof(file->size);
    //printf("Position 1 %d\n",pos);
    memcpy(&file->position, newData + pos, sizeof(file->position));//position
   pos = pos + sizeof(file->position);
    //printf("Position 2 %d\n",pos);
    memcpy(&file->framesAmount, newData + pos, sizeof(file->framesAmount));//framesAmount
    pos = pos + sizeof(file->framesAmount);
    //printf("Position 3 %d\n",pos);
    if(*size < data_size() + file->framesAmount * sizeof(*file->frames)) 
    {
        free(file);
        return NULL;
    }
    if(file->framesAmount > 0) 
    {
        file->frames = (uint16_t *) malloc(sizeof(uint16_t) * file->framesAmount);
        if(file->frames == NULL)
         {
            free(file);
            return NULL;
        }
    } 
    else 
    {
        file->frames = NULL;
    }

    memcpy(file->frames, newData + pos, sizeof(*file->frames) * file->framesAmount);//Frames
   pos = pos + sizeof(*file->frames) * file->framesAmount;
    //printf("Position 4 %d\n",pos);
    // memcpy(&file->isOpen, data + pos, sizeof(file->isOpen));
    // pos += sizeof(file->isOpen);
    file->isOpen = 0;
    memcpy(file->path, newData + pos, BLOCK_MAX_PATH_LENGTH + 1);//path
    pos = pos + BLOCK_MAX_PATH_LENGTH + 1;
    //printf("Position 5 %d\n",pos);
    *size = pos;
    

    return file;


}
static int load_UsedFrames()// loading the used frames into the data
{
    uint8_t frame[BLOCK_FRAME_SIZE];
    int answer;

    //memcpy(frame , usedFrames , BLOCK_FRAME_SIZE);// Saving the first 4096 bytes in Frame 1
    answer = read_BlockFrame(0,frame);
    if(answer != BLOCK_RET_SUCCESS)
    {
        return answer;
    }
    memcpy(usedFrames , frame , BLOCK_FRAME_SIZE);// load the first 4096 bytes in Frame 1
    
    
    answer = read_BlockFrame(1,frame);
    if(answer != BLOCK_RET_SUCCESS)
    {
        return answer;
    }
    memcpy(usedFrames + BLOCK_FRAME_SIZE , frame , BLOCK_FRAME_SIZE);// load the second 4096 bytes in Frame 2
    
    return answer;

}
static int load_FilesArray()// Loading files array from the block
{
    //uint8_t frames[BLOCK_FRAME_SIZE];
    int remainingSize = BLOCK_FRAME_SIZE *67;
    uint8_t frames[BLOCK_FRAME_SIZE *67];
    int size;
    int framePosition = 0;
    int answer;
    struct data *file;
    int i;

    for(i = 0; i< 67; i++)// Loading all the frames
    {
        answer = read_BlockFrame(i+2 , frames + i * BLOCK_FRAME_SIZE);
        if(answer != BLOCK_RET_SUCCESS)
        {
            return BLOCK_RET_ERROR;
        }

    }

    for(i = 0;  i< BLOCK_MAX_TOTAL_FILES; i++)// Deserializing all the files from the read frame
    {
        size = remainingSize;
        file = deserialize_Data(frames + framePosition , &size);

        if(file == NULL)
        {
            return BLOCK_RET_ERROR;
        }
        memcpy(files+ i , file ,sizeof(struct data));

        //remainingSize = remainingSize + size;
        remainingSize = remainingSize - size;
        framePosition = framePosition + size;

    }
    
    return BLOCK_RET_SUCCESS;

}
static int load_DataStructure()// Loading the current state f datat structures into the block
{
    int answer;
    
    
    answer = load_UsedFrames();// loading used frames

    if(answer != BLOCK_RET_SUCCESS)
    {
        return 0;
    }

    answer = load_FilesArray();// loading files array using frames 3-69


    if(answer != BLOCK_RET_SUCCESS)
    {
        return 0;
    }

    return 1;

}

static int data_size()
{
    int total_Size = 0;
    total_Size = total_Size + sizeof(uint32_t);//size
    total_Size = total_Size + sizeof(uint32_t);//position
    total_Size = total_Size + sizeof(uint16_t);//framesAmount
    //total_Size = total_Size + sizeof(uint8_t); we dont need one for open since the files are expected to be open
    total_Size = total_Size + sizeof(BLOCK_MAX_PATH_LENGTH + 1);//Path

    return total_Size;
}

