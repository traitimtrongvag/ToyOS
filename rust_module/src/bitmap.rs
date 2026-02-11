const BITMAP_SIZE: usize = 256;
const BITS_PER_BYTE: usize = 8;

pub struct BitmapAllocator {
    bitmap: [u8; BITMAP_SIZE],
    total_frames: usize,
    allocated_frames: usize,
}

impl BitmapAllocator {
    pub const fn new() -> Self {
        Self {
            bitmap: [0; BITMAP_SIZE],
            total_frames: BITMAP_SIZE * BITS_PER_BYTE,
            allocated_frames: 0,
        }
    }
    
    pub fn allocate_frame(&mut self) -> Option<usize> {
        for (byte_idx, byte) in self.bitmap.iter_mut().enumerate() {
            if *byte != 0xFF {
                for bit_idx in 0..BITS_PER_BYTE {
                    let mask = 1 << bit_idx;
                    
                    if (*byte & mask) == 0 {
                        *byte |= mask;
                        self.allocated_frames += 1;
                        return Some(byte_idx * BITS_PER_BYTE + bit_idx);
                    }
                }
            }
        }
        None
    }
    
    pub fn free_frame(&mut self, frame: usize) -> bool {
        if frame >= self.total_frames {
            return false;
        }
        
        if !self.is_allocated(frame) {
            return false;
        }
        
        let byte_idx = frame / BITS_PER_BYTE;
        let bit_idx = frame % BITS_PER_BYTE;
        let mask = !(1 << bit_idx);
        
        self.bitmap[byte_idx] &= mask;
        self.allocated_frames -= 1;
        true
    }
    
    pub fn is_allocated(&self, frame: usize) -> bool {
        if frame >= self.total_frames {
            return false;
        }
        
        let byte_idx = frame / BITS_PER_BYTE;
        let bit_idx = frame % BITS_PER_BYTE;
        let mask = 1 << bit_idx;
        
        (self.bitmap[byte_idx] & mask) != 0
    }
    
    pub fn get_free_count(&self) -> usize {
        self.total_frames - self.allocated_frames
    }
    
    pub fn get_allocated_count(&self) -> usize {
        self.allocated_frames
    }
}

