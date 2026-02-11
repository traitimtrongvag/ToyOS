const POOL_BLOCK_SIZE: usize = 64;
const POOL_BLOCK_COUNT: usize = 256;

#[repr(C, align(64))]
struct PoolBlock {
    data: [u8; POOL_BLOCK_SIZE],
}

pub struct MemoryPool {
    blocks: [PoolBlock; POOL_BLOCK_COUNT],
    free_list: [bool; POOL_BLOCK_COUNT],
    allocated_count: usize,
}

impl MemoryPool {
    pub const fn new() -> Self {
        const EMPTY_BLOCK: PoolBlock = PoolBlock {
            data: [0; POOL_BLOCK_SIZE],
        };
        
        Self {
            blocks: [EMPTY_BLOCK; POOL_BLOCK_COUNT],
            free_list: [true; POOL_BLOCK_COUNT],
            allocated_count: 0,
        }
    }
    
    pub fn allocate_block(&mut self) -> Option<*mut u8> {
        for (idx, is_free) in self.free_list.iter_mut().enumerate() {
            if *is_free {
                *is_free = false;
                self.allocated_count += 1;
                return Some(self.blocks[idx].data.as_mut_ptr());
            }
        }
        None
    }
    
    pub fn free_block(&mut self, ptr: *mut u8) -> bool {
        let pool_start = self.blocks[0].data.as_ptr() as usize;
        let pool_end = pool_start + (POOL_BLOCK_SIZE * POOL_BLOCK_COUNT);
        let addr = ptr as usize;
        
        if addr < pool_start || addr >= pool_end {
            return false;
        }
        
        if (addr - pool_start) % POOL_BLOCK_SIZE != 0 {
            return false;
        }
        
        let block_idx = (addr - pool_start) / POOL_BLOCK_SIZE;
        
        if self.free_list[block_idx] {
            return false;
        }
        
        self.free_list[block_idx] = true;
        self.allocated_count -= 1;
        true
    }
    
    pub fn get_allocated_count(&self) -> usize {
        self.allocated_count
    }
    
    pub fn get_free_count(&self) -> usize {
        POOL_BLOCK_COUNT - self.allocated_count
    }
}
