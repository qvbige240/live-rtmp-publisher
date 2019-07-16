#ifndef _MEMORY_POOL_H_
#define _MEMORY_POOL_H_

#include <cstring>
#include <mutex>
#include <algorithm>

#define MEM_TABLE_SIZE      11

class MemoryPool {
public:
    MemoryPool() { memset(mList, 0, MEM_TABLE_SIZE * sizeof(char*)); }

    ~MemoryPool();

    char* getChunk(int bytes);

    void putChunk(int bytes, char* data);
private:
    int getIndex(int bytes) const;
private:
    std::mutex mMutex;
    char *mList[MEM_TABLE_SIZE];
    static const int mTable[MEM_TABLE_SIZE];
};

#endif
