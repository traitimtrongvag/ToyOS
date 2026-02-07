extern "C" {
    void terminal_writestring(const char* s);
    void terminal_putchar(char c);
}

class SimpleVector {
private:
    int* data;
    unsigned int size;
    unsigned int capacity;
    
    void expand() {
        unsigned int new_capacity = capacity * 2;
        int* new_data = new int[new_capacity];
        
        for (unsigned int i = 0; i < size; i++) {
            new_data[i] = data[i];
        }
        
        delete[] data;
        data = new_data;
        capacity = new_capacity;
    }
    
public:
    SimpleVector() : data(nullptr), size(0), capacity(4) {
        data = new int[capacity];
    }
    
    ~SimpleVector() {
        delete[] data;
    }
    
    void push_back(int value) {
        if (size >= capacity) {
            expand();
        }
        data[size++] = value;
    }
    
    int get(unsigned int index) const {
        if (index < size) {
            return data[index];
        }
        return -1;
    }
    
    unsigned int get_size() const {
        return size;
    }
};

class Driver {
private:
    const char* name;
    bool initialized;
    SimpleVector* test_data;
    
public:
    Driver(const char* driver_name) : name(driver_name), initialized(false) {
        test_data = new SimpleVector();
    }
    
    ~Driver() {
        delete test_data;
    }
    
    void init() {
        initialized = true;
        terminal_writestring("  Driver '");
        terminal_writestring(name);
        terminal_writestring("' initialized\n");
    }
    
    void run_test() {
        if (!initialized) {
            terminal_writestring("  Driver not initialized!\n");
            return;
        }
        
        terminal_writestring("  Populating test data...\n");
        for (int i = 0; i < 5; i++) {
            test_data->push_back(i * 10);
        }
        
        terminal_writestring("  Test data: ");
        for (unsigned int i = 0; i < test_data->get_size(); i++) {
            print_number(test_data->get(i));
            if (i < test_data->get_size() - 1) {
                terminal_writestring(", ");
            }
        }
        terminal_writestring("\n");
    }
    
private:
    void print_number(int num) {
        char buffer[12];
        int i = 0;
        bool negative = false;
        
        if (num < 0) {
            negative = true;
            num = -num;
        }
        
        if (num == 0) {
            terminal_putchar('0');
            return;
        }
        
        while (num > 0) {
            buffer[i++] = '0' + (num % 10);
            num /= 10;
        }
        
        if (negative) {
            terminal_putchar('-');
        }
        
        while (i > 0) {
            terminal_putchar(buffer[--i]);
        }
    }
};

static Driver* global_driver = nullptr;

extern "C" void cpp_driver_init() {
    global_driver = new Driver("VirtualDevice");
    global_driver->init();
}

extern "C" void cpp_driver_test() {
    if (global_driver) {
        global_driver->run_test();
    }
}

void* operator new(unsigned long size) {
    static char heap[8192];
    static unsigned long heap_offset = 0;
    
    if (heap_offset + size > sizeof(heap)) {
        return nullptr;
    }
    
    void* ptr = &heap[heap_offset];
    heap_offset += size;
    
    return ptr;
}

void* operator new[](unsigned long size) {
    return operator new(size);
}

void operator delete(void*) noexcept {
}

void operator delete[](void*) noexcept {
}

void operator delete(void*, unsigned long) noexcept {
}

void operator delete[](void*, unsigned long) noexcept {
}
