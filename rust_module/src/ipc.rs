const MAX_MESSAGES: usize = 64;
const MESSAGE_DATA_SIZE: usize = 128;

#[derive(Copy, Clone, PartialEq)]
#[repr(u8)]
pub enum MessageType {
    Empty = 0,
    Data = 1,
    Signal = 2,
}

#[derive(Copy, Clone)]
pub struct Message {
    msg_type: MessageType,
    sender_pid: u32,
    receiver_pid: u32,
    data_length: usize,
    data: [u8; MESSAGE_DATA_SIZE],
}

impl Message {
    const fn new() -> Self {
        Self {
            msg_type: MessageType::Empty,
            sender_pid: 0,
            receiver_pid: 0,
            data_length: 0,
            data: [0; MESSAGE_DATA_SIZE],
        }
    }
}

pub struct MessageQueue {
    messages: [Message; MAX_MESSAGES],
    head: usize,
    tail: usize,
    count: usize,
}

impl MessageQueue {
    pub const fn new() -> Self {
        Self {
            messages: [Message::new(); MAX_MESSAGES],
            head: 0,
            tail: 0,
            count: 0,
        }
    }
    
    pub fn send_message(
        &mut self,
        msg_type: MessageType,
        sender_pid: u32,
        receiver_pid: u32,
        data: &[u8]
    ) -> bool {
        if self.count >= MAX_MESSAGES {
            return false;
        }
        
        let data_length = data.len().min(MESSAGE_DATA_SIZE);
        
        self.messages[self.tail].msg_type = msg_type;
        self.messages[self.tail].sender_pid = sender_pid;
        self.messages[self.tail].receiver_pid = receiver_pid;
        self.messages[self.tail].data_length = data_length;
        self.messages[self.tail].data[..data_length].copy_from_slice(&data[..data_length]);
        
        self.tail = (self.tail + 1) % MAX_MESSAGES;
        self.count += 1;
        true
    }
    
    pub fn receive_message(&mut self, receiver_pid: u32) -> Option<Message> {
        for i in 0..self.count {
            let idx = (self.head + i) % MAX_MESSAGES;
            
            if self.messages[idx].receiver_pid == receiver_pid {
                let msg = self.messages[idx];
                
                for j in i..self.count - 1 {
                    let curr_idx = (self.head + j) % MAX_MESSAGES;
                    let next_idx = (self.head + j + 1) % MAX_MESSAGES;
                    self.messages[curr_idx] = self.messages[next_idx];
                }
                
                self.count -= 1;
                self.tail = if self.tail == 0 {
                    MAX_MESSAGES - 1
                } else {
                    self.tail - 1
                };
                
                return Some(msg);
            }
        }
        
        None
    }
    
    pub fn has_message_for(&self, receiver_pid: u32) -> bool {
        for i in 0..self.count {
            let idx = (self.head + i) % MAX_MESSAGES;
            
            if self.messages[idx].receiver_pid == receiver_pid {
                return true;
            }
        }
        false
    }
    
    pub fn get_count(&self) -> usize {
        self.count
    }
}
