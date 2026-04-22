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
pub mod vfs;
pub mod memfs;

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
const MIN_PAGE_ADDR: u32 = 0x800000;
const MAX_PAGE_ADDR: u32 = MIN_PAGE_ADDR + (TOTAL_PAGES * PAGE_SIZE);

// Free-list holds page addresses returned by rust_free_page so they can be
// handed out again before bumping NEXT_PAGE further.
const FREE_LIST_SIZE: usize = TOTAL_PAGES as usize;

struct FreeList {
    pages: UnsafeCell<[u32; FREE_LIST_SIZE]>,
    top:   UnsafeCell<usize>,
}

unsafe impl Sync for FreeList {}

impl FreeList {
    const fn new() -> Self {
        Self {
            pages: UnsafeCell::new([0u32; FREE_LIST_SIZE]),
            top:   UnsafeCell::new(0),
        }
    }

    fn push(&self, page: u32) -> bool {
        unsafe {
            let top = self.top.get();
            if *top >= FREE_LIST_SIZE {
                return false;
            }
            (*self.pages.get())[*top] = page;
            *top += 1;
            true
        }
    }

    fn pop(&self) -> Option<u32> {
        unsafe {
            let top = self.top.get();
            if *top == 0 {
                return None;
            }
            *top -= 1;
            Some((*self.pages.get())[*top])
        }
    }

    fn reset(&self) {
        unsafe { *self.top.get() = 0; }
    }
}

static FREE_LIST: FreeList = FreeList::new();

static ALLOCATED_PAGES: AtomicU32 = AtomicU32::new(0);
static NEXT_PAGE: AtomicU32 = AtomicU32::new(MIN_PAGE_ADDR);

struct MemoryManager {
    free_pages: UnsafeCell<u32>,
}

unsafe impl Sync for MemoryManager {}

impl MemoryManager {
    const fn new() -> Self {
        Self {
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
    ALLOCATED_PAGES.store(0, Ordering::Relaxed);
    NEXT_PAGE.store(MIN_PAGE_ADDR, Ordering::Relaxed);
    FREE_LIST.reset();
}

#[no_mangle]
pub extern "C" fn rust_allocate_page() -> u32 {
    let allocated = ALLOCATED_PAGES.load(Ordering::Relaxed);
    if allocated >= TOTAL_PAGES {
        return 0;
    }

    // Reuse a previously freed page before consuming fresh address space.
    if let Some(page) = FREE_LIST.pop() {
        ALLOCATED_PAGES.fetch_add(1, Ordering::Relaxed);
        MEMORY_MANAGER.decrement_free_pages();
        return page;
    }

    loop {
        let current_page = NEXT_PAGE.load(Ordering::Relaxed);

        if current_page >= MAX_PAGE_ADDR {
            return 0;
        }

        let next_page = current_page + PAGE_SIZE;

        if NEXT_PAGE.compare_exchange(
            current_page,
            next_page,
            Ordering::Relaxed,
            Ordering::Relaxed
        ).is_ok() {
            ALLOCATED_PAGES.fetch_add(1, Ordering::Relaxed);
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

    let allocated = ALLOCATED_PAGES.load(Ordering::Relaxed);
    if allocated == 0 {
        return;
    }

    // Return the page to the free-list so rust_allocate_page can hand it out again.
    // If the free-list is somehow full (should not happen given FREE_LIST_SIZE == TOTAL_PAGES),
    // the page is lost — acceptable for this allocator's scope.
    FREE_LIST.push(page);
    ALLOCATED_PAGES.fetch_sub(1, Ordering::Relaxed);
    MEMORY_MANAGER.increment_free_pages();
}

extern "C" {
    fn terminal_writestring(s: *const u8);
    fn terminal_putchar(c: u8);
}

fn print_str(s: &str) {
    for b in s.bytes() {
        unsafe { terminal_putchar(b); }
    }
}

fn print_u32(num: u32) {
    if num == 0 {
        unsafe { terminal_putchar(b'0'); }
        return;
    }
    let powers: [u32; 10] = [
        1_000_000_000, 100_000_000, 10_000_000, 1_000_000,
        100_000, 10_000, 1_000, 100, 10, 1,
    ];
    let mut started = false;
    let mut n = num;
    for &p in powers.iter() {
        if n >= p {
            let mut d = 0u8;
            while n >= p { n -= p; d += 1; }
            unsafe { terminal_putchar(b'0' + d); }
            started = true;
        } else if started {
            unsafe { terminal_putchar(b'0'); }
        }
    }
}

#[no_mangle]
pub extern "C" fn rust_print_stats() {
    let allocated = ALLOCATED_PAGES.load(Ordering::Relaxed);
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
    ALLOCATED_PAGES.load(Ordering::Relaxed) * PAGE_SIZE
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
        let pool = &mut *core::ptr::addr_of_mut!(GLOBAL_MEMORY_POOL);
        pool.allocate_block().unwrap_or(core::ptr::null_mut())
    }
}

#[no_mangle]
pub extern "C" fn rust_pool_free(ptr: *mut u8) -> bool {
    unsafe {
        let pool = &mut *core::ptr::addr_of_mut!(GLOBAL_MEMORY_POOL);
        pool.free_block(ptr)
    }
}

#[no_mangle]
pub extern "C" fn rust_pool_stats() {
    unsafe {
        let pool = &*core::ptr::addr_of!(GLOBAL_MEMORY_POOL);
        print_str("  Pool allocated: ");
        print_u32(pool.get_allocated_count() as u32);
        print_str("\n  Pool free: ");
        print_u32(pool.get_free_count() as u32);
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
        
        let manager = &mut *core::ptr::addr_of_mut!(GLOBAL_PROCESS_MANAGER);
        manager.create_process(priority, name_slice).unwrap_or(0)
    }
}

#[no_mangle]
pub extern "C" fn rust_process_terminate(pid: u32) -> bool {
    unsafe {
        let manager = &mut *core::ptr::addr_of_mut!(GLOBAL_PROCESS_MANAGER);
        manager.terminate_process(pid)
    }
}

#[no_mangle]
pub extern "C" fn rust_process_schedule() -> u32 {
    unsafe {
        let manager = &mut *core::ptr::addr_of_mut!(GLOBAL_PROCESS_MANAGER);
        manager.schedule_next().unwrap_or(0)
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
        
        let queue = &mut *core::ptr::addr_of_mut!(GLOBAL_MESSAGE_QUEUE);
        queue.send_message(msg_type_enum, sender_pid, receiver_pid, data_slice)
    }
}

#[no_mangle]
pub extern "C" fn rust_ipc_has_message(receiver_pid: u32) -> bool {
    unsafe {
        let queue = &*core::ptr::addr_of!(GLOBAL_MESSAGE_QUEUE);
        queue.has_message_for(receiver_pid)
    }
}

