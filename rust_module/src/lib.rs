#![no_std]
#![allow(internal_features)]
#![feature(lang_items)]

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

const PAGE_SIZE: u32           = 4096;
const PAGE_POOL_COUNT: u32     = 1024;
const PAGE_POOL_START: u32     = 0x0080_0000;
const PAGE_POOL_END: u32       = PAGE_POOL_START + PAGE_POOL_COUNT * PAGE_SIZE;
const FREE_LIST_CAPACITY: usize = PAGE_POOL_COUNT as usize;

struct PageFreeList {
    pages: UnsafeCell<[u32; FREE_LIST_CAPACITY]>,
    top:   UnsafeCell<usize>,
}

unsafe impl Sync for PageFreeList {}

impl PageFreeList {
    const fn new() -> Self {
        Self {
            pages: UnsafeCell::new([0u32; FREE_LIST_CAPACITY]),
            top:   UnsafeCell::new(0),
        }
    }

    fn push(&self, page_addr: u32) -> bool {
        unsafe {
            let top = self.top.get();
            if *top >= FREE_LIST_CAPACITY {
                return false;
            }
            (*self.pages.get())[*top] = page_addr;
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

static PAGE_FREE_LIST: PageFreeList = PageFreeList::new();
static ALLOCATED_PAGE_COUNT: AtomicU32 = AtomicU32::new(0);
static NEXT_FREE_PAGE: AtomicU32 = AtomicU32::new(PAGE_POOL_START);

struct PageAllocStats {
    free_count: UnsafeCell<u32>,
}

unsafe impl Sync for PageAllocStats {}

impl PageAllocStats {
    const fn new() -> Self {
        Self { free_count: UnsafeCell::new(PAGE_POOL_COUNT) }
    }

    fn free_count(&self) -> u32 {
        unsafe { *self.free_count.get() }
    }

    fn decrement(&self) {
        unsafe {
            let count = self.free_count.get();
            if *count > 0 { *count -= 1; }
        }
    }

    fn increment(&self) {
        unsafe {
            let count = self.free_count.get();
            if *count < PAGE_POOL_COUNT { *count += 1; }
        }
    }

    fn reset(&self) {
        unsafe { *self.free_count.get() = PAGE_POOL_COUNT; }
    }
}

static PAGE_STATS: PageAllocStats = PageAllocStats::new();
static mut GLOBAL_MEMORY_POOL: MemoryPool = MemoryPool::new();
static mut GLOBAL_PROCESS_MANAGER: ProcessManager = ProcessManager::new();
static mut GLOBAL_MESSAGE_QUEUE: MessageQueue = MessageQueue::new();

#[no_mangle]
pub extern "C" fn rust_memory_init() {
    PAGE_STATS.reset();
    ALLOCATED_PAGE_COUNT.store(0, Ordering::Relaxed);
    NEXT_FREE_PAGE.store(PAGE_POOL_START, Ordering::Relaxed);
    PAGE_FREE_LIST.reset();
}

#[no_mangle]
pub extern "C" fn rust_allocate_page() -> u32 {
    if ALLOCATED_PAGE_COUNT.load(Ordering::Relaxed) >= PAGE_POOL_COUNT {
        return 0;
    }

    if let Some(page_addr) = PAGE_FREE_LIST.pop() {
        ALLOCATED_PAGE_COUNT.fetch_add(1, Ordering::Relaxed);
        PAGE_STATS.decrement();
        return page_addr;
    }

    loop {
        let current = NEXT_FREE_PAGE.load(Ordering::Relaxed);
        if current >= PAGE_POOL_END {
            return 0;
        }
        let next = current + PAGE_SIZE;
        if NEXT_FREE_PAGE.compare_exchange(current, next, Ordering::Relaxed, Ordering::Relaxed).is_ok() {
            ALLOCATED_PAGE_COUNT.fetch_add(1, Ordering::Relaxed);
            PAGE_STATS.decrement();
            return current;
        }
    }
}

#[no_mangle]
pub extern "C" fn rust_free_page(page_addr: u32) {
    if page_addr < PAGE_POOL_START || page_addr >= PAGE_POOL_END {
        return;
    }
    if page_addr % PAGE_SIZE != 0 {
        return;
    }
    if ALLOCATED_PAGE_COUNT.load(Ordering::Relaxed) == 0 {
        return;
    }
    PAGE_FREE_LIST.push(page_addr);
    ALLOCATED_PAGE_COUNT.fetch_sub(1, Ordering::Relaxed);
    PAGE_STATS.increment();
}

extern "C" {
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
    let allocated = ALLOCATED_PAGE_COUNT.load(Ordering::Relaxed);
    let free      = PAGE_STATS.free_count();

    print_str("  Total pages: "); print_u32(PAGE_POOL_COUNT);
    print_str("\n  Allocated: ");  print_u32(allocated);
    print_str("\n  Free: ");       print_u32(free);
    print_str("\n");
}

#[no_mangle]
pub extern "C" fn rust_get_total_memory() -> u32 {
    PAGE_POOL_COUNT * PAGE_SIZE
}

#[no_mangle]
pub extern "C" fn rust_get_free_memory() -> u32 {
    PAGE_STATS.free_count() * PAGE_SIZE
}

#[no_mangle]
pub extern "C" fn rust_get_allocated_memory() -> u32 {
    ALLOCATED_PAGE_COUNT.load(Ordering::Relaxed) * PAGE_SIZE
}

#[no_mangle]
pub extern "C" fn rust_is_valid_page(page_addr: u32) -> bool {
    page_addr >= PAGE_POOL_START
        && page_addr < PAGE_POOL_END
        && page_addr % PAGE_SIZE == 0
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
        print_str("  Pool allocated: "); print_u32(pool.get_allocated_count() as u32);
        print_str("\n  Pool free: ");     print_u32(pool.get_free_count() as u32);
        print_str("\n");
    }
}

#[no_mangle]
pub extern "C" fn rust_process_create(priority: u8, name: *const u8) -> u32 {
    unsafe {
        if name.is_null() { return 0; }
        let name_len   = utils::string_length(name);
        let name_slice = core::slice::from_raw_parts(name, name_len);
        let manager    = &mut *core::ptr::addr_of_mut!(GLOBAL_PROCESS_MANAGER);
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
    data_len: usize,
) -> bool {
    unsafe {
        if data.is_null() { return false; }
        let data_slice    = core::slice::from_raw_parts(data, data_len);
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
