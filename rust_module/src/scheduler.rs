#![no_std]

pub struct Task {
    id: u32,
    state: TaskState,
    priority: u8,
}

#[derive(Copy, Clone, PartialEq)]
pub enum TaskState {
    Ready,
    Running,
    Blocked,
    Terminated,
}

pub struct SimpleScheduler {
    tasks: [Option<Task>; 16],
    current_task: usize,
    task_count: usize,
}

impl SimpleScheduler {
    pub const fn new() -> Self {
        SimpleScheduler {
            tasks: [None; 16],
            current_task: 0,
            task_count: 0,
        }
    }
    
    pub fn add_task(&mut self, id: u32, priority: u8) -> bool {
        if self.task_count >= 16 {
            return false;
        }
        
        self.tasks[self.task_count] = Some(Task {
            id,
            state: TaskState::Ready,
            priority,
        });
        
        self.task_count += 1;
        true
    }
    
    pub fn schedule(&mut self) -> Option<u32> {
        if self.task_count == 0 {
            return None;
        }
        
        let mut next_task = None;
        let mut highest_priority = 0u8;
        
        for (idx, task_opt) in self.tasks.iter().enumerate() {
            if let Some(task) = task_opt {
                if task.state == TaskState::Ready && task.priority >= highest_priority {
                    highest_priority = task.priority;
                    next_task = Some(idx);
                }
            }
        }
        
        if let Some(idx) = next_task {
            self.current_task = idx;
            if let Some(ref mut task) = self.tasks[idx] {
                task.state = TaskState::Running;
                return Some(task.id);
            }
        }
        
        None
    }
    
    pub fn block_current(&mut self) {
        if let Some(ref mut task) = self.tasks[self.current_task] {
            task.state = TaskState::Blocked;
        }
    }
    
    pub fn unblock_task(&mut self, task_id: u32) {
        for task_opt in self.tasks.iter_mut() {
            if let Some(ref mut task) = task_opt {
                if task.id == task_id {
                    task.state = TaskState::Ready;
                    break;
                }
            }
        }
    }
}
