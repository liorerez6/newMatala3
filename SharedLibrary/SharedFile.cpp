#include "SharedFile.h"
#include <zlib.h>

// BlockForHash implementations
BlockForHash::BlockForHash(int i_height, int i_timestamp, unsigned int i_prev_hash, int i_difficulty, int i_nonce, int i_relayed_by)
    : height(i_height), timestamp(i_timestamp), prev_hash(i_prev_hash),
      difficulty(i_difficulty), nonce(i_nonce), relayed_by(i_relayed_by) {}

void BlockForHash::updateTimestamp() {
    timestamp = static_cast<int>(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));
}

BlockForHash::BlockForHash() : height(0), timestamp(0), prev_hash(0), difficulty(0), nonce(0), relayed_by(0) {}

// Block implementations
Block::Block(BlockForHash i_Block, int i_hash) : m_Block(i_Block), hash(i_hash) {}
Block::Block() : m_Block(), hash(0) {}

// CRC32 calculation
uLong calculateCRC32(BlockForHash block) {
    uLong crc = crc32(0L, Z_NULL, 0);
    crc = crc32(crc, (const Bytef*)&block, sizeof(block));
    return crc;
}

// Difficulty mask check
bool maskCheckForDifficulty(int i_Difficulty, int i_hash) {
    uLong mask = 0xFFFFFFFF;
    mask <<= (32 - i_Difficulty);
    return (!(mask & i_hash));
}

void redirectOutputToFile(const char* filepath) {
    // Open the file for writing
    int fileDescriptor = open(filepath, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
    if (fileDescriptor < 0) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    // Redirect stdout to the file
    if (dup2(fileDescriptor, STDOUT_FILENO) < 0) {
        perror("dup2");
        close(fileDescriptor);
        exit(EXIT_FAILURE);
    }

    // Close the original file descriptor
    close(fileDescriptor);
}