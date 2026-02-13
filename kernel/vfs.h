#ifndef VFS_H
#define VFS_H

#include <stdint.h>
#include <stddef.h>

#define VFS_TYPE_REGULAR 0
#define VFS_TYPE_DIRECTORY 1

void rust_vfs_init(void);
int32_t rust_vfs_create(const uint8_t* name, size_t name_len, uint8_t file_type);
int32_t rust_vfs_write(const uint8_t* name, size_t name_len, const uint8_t* data, size_t data_len);
int32_t rust_vfs_read(const uint8_t* name, size_t name_len, uint8_t* buf, size_t buf_len);

#endif
