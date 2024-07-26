#include <iostream>
#include <pthread.h>
#include <zlib.h>
#include <list>
#include <unistd.h> // For sleep function
#include <chrono>
#include <string.h>
#include <csignal>
#include <cstring>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

using namespace std;

class BlockForHash{

    public:

    int height;
    int timestamp;
    unsigned int prev_hash;
    int difficulty;
    int nonce;
    int relayed_by;

    // Constructor
    BlockForHash(int i_height, int i_timestamp, unsigned int i_prev_hash, int i_difficulty, int i_nonce, int i_relayed_by)
        : height(i_height), timestamp(i_timestamp), prev_hash(i_prev_hash),
          difficulty(i_difficulty), nonce(i_nonce), relayed_by(i_relayed_by) {}


    void updateTimestamp() {
        // Get the current time as an integer (seconds since epoch)
        timestamp = static_cast<int>(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));
    }

    BlockForHash() : height(0), timestamp(0), prev_hash(0), difficulty(0), nonce(0), relayed_by(0) {}

};



class Block
{
    public:

    BlockForHash m_Block;
    unsigned int hash;
    Block(BlockForHash i_Block, int i_hash) : m_Block(i_Block), hash(i_hash) {};
    Block() : m_Block(), hash(0) {}


};


uLong calculateCRC32(BlockForHash block) {
    uLong crc = crc32(0L, Z_NULL, 0);
    crc = crc32(crc, (const Bytef*)&block, sizeof(block));
    return crc;
}




bool maskCheckForDifficulty(int i_Difficulty, int i_hash) {   // true = proper difficulty. false = not good difficulty
    // Initialize mask with 1s in the most significant bit and zeros in the rest
     uLong mask = 0xFFFFFFFF;
    mask <<= (32 - i_Difficulty); // 100000000000
    return (!(mask & i_hash));
    
    
}

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
void minerLoop() {
    // Miner ID

    Block currentBlock;
    
    // config file handling ------------------------------------------------------------------
    int fdOfCommonFile = open("/home/liorerez6/Desktop/CommonFile.conf", O_RDONLY);

    if (fdOfCommonFile == -1) {
        perror("open");
        exit(1);
    }

    // Read the file content
    char buffer[256];
    ssize_t bytesRead = read(fdOfCommonFile, buffer, sizeof(buffer) - 1);
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

    // config file handling ------------------------------------------------------------------
    
    minerCounter++;
    int Read_fd;
    int Write_fd;


    
    //-------------------------------------------------------------------------------
    string PIPED_NAME_1 = "/home/liorerez6/Desktop/Piped_Miner_To_Server";
    const char* WRITE_PIPED_NAME = PIPED_NAME_1.c_str();
    //-------------------------------------------------------------------------------
    string PIPED_NAME_2 = "/home/liorerez6/Desktop/Piped_Server_To_Miner_";
    string PIPED_NAME_STR_2 = PIPED_NAME_2 + to_string(minerCounter);
    const char* READ_PIPED_NAME = PIPED_NAME_STR_2.c_str();
    //-------------------------------------------------------------------------------

    if(minerCounter == 1)
    {
        if (mkfifo(WRITE_PIPED_NAME, 0666) != 0) 
        {
            perror("mkfifo");
            exit(1);
        }
    }
   
    if (mkfifo(READ_PIPED_NAME, 0666) != 0) 
    {
        perror("mkfifo");
        exit(1);
    }

    Read_fd = open(READ_PIPED_NAME, O_RDWR);

    kill(serverId, SIGUSR1);

    if (Read_fd == -1) 
    {
        perror("open");
        exit(1);
    }

    //read(Read_fd, &currentBlock, sizeof(Block));

   // close(Read_fd);

    Write_fd = open(WRITE_PIPED_NAME, O_WRONLY);

    if (Write_fd == -1) 
    {
        perror("open");
        exit(1);
    }

    //Read_fd = open(READ_PIPED_NAME, O_RDONLY | O_NONBLOCK);

    read(Read_fd, &currentBlock, sizeof(Block)); // waiting untill gets

    close(Read_fd);

    Read_fd = open(READ_PIPED_NAME, O_RDONLY | O_NONBLOCK);

    cout << "first block read: " << currentBlock.hash << " and its height: " << currentBlock.m_Block.height << " difficulty: " << currentBlock.m_Block.difficulty << endl;

    setNewBlock(currentBlock, minerCounter, difficulty); 
    //int old_height = currentBlock.m_Block.height; // soppouse to be always one because the genesis send block with height = 0

    Block temp;
    temp.m_Block.height = 0;


    while (true) 
    {
        while(true)
        {
            
            read(Read_fd, &temp, sizeof(Block));
            //read(Read_fd, &currentBlock, sizeof(Block));

            if(temp.m_Block.height != 0)
            {
                cout << "i am inside the if of checking old height, about to print new block with height: " << currentBlock.m_Block.height << " difficulty: " << currentBlock.m_Block.difficulty << endl;
                currentBlock = temp;
                setNewBlock(currentBlock, minerCounter, difficulty);
                temp.m_Block.height = 0;
                cout << "i am after setting new block inside the if statment , about to print new block with height: " << currentBlock.m_Block.height << " difficulty: " << currentBlock.m_Block.difficulty << endl;
            }

            // if(currentBlock.m_Block.height > old_height)
            // {
            //     cout << "i am inside the if of checking old height, about to print new block with height: " << currentBlock.m_Block.height << " difficulty: " << currentBlock.m_Block.difficulty << endl;
            //     setNewBlock(currentBlock, minerCounter, difficulty);
            //     old_height = currentBlock.m_Block.height;
            //     cout << "i am after setting new block inside the if statment , about to print new block with height: " << currentBlock.m_Block.height << " difficulty: " << currentBlock.m_Block.difficulty << endl;
            // }

            currentBlock.m_Block.updateTimestamp();
            currentBlock.m_Block.nonce++;
            currentBlock.hash = calculateCRC32(currentBlock.m_Block);

            //if mining is succeseful so :
            if(maskCheckForDifficulty(currentBlock.m_Block.difficulty, currentBlock.hash) == true)
            {
                break;
            }
        }

        //write to log file
        //------------------------------
        cout << "Miner #" << dec << currentBlock.m_Block.relayed_by << ": Mined a new block #" << currentBlock.m_Block.height << ", with the hash ";
        cout << "0x" << hex << currentBlock.hash << endl;
        cout << dec;
        //------------------------------
        write(Write_fd, &currentBlock, sizeof(Block)); // chat gpt please check that this line corespond to the line in the server.cpp Read_fd
    }
}

