#include "memory_manager.h"

void *memory_;
dynamic_array_head blocks_;
size_t size_;

/// @brief loads up the memory with memory
/// @param size
void mem_init(size_t size) {
    memory_ = malloc(size);
    DA_init(&blocks_, (size / 64) + 1);
    size_ = size;
}

/// @brief returns pointer to memory block, NULL if no chunk of proper size
/// found
/// @param size
/// @return
void *mem_alloc(size_t size) {
    if (size > size_) return NULL;
    if(size == 0) return memory_; // wack
    memory_block *current_block = DA_get_first(&blocks_);
    if (current_block == NULL || current_block->start - memory_ >= size) {
        DA_add(&blocks_, (memory_block){memory_, memory_ + size - 1});
        return memory_;
    }
    memory_block *next_block = DA_get_next(&blocks_, current_block);
    while (next_block != NULL) {
        if (next_block->start - current_block->end - 1 >= size) {
            void *allocated = current_block->end + 1;
            DA_add(&blocks_, (memory_block){allocated, allocated + size - 1});
            return allocated;
        }
        current_block = next_block;
        next_block = DA_get_next(&blocks_, next_block);
    }
    if (memory_ + size_ - 1 - current_block->end >= size) {
        void *allocated = current_block->end + 1;
        DA_add(&blocks_, (memory_block){allocated, allocated + size - 1});
        return allocated;
    }
    return NULL;
}

void *mem_alloc_bestfit(size_t size) { return NULL; }

/// @brief Frees the memory block preventing memory leaks
/// @param block
void mem_free(void *block) {
    return DA_remove(&blocks_, DA_find(&blocks_, block));
}

/// @brief changes the size of the block, if possible without moving it, returns
/// NULL if failed
/// @param block
/// @param size
/// @return
void *mem_resize(void *block, size_t size) {
    if (!block) return mem_alloc(size);
    if (size == 0) {
        mem_free(block);
        return NULL;
    }
    memory_block *current_block = DA_find(&blocks_, block);
    if (!current_block) return NULL;
    memory_block *next_block = DA_get_next(&blocks_, current_block);
    void *next_block_start = (next_block) ? next_block->start : memory_ + size_;
    if (next_block_start - current_block->start >= size) {
        current_block->end = current_block->start + size - 1;
        return current_block->start;
    }
    memory_block temp = *current_block;
    mem_free(current_block);
    void *new_block = mem_alloc(size);
    if (new_block) {
        size_t copy_size = (size > temp.end - temp.start + 1) ? size : temp.end - temp.start + 1;
        memcpy(new_block, temp.start, copy_size);
        return new_block;
    }
    *current_block = temp;
    return NULL;
}

/// @brief gives back the memory used by the memory manager
void mem_deinit() {
    free(memory_);
    DA_deinit(&blocks_);
}