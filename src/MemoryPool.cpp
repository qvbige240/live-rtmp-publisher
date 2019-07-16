#include "MemoryPool.h"

const int MemoryPool::mTable[MEM_TABLE_SIZE] = { 256, 512, 1024, 2048, 4096, 8192, 16384, 32768, 65536, 65536*2, 65536*3 };

MemoryPool::~MemoryPool() {
    char *head, *next;
    for (int i = 0; i != MEM_TABLE_SIZE; ++i) {
        head = mList[i];
        while (head != NULL) {
            next = *(reinterpret_cast<char**>(head));
            delete[] head;
            head = next;
        }
    }
}

char* MemoryPool::getChunk(int bytes) {
    int idx = getIndex(bytes);
    std::lock_guard<std::mutex> lock(mMutex);
    char *head = mList[idx];

    if (!head) {
        return new char[mTable[idx]];
    }

    mList[idx] = *(reinterpret_cast<char**>(head));

    return head;
}

void MemoryPool::putChunk(int bytes, char* data) {
    int idx = getIndex(bytes);
    std::lock_guard<std::mutex> lock(mMutex);

    *(reinterpret_cast<char**>(data)) = mList[idx];
    mList[idx] = data;
}

inline int MemoryPool::getIndex(int bytes) const {
    return std::upper_bound(mTable, mTable + MEM_TABLE_SIZE, bytes) - mTable;
}
