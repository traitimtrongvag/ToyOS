use core::fmt;

pub const MAX_FILENAME: usize = 255;
pub const MAX_PATH: usize = 4096;

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum FileType {
    Regular,
    Directory,
    CharDevice,
    BlockDevice,
    SymLink,
}

#[derive(Debug, Clone, Copy)]
pub struct FilePermissions {
    pub read: bool,
    pub write: bool,
    pub execute: bool,
}

impl FilePermissions {
    pub const fn new(read: bool, write: bool, execute: bool) -> Self {
        Self { read, write, execute }
    }

    pub const fn readonly() -> Self {
        Self::new(true, false, false)
    }

    pub const fn readwrite() -> Self {
        Self::new(true, true, false)
    }

    pub const fn executable() -> Self {
        Self::new(true, false, true)
    }
}

#[derive(Debug, Clone, Copy)]
pub struct FileMetadata {
    pub file_type: FileType,
    pub permissions: FilePermissions,
    pub size: u64,
    pub inode: u32,
}

impl FileMetadata {
    pub const fn new(file_type: FileType, permissions: FilePermissions, size: u64, inode: u32) -> Self {
        Self {
            file_type,
            permissions,
            size,
            inode,
        }
    }

    pub fn is_dir(&self) -> bool {
        self.file_type == FileType::Directory
    }

    pub fn is_file(&self) -> bool {
        self.file_type == FileType::Regular
    }
}

#[derive(Debug)]
pub enum VfsError {
    NotFound,
    PermissionDenied,
    AlreadyExists,
    NotDirectory,
    IsDirectory,
    InvalidPath,
    OutOfSpace,
    IoError,
}

impl fmt::Display for VfsError {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        match self {
            VfsError::NotFound => write!(f, "file not found"),
            VfsError::PermissionDenied => write!(f, "permission denied"),
            VfsError::AlreadyExists => write!(f, "file already exists"),
            VfsError::NotDirectory => write!(f, "not a directory"),
            VfsError::IsDirectory => write!(f, "is a directory"),
            VfsError::InvalidPath => write!(f, "invalid path"),
            VfsError::OutOfSpace => write!(f, "out of space"),
            VfsError::IoError => write!(f, "io error"),
        }
    }
}

pub type VfsResult<T> = Result<T, VfsError>;

pub trait VfsNode {
    fn name(&self) -> &str;
    fn metadata(&self) -> &FileMetadata;
    fn read(&self, buf: &mut [u8], offset: u64) -> VfsResult<usize>;
    fn write(&mut self, buf: &[u8], offset: u64) -> VfsResult<usize>;
}

pub trait VfsDirectory {
    fn lookup(&self, name: &str) -> VfsResult<&dyn VfsNode>;
    fn create(&mut self, name: &str, file_type: FileType) -> VfsResult<()>;
    fn remove(&mut self, name: &str) -> VfsResult<()>;
    fn list(&self) -> VfsResult<&[&str]>;
}

pub struct VfsMount {
    mount_point: [u8; MAX_PATH],
    mount_point_len: usize,
}

impl VfsMount {
    pub const fn new() -> Self {
        Self {
            mount_point: [0; MAX_PATH],
            mount_point_len: 0,
        }
    }

    pub fn mount(&mut self, path: &str) -> VfsResult<()> {
        let bytes = path.as_bytes();
        if bytes.len() >= MAX_PATH {
            return Err(VfsError::InvalidPath);
        }

        self.mount_point[..bytes.len()].copy_from_slice(bytes);
        self.mount_point_len = bytes.len();
        Ok(())
    }

    pub fn path(&self) -> &str {
        core::str::from_utf8(&self.mount_point[..self.mount_point_len])
            .unwrap_or("")
    }
}
