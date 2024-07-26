
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
#include <vector>

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

void serverCheckingBlocks();
volatile sig_atomic_t signal_received = 0;
volatile sig_atomic_t check_blocks_flag = 0;
list<Block> blockchain;

Block testingBlock; // testing the block if its good --> push to blockchain list. 

int numOfMiners = 0;
int fdOfCommonFile = 0;
int g_Difficulty = 0;
int ReadMinerFD;
vector<int> WriteMinerFD;

// Function to calculate CRC32
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


bool proofOfWork(const Block& i_Block) 
{
    Block curr = blockchain.back();
    if(curr.m_Block.height >= i_Block.m_Block.height)
    {
        cout << "Server: Block that accept from Miner " << dec << i_Block.m_Block.relayed_by << " already exist from Miner " << curr.m_Block.relayed_by << endl;

        return false;
        
    }

    int hashOfCRC32 = calculateCRC32(i_Block.m_Block);

    if(hashOfCRC32 != i_Block.hash)
    {
        cout << "Server: Wrong hash for block #" << dec << i_Block.m_Block.height << " by miner " << i_Block.m_Block.relayed_by << hex << ", recived 0x" << i_Block.hash << " but calculated 0x" << hashOfCRC32 << endl;
        cout << dec;
    }

    return (maskCheckForDifficulty(i_Block.m_Block.difficulty, hashOfCRC32));
}

// void signal_handler(int i_Sig)
// {
//     numOfMiners++;
//     string PIPED_NAME_1 = "/home/liorerez6/Desktop/Piped_Miner_To_Server";
//     const char* READ_PIPED_NAME = PIPED_NAME_1.c_str();

   
//     if(numOfMiners == 1)
//     {
//         ReadMinerFD = open(READ_PIPED_NAME, O_RDWR); //maybe add RDWR

//         if(ReadMinerFD < 0)
//         {
//             exit(1);
//         }
//     }


//     string PIPED_NAME_2 = "/home/liorerez6/Desktop/Piped_Server_To_Miner_";
//     string PIPED_NAME_STR_2 = PIPED_NAME_2 + to_string(numOfMiners);
//     const char* WRITE_PIPED_NAME = PIPED_NAME_STR_2.c_str();

//     int Write_fd = open(WRITE_PIPED_NAME, O_WRONLY);

//     if(Write_fd < 0) 
//     {
//         exit(1);
//     }

//     WriteMinerFD.push_back(Write_fd);
//     Block currentBlock = blockchain.back();
//     cout << "wrote genesis block: " << currentBlock.hash << " and its height: " << currentBlock.m_Block.height << " difficulty: " << currentBlock.m_Block.difficulty << endl;

//     write(Write_fd, &blockchain.back(), sizeof(Block)); // writes the last block in the chain
    
//     // write to the common file
//     //--------------------------------------------------
//     const char* commonFilePath = "/home/liorerez6/Desktop/CommonFile.conf";
//     FILE* file = fopen(commonFilePath, "r+");
//     if (file == nullptr) {
//         perror("fopen");
//         exit(1);
//     }

//     // Buffer to hold the content of the file
//     char buffer[256];
//     fread(buffer, sizeof(char), sizeof(buffer) - 1, file);
//     buffer[255] = '\0';  // Null-terminate the buffer

//     // Find the position of MINER_COUNTER
//     char* pos = strstr(buffer, "MINER_COUNTER = ");
//     if (pos != nullptr) {
//         // Move the pointer to the value part and update it
//         pos += strlen("MINER_COUNTER = ");
//         sprintf(pos, "%d\n", numOfMiners);
//     }

//     // Rewind and overwrite the file with updated content
//     rewind(file);
//     fwrite(buffer, sizeof(char), strlen(buffer), file);
//     fclose(file);

//     //--------------------------------------------------
//     serverCheckingBlocks();
// }

void InitServer()
{
    int pid;
    pid = getpid();
    fdOfCommonFile = open("/home/liorerez6/Desktop/CommonFile.conf", O_RDONLY);

    if (fdOfCommonFile == -1) {
        perror("open");
        exit(1);
    }
    
    int fdOfCommonFile = open("/home/liorerez6/Desktop/CommonFile.conf", O_RDWR | O_CREAT, 0666);

    if (fdOfCommonFile == -1) {
        perror("open");
        exit(1);
    }

    // Read the existing content into a buffer
    char buffer[256];
    ssize_t bytesRead = read(fdOfCommonFile, buffer, sizeof(buffer) - 1);
    if (bytesRead == -1) {
        perror("read");
        close(fdOfCommonFile);
        exit(1);
    }
    buffer[bytesRead] = '\0'; // Null-terminate the buffer

    // Parse the config file content
    sscanf(buffer, "DIFFICULTY = %d", &g_Difficulty);


    // Prepare new content
    string newContent;
    
    newContent += "PID = " + to_string(pid) + "\n";
    newContent += "MINER_COUNTER = " + to_string(numOfMiners) + "\n";
    

    // Write the new content to the file
    lseek(fdOfCommonFile, 0, SEEK_END); // Rewind to the beginning of the file
    write(fdOfCommonFile, newContent.c_str(), newContent.size());

    close(fdOfCommonFile);

}

