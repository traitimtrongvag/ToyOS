#![allow(static_mut_refs)]
use crate::vfs::{
    FileMetadata, FilePermissions, FileType, VfsDirectory, VfsError, VfsNode, VfsResult,
    MAX_FILENAME,
};
use core::cmp;

const MAX_FILES: usize = 64;
const MAX_FILE_SIZE: usize = 4096;

#[derive(Clone)]
struct MemFile {
    name: [u8; MAX_FILENAME],
    name_len: usize,
    metadata: FileMetadata,
    data: [u8; MAX_FILE_SIZE],
    data_len: usize,
}

impl MemFile {
    fn new(name: &str, file_type: FileType, permissions: FilePermissions) -> Self {
        let mut name_buf = [0u8; MAX_FILENAME];
        let name_bytes = name.as_bytes();
        let name_len = cmp::min(name_bytes.len(), MAX_FILENAME);
        name_buf[..name_len].copy_from_slice(&name_bytes[..name_len]);

        Self {
            name: name_buf,
            name_len,
            metadata: FileMetadata::new(file_type, permissions, 0, 0),
            data: [0; MAX_FILE_SIZE],
            data_len: 0,
        }
    }

    fn name_str(&self) -> &str {
        core::str::from_utf8(&self.name[..self.name_len]).unwrap_or("")
    }
}

impl VfsNode for MemFile {
    fn name(&self) -> &str {
        self.name_str()
    }

    fn metadata(&self) -> &FileMetadata {
        &self.metadata
    }

    fn read(&self, buf: &mut [u8], offset: u64) -> VfsResult<usize> {
        if self.metadata.file_type == FileType::Directory {
            return Err(VfsError::IsDirectory);
        }

        let offset = offset as usize;
        if offset >= self.data_len {
            return Ok(0);
        }

        let to_read = cmp::min(buf.len(), self.data_len - offset);
        buf[..to_read].copy_from_slice(&self.data[offset..offset + to_read]);
        Ok(to_read)
    }

    fn write(&mut self, buf: &[u8], offset: u64) -> VfsResult<usize> {
        if self.metadata.file_type == FileType::Directory {
            return Err(VfsError::IsDirectory);
        }

        if !self.metadata.permissions.write {
            return Err(VfsError::PermissionDenied);
        }

        let offset = offset as usize;
        if offset + buf.len() > MAX_FILE_SIZE {
            return Err(VfsError::OutOfSpace);
        }

        let to_write = cmp::min(buf.len(), MAX_FILE_SIZE - offset);
        self.data[offset..offset + to_write].copy_from_slice(&buf[..to_write]);
        self.data_len = cmp::max(self.data_len, offset + to_write);
        self.metadata.size = self.data_len as u64;
        Ok(to_write)
    }
}

pub struct MemFs {
    files: [Option<MemFile>; MAX_FILES],
    file_count: usize,
}

impl MemFs {
    pub const fn new() -> Self {
        const NONE_FILE: Option<MemFile> = None;
        Self {
            files: [NONE_FILE; MAX_FILES],
            file_count: 0,
        }
    }

    fn find_file(&self, name: &str) -> Option<usize> {
        for (i, file) in self.files.iter().enumerate() {
            if let Some(f) = file {
                if f.name_str() == name {
                    return Some(i);
                }
            }
        }
        None
    }
}

impl VfsDirectory for MemFs {
    fn lookup(&self, name: &str) -> VfsResult<&dyn VfsNode> {
        match self.find_file(name) {
            Some(idx) => Ok(self.files[idx].as_ref().unwrap()),
            None => Err(VfsError::NotFound),
        }
    }

    fn create(&mut self, name: &str, file_type: FileType) -> VfsResult<()> {
        if self.find_file(name).is_some() {
            return Err(VfsError::AlreadyExists);
        }

        if self.file_count >= MAX_FILES {
            return Err(VfsError::OutOfSpace);
        }

        let permissions = match file_type {
            FileType::Directory => FilePermissions::readonly(),
            _ => FilePermissions::readwrite(),
        };

        for slot in &mut self.files {
            if slot.is_none() {
                *slot = Some(MemFile::new(name, file_type, permissions));
                self.file_count += 1;
                return Ok(());
            }
        }

        Err(VfsError::OutOfSpace)
    }

    fn remove(&mut self, name: &str) -> VfsResult<()> {
        match self.find_file(name) {
            Some(idx) => {
                self.files[idx] = None;
                self.file_count -= 1;
                Ok(())
            }
            None => Err(VfsError::NotFound),
        }
    }

    fn list(&self) -> VfsResult<&[&str]> {
        Err(VfsError::IoError)
    }
}

static mut GLOBAL_MEMFS: MemFs = MemFs::new();

#[no_mangle]
pub extern "C" fn rust_vfs_init() {
    unsafe {
        GLOBAL_MEMFS = MemFs::new();
    }
}

#[no_mangle]
pub extern "C" fn rust_vfs_create(name_ptr: *const u8, name_len: usize, file_type: u8) -> i32 {
    if name_ptr.is_null() {
        return -1;
    }

    let name_slice = unsafe { core::slice::from_raw_parts(name_ptr, name_len) };
    let name = match core::str::from_utf8(name_slice) {
        Ok(s) => s,
        Err(_) => return -1,
    };

    let ftype = match file_type {
        0 => FileType::Regular,
        1 => FileType::Directory,
        _ => return -1,
    };

    unsafe {
        match GLOBAL_MEMFS.create(name, ftype) {
            Ok(_) => 0,
            Err(_) => -1,
        }
    }
}

#[no_mangle]
pub extern "C" fn rust_vfs_write(
    name_ptr: *const u8,
    name_len: usize,
    data_ptr: *const u8,
    data_len: usize,
) -> i32 {
    if name_ptr.is_null() || data_ptr.is_null() {
        return -1;
    }

    let name_slice = unsafe { core::slice::from_raw_parts(name_ptr, name_len) };
    let name = match core::str::from_utf8(name_slice) {
        Ok(s) => s,
        Err(_) => return -1,
    };

    let data_slice = unsafe { core::slice::from_raw_parts(data_ptr, data_len) };

    unsafe {
        let file_idx = match GLOBAL_MEMFS.find_file(name) {
            Some(idx) => idx,
            None => return -1,
        };

        match &mut GLOBAL_MEMFS.files[file_idx] {
            Some(file) => match file.write(data_slice, 0) {
                Ok(written) => written as i32,
                Err(_) => -1,
            },
            None => -1,
        }
    }
}

#[no_mangle]
pub extern "C" fn rust_vfs_read(
    name_ptr: *const u8,
    name_len: usize,
    buf_ptr: *mut u8,
    buf_len: usize,
) -> i32 {
    if name_ptr.is_null() || buf_ptr.is_null() {
        return -1;
    }

    let name_slice = unsafe { core::slice::from_raw_parts(name_ptr, name_len) };
    let name = match core::str::from_utf8(name_slice) {
        Ok(s) => s,
        Err(_) => return -1,
    };

    let buf_slice = unsafe { core::slice::from_raw_parts_mut(buf_ptr, buf_len) };

    unsafe {
        let file_idx = match GLOBAL_MEMFS.find_file(name) {
            Some(idx) => idx,
            None => return -1,
        };

        match &GLOBAL_MEMFS.files[file_idx] {
            Some(file) => match file.read(buf_slice, 0) {
                Ok(read) => read as i32,
                Err(_) => -1,
            },
            None => -1,
        }
    }
}
