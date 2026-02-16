#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define HEAP_START 0x00100000
#define HEAP_SIZE 0x00100000
#define BLOCK_SIZE 16

typedef struct heap_block {
    size_t size;
    bool used;
    struct heap_block* next;
} heap_block_t;

static heap_block_t* heap_start = NULL;
static uint32_t heap_used = 0;
static uint32_t heap_total = HEAP_SIZE;

void heap_init(void) {
    heap_start = (heap_block_t*)HEAP_START;
    heap_start->size = HEAP_SIZE - sizeof(heap_block_t);
    heap_start->used = false;
    heap_start->next = NULL;
    heap_used = 0;
}

static void split_block(heap_block_t* block, size_t size) {
    if (block->size >= size + sizeof(heap_block_t) + BLOCK_SIZE) {
        heap_block_t* new_block = (heap_block_t*)((uint8_t*)block + sizeof(heap_block_t) + size);
        new_block->size = block->size - size - sizeof(heap_block_t);
        new_block->used = false;
        new_block->next = block->next;
        block->size = size;
        block->next = new_block;
    }
}

static void merge_free_blocks(void) {
    heap_block_t* current = heap_start;
    while (current && current->next) {
        if (!current->used && !current->next->used) {
            current->size += sizeof(heap_block_t) + current->next->size;
            current->next = current->next->next;
        } else {
            current = current->next;
        }
    }
}

void* kmalloc(size_t size) {
    if (size == 0) return NULL;
    size = (size + BLOCK_SIZE - 1) & ~(BLOCK_SIZE - 1);
    heap_block_t* current = heap_start;
    while (current) {
        if (!current->used && current->size >= size) {
            split_block(current, size);
            current->used = true;
            heap_used += size + sizeof(heap_block_t);
            return (void*)((uint8_t*)current + sizeof(heap_block_t));
        }
        current = current->next;
    }
    return NULL;
}

void kfree(void* ptr) {
    if (!ptr) return;
    heap_block_t* block = (heap_block_t*)((uint8_t*)ptr - sizeof(heap_block_t));
    if ((uint32_t)block < HEAP_START || (uint32_t)block >= HEAP_START + HEAP_SIZE) return;
    if (!block->used) return;
    block->used = false;
    heap_used -= block->size + sizeof(heap_block_t);
    merge_free_blocks();
}

void* krealloc(void* ptr, size_t size) {
    if (!ptr) return kmalloc(size);
    if (size == 0) { kfree(ptr); return NULL; }
    heap_block_t* block = (heap_block_t*)((uint8_t*)ptr - sizeof(heap_block_t));
    if (block->size >= size) return ptr;
    void* new_ptr = kmalloc(size);
    if (!new_ptr) return NULL;
    uint8_t* src = (uint8_t*)ptr;
    uint8_t* dst = (uint8_t*)new_ptr;
    for (size_t i = 0; i < block->size; i++) dst[i] = src[i];
    kfree(ptr);
    return new_ptr;
}

uint32_t heap_get_used(void) { return heap_used; }
uint32_t heap_get_free(void) { return heap_total - heap_used; }