void signal_handler(int i_Sig) {
    numOfMiners++;
    string PIPED_NAME_1 = "/home/liorerez6/Desktop/Piped_Miner_To_Server";
    const char* READ_PIPED_NAME = PIPED_NAME_1.c_str();

    if(numOfMiners == 1) {
        ReadMinerFD = open(READ_PIPED_NAME, O_RDWR); //maybe add RDWR
        if(ReadMinerFD < 0) {
            exit(1);
        }
    }

    string PIPED_NAME_2 = "/home/liorerez6/Desktop/Piped_Server_To_Miner_";
    string PIPED_NAME_STR_2 = PIPED_NAME_2 + to_string(numOfMiners);
    const char* WRITE_PIPED_NAME = PIPED_NAME_STR_2.c_str();

    int Write_fd = open(WRITE_PIPED_NAME, O_WRONLY);
    if(Write_fd < 0) {
        exit(1);
    }

    WriteMinerFD.push_back(Write_fd);
    Block currentBlock = blockchain.back();
    cout << "wrote genesis block: " << currentBlock.hash << " and its height: " << currentBlock.m_Block.height << " difficulty: " << currentBlock.m_Block.difficulty << endl;
    write(Write_fd, &blockchain.back(), sizeof(Block)); // writes the last block in the chain

    // write to the common file
    //--------------------------------------------------
    const char* commonFilePath = "/home/liorerez6/Desktop/CommonFile.conf";
    FILE* file = fopen(commonFilePath, "r+");
    if (file == nullptr) {
        perror("fopen");
        exit(1);
    }

    // Buffer to hold the content of the file
    char buffer[256];
    fread(buffer, sizeof(char), sizeof(buffer) - 1, file);
    buffer[255] = '\0';  // Null-terminate the buffer

    // Find the position of MINER_COUNTER
    char* pos = strstr(buffer, "MINER_COUNTER = ");
    if (pos != nullptr) {
        // Move the pointer to the value part and update it
        pos += strlen("MINER_COUNTER = ");
        sprintf(pos, "%d\n", numOfMiners);
    }

    // Rewind and overwrite the file with updated content
    rewind(file);
    fwrite(buffer, sizeof(char), strlen(buffer), file);
    fclose(file);

    //--------------------------------------------------
    check_blocks_flag = 1;
}


void serverLoop();

int main()
{
    serverLoop();
    return 0;
}

void broadcastBlockToAllMiners()
{
    Block temp = blockchain.back();
    //temp.m_Block.height++;     // maybe to change

    for (int i = 0; i < numOfMiners;i++)
    {
        write(WriteMinerFD[i],&temp ,sizeof(Block));
    }

}

void serverLoop() 
{

    InitServer();

    BlockForHash genesisBlock1(0, time(NULL), 0, g_Difficulty, 1, 0);
    genesisBlock1.updateTimestamp();
    Block genesisBlock(genesisBlock1, 0);
    blockchain.push_back(genesisBlock);

    signal(SIGUSR1, signal_handler);

    while(true) 
    {
        if (check_blocks_flag) {
            serverCheckingBlocks();
            check_blocks_flag = 0;
        }
    }
}

void serverCheckingBlocks()
{
    while (true) 
    {
        read(ReadMinerFD, &testingBlock, sizeof(Block)); 

        if(proofOfWork(testingBlock))
        {
            blockchain.push_back(testingBlock);
            cout << dec;
            cout << "Server: New block added by " << testingBlock.m_Block.relayed_by << ", attributes: height(" << testingBlock.m_Block.height << ")" << 
            ", timestamp(" << testingBlock.m_Block.timestamp << ")" << ", hash("; 
            cout << "0x" << hex << testingBlock.hash << ")" << ", prev_hash(0x" << testingBlock.m_Block.prev_hash << ")";
            cout << dec;
            cout << ", difficulty(" << testingBlock.m_Block.difficulty << ")" << ", nonce(" << testingBlock.m_Block.nonce << ")" << endl;
            
            broadcastBlockToAllMiners();
        }
    }
}