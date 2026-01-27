#ifndef ALLOCATOR_H
#define ALLOCATOR_H

namespace SquareCore {

class PoolAllocator {
public:
    // Creates a pool allocator with specified slot size and count
    PoolAllocator(int size, int count);
    // Destroys the allocator and frees all reserved memory
    ~PoolAllocator();
    
    // Allocates a slot from the pool and returns its ID (-1 if pool is full)
    int Alloc();
    // Frees a specific slot by ID, making it available for reuse
    void FreeSlot(int id);
    // Gets a pointer to the memory at the specified slot ID (nullptr if invalid)
    void* GetPointer(int id);
    // Returns the number of currently allocated slots
    int GetUsed() const;
    // Returns the total number of slots in the pool
    int GetTotal() const;
    // Returns the percentage of used slots (0.0-100.0)
    float GetUsedPercent() const;

private:
    int size;           // Size of each slot in bytes
    int count;          // Total number of slots
    char* memory;       // Pointer to the allocated memory pool
    bool* used;         // Array tracking which slots are allocated
    int usedNum;        // Current number of used slots
};

}

#endif
