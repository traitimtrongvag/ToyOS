#![no_std]
#![allow(internal_features)]
#![feature(lang_items)]
#![feature(core_intrinsics)]

use core::panic::PanicInfo;
use core::sync::atomic::{AtomicU32, Ordering};
use core::cell::UnsafeCell;

pub mod bitmap;
pub mod scheduler;
pub mod memory_pool;
pub mod process;
pub mod ipc;
pub mod utils;

use memory_pool::MemoryPool;
use process::ProcessManager;
use ipc::MessageQueue;

#[panic_handler]
fn panic(_info: &PanicInfo) -> ! {
    loop {}
}

#[lang = "eh_personality"]
extern "C" fn eh_personality() {}

const TOTAL_PAGES: u32 = 1024;
const PAGE_SIZE: u32 = 4096;
const MIN_PAGE_ADDR: u32 = 0x100000;
const MAX_PAGE_ADDR: u32 = MIN_PAGE_ADDR + (TOTAL_PAGES * PAGE_SIZE);

static ALLOCATED_PAGES: AtomicU32 = AtomicU32::new(0);
static NEXT_PAGE: AtomicU32 = AtomicU32::new(MIN_PAGE_ADDR);

struct MemoryManager {
    total_pages: u32,
    free_pages: UnsafeCell<u32>,
}

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
static mut GLOBAL_MEMORY_POOL: MemoryPool = MemoryPool::new();
static mut GLOBAL_PROCESS_MANAGER: ProcessManager = ProcessManager::new();
static mut GLOBAL_MESSAGE_QUEUE: MessageQueue = MessageQueue::new();

#[no_mangle]
pub extern "C" fn rust_memory_init() {
    MEMORY_MANAGER.reset();
    ALLOCATED_PAGES.store(0, Ordering::SeqCst);
    NEXT_PAGE.store(MIN_PAGE_ADDR, Ordering::SeqCst);
}

#[no_mangle]
pub extern "C" fn rust_allocate_page() -> u32 {
    let allocated = ALLOCATED_PAGES.load(Ordering::SeqCst);
    if allocated >= TOTAL_PAGES {
        return 0;
    }
    
    loop {
        let current_page = NEXT_PAGE.load(Ordering::SeqCst);
        
        if current_page >= MAX_PAGE_ADDR {
            return 0;
        }
        
        let next_page = current_page + PAGE_SIZE;
        
        if NEXT_PAGE.compare_exchange(
            current_page,
            next_page,
            Ordering::SeqCst,
            Ordering::SeqCst
        ).is_ok() {
            ALLOCATED_PAGES.fetch_add(1, Ordering::SeqCst);
            MEMORY_MANAGER.decrement_free_pages();
            return current_page;
        }
    }
}

#[no_mangle]
pub extern "C" fn rust_free_page(page: u32) {
    if page < MIN_PAGE_ADDR || page >= MAX_PAGE_ADDR {
        return;
    }
    
    if page % PAGE_SIZE != 0 {
        return;
    }
    
    let allocated = ALLOCATED_PAGES.load(Ordering::SeqCst);
    if allocated == 0 {
        return;
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

#[no_mangle]
pub extern "C" fn rust_is_valid_page(page: u32) -> bool {
    page >= MIN_PAGE_ADDR && 
    page < MAX_PAGE_ADDR && 
    page % PAGE_SIZE == 0
}

#[no_mangle]
pub extern "C" fn rust_pool_allocate() -> *mut u8 {
    unsafe {
        GLOBAL_MEMORY_POOL.allocate_block().unwrap_or(core::ptr::null_mut())
    }
}

#[no_mangle]
pub extern "C" fn rust_pool_free(ptr: *mut u8) -> bool {
    unsafe {
        GLOBAL_MEMORY_POOL.free_block(ptr)
    }
}

#[no_mangle]
pub extern "C" fn rust_pool_stats() {
    unsafe {
        print_str("  Pool allocated: ");
        print_u32(GLOBAL_MEMORY_POOL.get_allocated_count() as u32);
        print_str("\n  Pool free: ");
        print_u32(GLOBAL_MEMORY_POOL.get_free_count() as u32);
        print_str("\n");
    }
}

#[no_mangle]
pub extern "C" fn rust_process_create(priority: u8, name: *const u8) -> u32 {
    unsafe {
        if name.is_null() {
            return 0;
        }
        
        let name_len = utils::string_length(name);
        let name_slice = core::slice::from_raw_parts(name, name_len);
        
        GLOBAL_PROCESS_MANAGER
            .create_process(priority, name_slice)
            .unwrap_or(0)
    }
}

#[no_mangle]
pub extern "C" fn rust_process_terminate(pid: u32) -> bool {
    unsafe {
        GLOBAL_PROCESS_MANAGER.terminate_process(pid)
    }
}

#[no_mangle]
pub extern "C" fn rust_process_schedule() -> u32 {
    unsafe {
        GLOBAL_PROCESS_MANAGER.schedule_next().unwrap_or(0)
    }
}

#[no_mangle]
pub extern "C" fn rust_ipc_send(
    msg_type: u8,
    sender_pid: u32,
    receiver_pid: u32,
    data: *const u8,
    data_len: usize
) -> bool {
    unsafe {
        if data.is_null() {
            return false;
        }
        
        let data_slice = core::slice::from_raw_parts(data, data_len);
        let msg_type_enum = match msg_type {
            1 => ipc::MessageType::Data,
            2 => ipc::MessageType::Signal,
            _ => ipc::MessageType::Empty,
        };
        
        GLOBAL_MESSAGE_QUEUE.send_message(msg_type_enum, sender_pid, receiver_pid, data_slice)
    }
}

#[no_mangle]
pub extern "C" fn rust_ipc_has_message(receiver_pid: u32) -> bool {
    unsafe {
        GLOBAL_MESSAGE_QUEUE.has_message_for(receiver_pid)
    }
}

