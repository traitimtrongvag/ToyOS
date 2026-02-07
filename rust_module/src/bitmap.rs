#![no_std]

const BITMAP_SIZE: usize = 128;

pub struct BitmapAllocator {
    bitmap: [u8; BITMAP_SIZE],
    total_frames: usize,
}

impl BitmapAllocator {
    pub const fn new() -> Self {
        BitmapAllocator {
            bitmap: [0; BITMAP_SIZE],
            total_frames: BITMAP_SIZE * 8,
        }
    }
    
    pub fn allocate_frame(&mut self) -> Option<usize> {
        for (byte_idx, byte) in self.bitmap.iter_mut().enumerate() {
            if *byte != 0xFF {
                for bit_idx in 0..8 {
                    let mask = 1 << bit_idx;
                    if (*byte & mask) == 0 {
                        *byte |= mask;
                        return Some(byte_idx * 8 + bit_idx);
                    }
                }
            }
        }
        None
    }
    
    pub fn free_frame(&mut self, frame: usize) {
        if frame >= self.total_frames {
            return;
        }
        
        let byte_idx = frame / 8;
        let bit_idx = frame % 8;
        let mask = !(1 << bit_idx);
        
        self.bitmap[byte_idx] &= mask;
    }
    
    pub fn is_allocated(&self, frame: usize) -> bool {
        if frame >= self.total_frames {
            return false;
        }
        
        let byte_idx = frame / 8;
        let bit_idx = frame % 8;
        let mask = 1 << bit_idx;
        
        (self.bitmap[byte_idx] & mask) != 0
    }
    
    pub fn count_free(&self) -> usize {
        let mut count = 0;
        for byte in &self.bitmap {
            for bit in 0..8 {
                if (*byte & (1 << bit)) == 0 {
                    count += 1;
                }
            }
        }
        count
    }
}
