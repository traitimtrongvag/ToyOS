#![no_std]
#![allow(internal_features)]
#![feature(lang_items)]
#![feature(core_intrinsics)]

use core::panic::PanicInfo;
use core::sync::atomic::{AtomicU32, Ordering};
use core::cell::UnsafeCell;

#[panic_handler]
fn panic(_info: &PanicInfo) -> ! {
    loop {}
}

#[lang = "eh_personality"]
extern "C" fn eh_personality() {}

const TOTAL_PAGES: u32 = 1024;
const PAGE_SIZE: u32 = 4096;
const MIN_PAGE_ADDR: u32 = 0x100000; // 1MB
const MAX_PAGE_ADDR: u32 = MIN_PAGE_ADDR + (TOTAL_PAGES * PAGE_SIZE);

static ALLOCATED_PAGES: AtomicU32 = AtomicU32::new(0);
static NEXT_PAGE: AtomicU32 = AtomicU32::new(MIN_PAGE_ADDR);

// Thread-safe wrapper for MemoryManager using UnsafeCell
struct MemoryManager {
    total_pages: u32,
    free_pages: UnsafeCell<u32>,
}

// Safety: We ensure single-threaded access in our kernel
unsafe impl Sync for MemoryManager {}

impl MemoryManager {
    const fn new() -> Self {
        Self {
            total_pages: TOTAL_PAGES,
            free_pages: UnsafeCell::new(TOTAL_PAGES),
        }
    }
    
    fn get_free_pages(&self) -> u32 {
        unsafe { *self.free_pages.get() }
    }
    
    fn decrement_free_pages(&self) {
        unsafe {
            let free = self.free_pages.get();
            if *free > 0 {
                *free -= 1;
            }
        }
    }
    
    fn increment_free_pages(&self) {
        unsafe {
            let free = self.free_pages.get();
            if *free < TOTAL_PAGES {
                *free += 1;
            }
        }
    }
    
    fn reset(&self) {
        unsafe {
            *self.free_pages.get() = TOTAL_PAGES;
        }
    }
}

static MEMORY_MANAGER: MemoryManager = MemoryManager::new();

#[no_mangle]
pub extern "C" fn rust_memory_init() {
    MEMORY_MANAGER.reset();
    ALLOCATED_PAGES.store(0, Ordering::SeqCst);
    NEXT_PAGE.store(MIN_PAGE_ADDR, Ordering::SeqCst);
}

#[no_mangle]
pub extern "C" fn rust_allocate_page() -> u32 {
    // Check if we've exceeded the page limit
    let allocated = ALLOCATED_PAGES.load(Ordering::SeqCst);
    if allocated >= TOTAL_PAGES {
        return 0; // Out of memory
    }
    
    // Atomically increment and get the page address
    let page = NEXT_PAGE.fetch_add(PAGE_SIZE, Ordering::SeqCst);
    
    // Validate page address is within bounds
    if page >= MAX_PAGE_ADDR {
        return 0; // Address out of range
    }
    
    // Update allocation counters
    ALLOCATED_PAGES.fetch_add(1, Ordering::SeqCst);
    MEMORY_MANAGER.decrement_free_pages();
    
    page
}

#[no_mangle]
pub extern "C" fn rust_free_page(page: u32) {
    // Validate page address
    if page < MIN_PAGE_ADDR || page >= MAX_PAGE_ADDR {
        return; // Invalid page address
    }
    
    // Check alignment
    if page % PAGE_SIZE != 0 {
        return; // Page not properly aligned
    }
    
    // Check if we have pages to free
    let allocated = ALLOCATED_PAGES.load(Ordering::SeqCst);
    if allocated == 0 {
        return; // Nothing to free
    }
    
    ALLOCATED_PAGES.fetch_sub(1, Ordering::SeqCst);
    MEMORY_MANAGER.increment_free_pages();
}

extern "C" {
    fn terminal_writestring(s: *const u8);
    fn terminal_putchar(c: u8);
}

fn print_str(s: &str) {
    unsafe {
        terminal_writestring(s.as_ptr());
    }
}

fn print_u32(num: u32) {
    let mut buffer = [0u8; 10];
    let mut n = num;
    let mut i = 0;
    
    if n == 0 {
        unsafe { terminal_putchar(b'0'); }
        return;
    }
    
    while n > 0 && i < buffer.len() {
        buffer[i] = (n % 10) as u8 + b'0';
        n /= 10;
        i += 1;
    }
    
    while i > 0 {
        i -= 1;
        unsafe { terminal_putchar(buffer[i]); }
    }
}

#[no_mangle]
pub extern "C" fn rust_print_stats() {
    let allocated = ALLOCATED_PAGES.load(Ordering::SeqCst);
    let free = MEMORY_MANAGER.get_free_pages();
    
    print_str("  Total pages: ");
    print_u32(TOTAL_PAGES);
    print_str("\n  Allocated: ");
    print_u32(allocated);
    print_str("\n  Free: ");
    print_u32(free);
    print_str("\n");
}

#[no_mangle]
pub extern "C" fn rust_get_total_memory() -> u32 {
    TOTAL_PAGES * PAGE_SIZE
}

#[no_mangle]
pub extern "C" fn rust_get_free_memory() -> u32 {
    MEMORY_MANAGER.get_free_pages() * PAGE_SIZE
}

#[no_mangle]
pub extern "C" fn rust_get_allocated_memory() -> u32 {
    ALLOCATED_PAGES.load(Ordering::SeqCst) * PAGE_SIZE
}

// Additional utility function to check if an address is valid
#[no_mangle]
pub extern "C" fn rust_is_valid_page(page: u32) -> bool {
    page >= MIN_PAGE_ADDR && 
    page < MAX_PAGE_ADDR && 
    page % PAGE_SIZE == 0
}
