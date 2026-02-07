#![no_std]
#![allow(internal_features)]
#![feature(lang_items)]
#![feature(core_intrinsics)]

use core::panic::PanicInfo;
use core::sync::atomic::{AtomicU32, Ordering};

#[panic_handler]
fn panic(_info: &PanicInfo) -> ! {
    loop {}
}

#[lang = "eh_personality"]
extern "C" fn eh_personality() {}

const TOTAL_PAGES: u32 = 1024;
const PAGE_SIZE: u32 = 4096;

static ALLOCATED_PAGES: AtomicU32 = AtomicU32::new(0);
static NEXT_PAGE: AtomicU32 = AtomicU32::new(0x100000);

struct MemoryManager {
    total_pages: u32,
    free_pages: u32,
}

static mut MEMORY_MANAGER: MemoryManager = MemoryManager {
    total_pages: TOTAL_PAGES,
    free_pages: TOTAL_PAGES,
};

#[no_mangle]
pub extern "C" fn rust_memory_init() {
    unsafe {
        MEMORY_MANAGER.total_pages = TOTAL_PAGES;
        MEMORY_MANAGER.free_pages = TOTAL_PAGES;
    }
    ALLOCATED_PAGES.store(0, Ordering::SeqCst);
    NEXT_PAGE.store(0x100000, Ordering::SeqCst);
}

#[no_mangle]
pub extern "C" fn rust_allocate_page() -> u32 {
    let allocated = ALLOCATED_PAGES.fetch_add(1, Ordering::SeqCst);
    
    if allocated >= TOTAL_PAGES {
        return 0;
    }
    
    unsafe {
        MEMORY_MANAGER.free_pages -= 1;
    }
    
    let page = NEXT_PAGE.fetch_add(PAGE_SIZE, Ordering::SeqCst);
    page
}

#[no_mangle]
pub extern "C" fn rust_free_page(_page: u32) {
    ALLOCATED_PAGES.fetch_sub(1, Ordering::SeqCst);
    unsafe {
        MEMORY_MANAGER.free_pages += 1;
    }
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
    
    while n > 0 {
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
    let free = unsafe { MEMORY_MANAGER.free_pages };
    
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
    unsafe { MEMORY_MANAGER.free_pages * PAGE_SIZE }
}
