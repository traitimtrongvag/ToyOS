#![no_std]

const MAX_PROCESSES: usize = 32;
const PROCESS_NAME_LEN: usize = 16;

#[derive(Copy, Clone, PartialEq)]
#[repr(u8)]
pub enum ProcessState {
    Unused = 0,
    Ready = 1,
    Running = 2,
    Blocked = 3,
}

#[derive(Copy, Clone)]
pub struct ProcessInfo {
    pid: u32,
    state: ProcessState,
    priority: u8,
    name: [u8; PROCESS_NAME_LEN],
}

impl ProcessInfo {
    const fn new() -> Self {
        Self {
            pid: 0,
            state: ProcessState::Unused,
            priority: 0,
            name: [0; PROCESS_NAME_LEN],
        }
    }
}

pub struct ProcessManager {
    processes: [ProcessInfo; MAX_PROCESSES],
    next_pid: u32,
    active_count: usize,
}

impl ProcessManager {
    pub const fn new() -> Self {
        Self {
            processes: [ProcessInfo::new(); MAX_PROCESSES],
            next_pid: 1,
            active_count: 0,
        }
    }
    
    pub fn create_process(&mut self, priority: u8, name: &[u8]) -> Option<u32> {
        if self.active_count >= MAX_PROCESSES {
            return None;
        }
        
        for process in self.processes.iter_mut() {
            if process.state == ProcessState::Unused {
                let pid = self.next_pid;
                self.next_pid += 1;
                
                process.pid = pid;
                process.state = ProcessState::Ready;
                process.priority = priority;
                
                let copy_len = name.len().min(PROCESS_NAME_LEN);
                process.name[..copy_len].copy_from_slice(&name[..copy_len]);
                
                self.active_count += 1;
                return Some(pid);
            }
        }
        
        None
    }
    
    pub fn terminate_process(&mut self, pid: u32) -> bool {
        for process in self.processes.iter_mut() {
            if process.pid == pid && process.state != ProcessState::Unused {
                process.state = ProcessState::Unused;
                self.active_count -= 1;
                return true;
            }
        }
        false
    }
    
    pub fn schedule_next(&mut self) -> Option<u32> {
        let mut best_idx: Option<usize> = None;
        let mut highest_priority = 0u8;
        
        for (idx, process) in self.processes.iter().enumerate() {
            if process.state == ProcessState::Ready && process.priority > highest_priority {
                highest_priority = process.priority;
                best_idx = Some(idx);
            }
        }
        
        if let Some(idx) = best_idx {
            for p in self.processes.iter_mut() {
                if p.state == ProcessState::Running {
                    p.state = ProcessState::Ready;
                }
            }
            
            self.processes[idx].state = ProcessState::Running;
            return Some(self.processes[idx].pid);
        }
        
        None
    }
    
    pub fn get_active_count(&self) -> usize {
        self.active_count
    }
}
