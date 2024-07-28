#include "SharedFile.h"

using namespace std;


/* method's declaration */
void serverCheckingBlocks();
void serverLoop();

/* Global variables */
list<Block> blockchain; 
int numOfMiners = 0;
int g_Difficulty = 0;
int ReadMinerFD;
vector<int> WriteMinerFD;

/* methods */
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

void InitServer()
{
    int pid;
    pid = getpid();
    ssize_t bytesRead;

    int fdOfCommonFile = open(PATH_OF_COMMON_FILE, O_RDWR | O_CREAT, 0666);

    if (fdOfCommonFile == -1) {
        perror("open");
        exit(1);
    }

    // Read the existing content into a buffer
    char buffer[256];
    bytesRead = read(fdOfCommonFile, buffer, sizeof(buffer) - 1);
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

    bytesRead = write(fdOfCommonFile, newContent.c_str(), newContent.size());
    if (bytesRead == -1) {
        perror("write");
        exit(1);
    }

    close(fdOfCommonFile);

    const char* READ_PIPED_NAME = PATH_PIPED_NAME_MINER_TO_SERVER;

    
    if (mkfifo(READ_PIPED_NAME, 0666) != 0) 
    {
        cout << "NNNNNNNN";
        perror("mkfifo");
        exit(1);
    }

    ReadMinerFD = open(READ_PIPED_NAME, O_RDWR); //maybe add RDWR

    if(ReadMinerFD < 0) 
    {
        exit(1);
    }
}

void MinerSubscription() 
{
    numOfMiners++;
    ssize_t bytesWrite;

    string piped_name_str_2 = PATH_PIPED_NAME_SERVER_TO_MINER + to_string(numOfMiners);
    const char* write_piped_name = piped_name_str_2.c_str();

    int Write_fd = open(write_piped_name, O_WRONLY);

    if(Write_fd < 0) 
    {
        exit(1);
    }

    WriteMinerFD.push_back(Write_fd);
    Block currentBlock = blockchain.back();
    cout << "wrote genesis block: " << currentBlock.hash << " and its height: " << currentBlock.m_Block.height << " difficulty: " << currentBlock.m_Block.difficulty << endl;
    bytesWrite = write(Write_fd, &blockchain.back(), sizeof(Block)); // writes the last block in the chain
    if (bytesWrite == -1) {
        perror("write");
        exit(1);
    }
    // write to the common file
    //--------------------------------------------------
    FILE* file = fopen(PATH_OF_COMMON_FILE, "r+");
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
}

void broadcastBlockToAllMiners()
{
    ssize_t bytesSend;

    for (int i = 0; i < numOfMiners;i++)
    {
        bytesSend = write(WriteMinerFD[i],&blockchain.back() ,sizeof(Block));

        if (bytesSend == -1) 
        {
            perror("write");
            exit(1);
        } 
    }
}


void serverLoop() 
{
    const char* outputFilePath = "/var/log/mtacoin.log";

    InitServer();

    redirectOutputToFile(outputFilePath);

   
    TLV tlv;
    ssize_t bytesRead;

    BlockForHash genesisBlock1(0, time(NULL), 0, g_Difficulty, 1, 0);
    genesisBlock1.updateTimestamp();
    Block genesisBlock(genesisBlock1, 0);
    blockchain.push_back(genesisBlock);

    while (true) 
    {
        bytesRead = read(ReadMinerFD, &tlv, sizeof(TLV)); 
        if (bytesRead == -1) 
        {
            perror("write");
            exit(1);
        } 

        if(tlv.m_subscription)
        {
            MinerSubscription();
        }
        else
        {
            if(proofOfWork(tlv.m_Block))
            {
                Block testingBlock = tlv.m_Block;
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
}

int main()
{
    serverLoop();
    return 0;
}