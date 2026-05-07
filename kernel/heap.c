#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define HEAP_BASE           0x00200000U
#define HEAP_SIZE           0x00100000U
#define HEAP_END            (HEAP_BASE + HEAP_SIZE)
#define HEAP_ALIGN          16U
#define HEAP_MIN_SPLIT_SIZE (sizeof(heap_block_t) + HEAP_ALIGN)

typedef struct heap_block {
    size_t             size;
    bool               used;
    struct heap_block *next;
} heap_block_t;

static heap_block_t *heap_head  = NULL;
static uint32_t      heap_used  = 0;
static uint32_t      heap_total = HEAP_SIZE;

void heap_init(void) {
    heap_head = (heap_block_t *)HEAP_BASE;
    heap_head->size = HEAP_SIZE - sizeof(heap_block_t);
    heap_head->used = false;
    heap_head->next = NULL;
    heap_used = 0;
}

static void split_block(heap_block_t *block, size_t size) {
    if (block->size < size + HEAP_MIN_SPLIT_SIZE)
        return;

    heap_block_t *remainder    = (heap_block_t *)((uint8_t *)block + sizeof(heap_block_t) + size);
    remainder->size = block->size - size - sizeof(heap_block_t);
    remainder->used = false;
    remainder->next = block->next;
    block->size     = size;
    block->next     = remainder;
}

static void merge_free_blocks(void) {
    heap_block_t *cur = heap_head;
    while (cur && cur->next) {
        if (!cur->used && !cur->next->used) {
            cur->size += sizeof(heap_block_t) + cur->next->size;
            cur->next  = cur->next->next;
        } else {
            cur = cur->next;
        }
    }
}

void *kmalloc(size_t size) {
    if (size == 0)
        return NULL;

    size = (size + HEAP_ALIGN - 1) & ~(HEAP_ALIGN - 1);

    for (heap_block_t *cur = heap_head; cur; cur = cur->next) {
        if (!cur->used && cur->size >= size) {
            split_block(cur, size);
            cur->used = true;
            heap_used += size + sizeof(heap_block_t);
            return (void *)((uint8_t *)cur + sizeof(heap_block_t));
        }
    }
    return NULL;
}

void kfree(void *ptr) {
    if (!ptr)
        return;

    heap_block_t *block = (heap_block_t *)((uint8_t *)ptr - sizeof(heap_block_t));
    uint32_t addr = (uint32_t)block;
    if (addr < HEAP_BASE || addr >= HEAP_END || !block->used)
        return;

    block->used  = false;
    heap_used   -= block->size + sizeof(heap_block_t);
    merge_free_blocks();
}

void *krealloc(void *ptr, size_t size) {
    if (!ptr)
        return kmalloc(size);
    if (size == 0) {
        kfree(ptr);
        return NULL;
    }

    heap_block_t *block = (heap_block_t *)((uint8_t *)ptr - sizeof(heap_block_t));
    if (block->size >= size)
        return ptr;

    void *new_ptr = kmalloc(size);
    if (!new_ptr)
        return NULL;

    size_t copy_len = block->size < size ? block->size : size;
    uint8_t *src = (uint8_t *)ptr;
    uint8_t *dst = (uint8_t *)new_ptr;
    for (size_t i = 0; i < copy_len; i++)
        dst[i] = src[i];

    kfree(ptr);
    return new_ptr;
}

uint32_t heap_get_used(void) { return heap_used; }
uint32_t heap_get_free(void) { return heap_total - heap_used; }
