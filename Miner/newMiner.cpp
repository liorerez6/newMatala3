#include "SharedFile.cpp"

using namespace std;

/* for us to preaprae calculations for next block */
void setNewBlock(Block& i_newBlockForUpdate, int i_MinerId, int i_Difficulty)
{
    i_newBlockForUpdate.m_Block.height++;
    i_newBlockForUpdate.m_Block.prev_hash = i_newBlockForUpdate.hash;
    i_newBlockForUpdate.m_Block.relayed_by = i_MinerId;
    i_newBlockForUpdate.m_Block.difficulty = i_Difficulty;
}

void minerLoop();

int main()
{
    minerLoop();
    return 0;
}

// Miner thread function
void minerLoop() 
{
    const char* outputFilePath = "/var/log/mtacoin.log";
    redirectOutputToFile(outputFilePath);

    // Miner ID
    ssize_t bytesWrite;
    ssize_t bytesRead;
    Block currentBlock;
    TLV tlv;

    // config file handling ------------------------------------------------------------------
    int fdOfCommonFile = open(PATH_OF_COMMON_FILE, O_RDONLY);

    if (fdOfCommonFile == -1) {
        perror("open");
        exit(1);
    }

    // Read the file content
    char buffer[256];
    bytesRead = read(fdOfCommonFile, buffer, sizeof(buffer) - 1);
    if (bytesRead == -1) {
        perror("read");
        close(fdOfCommonFile);
        exit(1);
    }
    buffer[bytesRead] = '\0'; // Null-terminate the buffer

    close(fdOfCommonFile);

    // Parse the config file content
    int difficulty = 0;
    int serverId = 0;
    
    int minerCounter = 0;
    
    sscanf(buffer, "DIFFICULTY = %d\nPID = %d\nMINER_COUNTER = %d", &difficulty, &serverId, &minerCounter);

    // config file handling
    //------------------------------------------------------------------
    
    minerCounter++;
    int Read_fd;
    int Write_fd;

    //-------------------------------------------------------------------------------
    string piped_name_1 = PATH_PIPED_NAME_MINER_TO_SERVER;
    const char* write_piped_name = piped_name_1.c_str();
    //-------------------------------------------------------------------------------
    string piped_name_2 = PATH_PIPED_NAME_SERVER_TO_MINER;
    string piped_name_str_2 = piped_name_2 + to_string(minerCounter);
    const char* read_piped_name = piped_name_str_2.c_str();
    //-------------------------------------------------------------------------------
   
    if (mkfifo(read_piped_name, 0666) != 0) 
    {
        perror("mkfifo");
        exit(1);
    }

    Read_fd = open(read_piped_name, O_RDWR);

    if (Read_fd == -1) 
    {
        perror("open");
        exit(1);
    }

    Write_fd = open(write_piped_name, O_WRONLY);
    if (Write_fd == -1) 
    {
        perror("open");
        exit(1);
    }

    tlv.m_subscription = true;
    tlv.m_minerId = minerCounter;
    
    bytesWrite = write(Write_fd, &tlv, sizeof(TLV));

    if(bytesWrite == -1)
    {
        perror("write");
        exit(1);
    }

    bytesRead = read(Read_fd, &currentBlock, sizeof(Block)); // waiting untill gets

    if(bytesRead == -1)
    {
        perror("read");
        exit(1);
    }

    close(Read_fd);

    Read_fd = open(read_piped_name, O_RDONLY | O_NONBLOCK);

    if(Read_fd == -1)
    {
        perror("open");
        exit(1);
    }
    
    setNewBlock(currentBlock, minerCounter, difficulty); 

    Block temp;
    temp.m_Block.height = 0;

    while(true)
    {
        read(Read_fd, &temp, sizeof(Block));

        if(temp.m_Block.height != 0)
        {  
            currentBlock = temp;
            setNewBlock(currentBlock, minerCounter, difficulty);
            temp.m_Block.height = 0;
        }

        currentBlock.m_Block.updateTimestamp();
        currentBlock.m_Block.nonce++;
        currentBlock.hash = calculateCRC32(currentBlock.m_Block);
        
        //if mining is succeseful so :
        if(maskCheckForDifficulty(currentBlock.m_Block.difficulty, currentBlock.hash) == true)
        {
            cout << "Miner #" << dec << currentBlock.m_Block.relayed_by << ": Mined a new block #" << currentBlock.m_Block.height << ", with the hash ";
            cout << "0x" << hex << currentBlock.hash << endl;
            cout << dec;

            tlv.m_subscription = false;
            tlv.m_Block = currentBlock;

            ssize_t bytesWritten = write(Write_fd, &tlv, sizeof(TLV));
            if(bytesWritten == -1)
            {
                perror("write");
                exit(1);
            }
        }
    }
}

