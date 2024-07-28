#ifndef COMMON_H
#define COMMON_H

#include <vector>
#include <chrono>
#include <iostream>
#include <pthread.h>
#include <zlib.h>
#include <list>
#include <unistd.h> // For sleep function
#include <string.h>
#include <csignal>
#include <cstring>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define PATH_PIPED_NAME_SERVER_TO_MINER "/app/mnt/mta/Piped_Server_To_Miner_"
#define PATH_OF_COMMON_FILE "/app/mnt/mta/CommonFile.conf"
#define PATH_PIPED_NAME_MINER_TO_SERVER "/app/mnt/mta/Piped_Miner_To_Server"

class BlockForHash {
public:
    int height;
    int timestamp;
    unsigned int prev_hash;
    int difficulty;
    int nonce;
    int relayed_by;

    // Constructor
    BlockForHash(int i_height, int i_timestamp, unsigned int i_prev_hash, int i_difficulty, int i_nonce, int i_relayed_by);

    void updateTimestamp();

    BlockForHash();
};

class Block 
{
public:
    BlockForHash m_Block;
    unsigned int hash;
    Block(BlockForHash i_Block, int i_hash);
    Block();
};

class TLV
{
public:

    bool m_subscription;
    Block m_Block;
    int m_minerId;
};


uLong calculateCRC32(BlockForHash block);
bool maskCheckForDifficulty(int i_Difficulty, int i_hash);
void redirectOutputToFile(const char* filepath);

#endif