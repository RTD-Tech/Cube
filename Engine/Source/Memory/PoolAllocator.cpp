#include "PoolAllocator.h"
#include <cstring>

namespace SquareCore {

PoolAllocator::PoolAllocator(int size, int count) : size(size), count(count), usedNum(0) {
    memory = new char[size * count];
    used = new bool[count];
    std::memset(used, 0, count * sizeof(bool));
}

PoolAllocator::~PoolAllocator() {
    delete[] memory;
    delete[] used;
}

int PoolAllocator::Alloc() {
    for (int i = 0; i < count; i++) {
        if (!used[i]) {
            used[i] = true;
            usedNum++;
            return i;
        }
    }
    return -1;
}

void PoolAllocator::FreeSlot(int id) {
    if (id >= 0 && id < count && used[id]) {
        used[id] = false;
        usedNum--;
    }
}

void* PoolAllocator::GetPointer(int id) {
    if (id < 0 || id >= count) {
        return nullptr;
    }
    return memory + (id * size);
}

int PoolAllocator::GetUsed() const {
    return usedNum;
}

int PoolAllocator::GetTotal() const {
    return count;
}

float PoolAllocator::GetUsedPercent() const {
    if (count == 0) {
        return 0.0f;
    }
    return (float)usedNum / (float)count * 100.0f;
}

}
