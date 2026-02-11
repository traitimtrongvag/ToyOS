const MAX_TASKS: usize = 32;
const DEFAULT_TIME_SLICE: u32 = 10;

#[derive(Copy, Clone)]
pub struct Task {
    id: u32,
    state: TaskState,
    priority: u8,
    time_slice: u32,
    remaining_time: u32,
}

#[derive(Copy, Clone, PartialEq)]
#[repr(u8)]
pub enum TaskState {
    Unused = 0,
    Ready = 1,
    Running = 2,
    Blocked = 3,
}

pub struct RoundRobinScheduler {
    tasks: [Option<Task>; MAX_TASKS],
    current_task_idx: usize,
    task_count: usize,
    next_task_id: u32,
}

impl RoundRobinScheduler {
    pub const fn new() -> Self {
        Self {
            tasks: [None; MAX_TASKS],
            current_task_idx: 0,
            task_count: 0,
            next_task_id: 1,
        }
    }
    
    pub fn create_task(&mut self, priority: u8) -> Option<u32> {
        if self.task_count >= MAX_TASKS {
            return None;
        }
        
        for slot in self.tasks.iter_mut() {
            if slot.is_none() {
                let task_id = self.next_task_id;
                self.next_task_id = self.next_task_id.wrapping_add(1);
                
                *slot = Some(Task {
                    id: task_id,
                    state: TaskState::Ready,
                    priority,
                    time_slice: DEFAULT_TIME_SLICE,
                    remaining_time: DEFAULT_TIME_SLICE,
                });
                
                self.task_count += 1;
                return Some(task_id);
            }
        }
        
        None
    }
    
    pub fn terminate_task(&mut self, task_id: u32) -> bool {
        for (idx, slot) in self.tasks.iter_mut().enumerate() {
            if let Some(task) = slot {
                if task.id == task_id {
                    *slot = None;
                    self.task_count = self.task_count.saturating_sub(1);
                    if idx == self.current_task_idx {
                        self.current_task_idx = 0;
                    }
                    return true;
                }
            }
        }
        false
    }
    
    pub fn schedule_next(&mut self) -> Option<u32> {
        if self.task_count == 0 {
            return None;
        }
        
        let start_idx = self.current_task_idx;
        let mut attempts = 0;
        let mut found_idx = None;
        let mut found_id = None;
        
        loop {
            let idx = (start_idx + attempts) % MAX_TASKS;
            attempts += 1;
            
            if attempts > MAX_TASKS {
                break;
            }
            
            if let Some(task) = &self.tasks[idx] {
                if task.state == TaskState::Ready {
                    found_idx = Some(idx);
                    found_id = Some(task.id);
                    break;
                }
            }
        }
        
        if let (Some(idx), Some(id)) = (found_idx, found_id) {
            self.mark_current_as_ready();
            
            if let Some(task) = &mut self.tasks[idx] {
                task.state = TaskState::Running;
                task.remaining_time = task.time_slice;
                self.current_task_idx = idx;
            }
            
            return Some(id);
        }
        
        None
    }
    
    pub fn tick(&mut self) -> bool {
        if let Some(task) = &mut self.tasks[self.current_task_idx] {
            if task.state == TaskState::Running && task.remaining_time > 0 {
                task.remaining_time -= 1;
                return task.remaining_time == 0;
            }
        }
        false
    }
    
    pub fn block_task(&mut self, task_id: u32) -> bool {
        self.update_task_state(task_id, TaskState::Blocked)
    }
    
    pub fn unblock_task(&mut self, task_id: u32) -> bool {
        self.update_task_state(task_id, TaskState::Ready)
    }
    
    pub fn get_task_count(&self) -> usize {
        self.task_count
    }
    
    fn mark_current_as_ready(&mut self) {
        if let Some(task) = &mut self.tasks[self.current_task_idx] {
            if task.state == TaskState::Running {
                task.state = TaskState::Ready;
            }
        }
    }
    
    fn update_task_state(&mut self, task_id: u32, new_state: TaskState) -> bool {
        for slot in self.tasks.iter_mut() {
            if let Some(task) = slot {
                if task.id == task_id {
                    task.state = new_state;
                    return true;
                }
            }
        }
        false
    }
}

