pub fn align_up(addr: usize, align: usize) -> usize {
    if align == 0 {
        return addr;
    }
    (addr + align - 1) & !(align - 1)
}

pub fn align_down(addr: usize, align: usize) -> usize {
    if align == 0 {
        return addr;
    }
    addr & !(align - 1)
}

pub fn is_aligned(addr: usize, align: usize) -> bool {
    if align == 0 {
        return true;
    }
    (addr & (align - 1)) == 0
}

pub fn copy_memory(dest: *mut u8, src: *const u8, count: usize) {
    unsafe {
        for i in 0..count {
            *dest.add(i) = *src.add(i);
        }
    }
}

pub fn set_memory(dest: *mut u8, value: u8, count: usize) {
    unsafe {
        for i in 0..count {
            *dest.add(i) = value;
        }
    }
}

pub fn compare_memory(ptr1: *const u8, ptr2: *const u8, count: usize) -> bool {
    unsafe {
        for i in 0..count {
            if *ptr1.add(i) != *ptr2.add(i) {
                return false;
            }
        }
    }
    true
}

pub fn string_length(s: *const u8) -> usize {
    let mut len = 0;
    unsafe {
        while *s.add(len) != 0 {
            len += 1;
        }
    }
    len
}

pub fn string_copy(dest: *mut u8, src: *const u8, max_len: usize) -> usize {
    let mut copied = 0;
    unsafe {
        while copied < max_len {
            let byte = *src.add(copied);
            if byte == 0 {
                break;
            }
            *dest.add(copied) = byte;
            copied += 1;
        }
        if copied < max_len {
            *dest.add(copied) = 0;
        }
    }
    copied
}
