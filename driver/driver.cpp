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
        unsigned int new_capacity = capacity == 0 ? 4 : capacity * 2;
        int* new_data = new int[new_capacity];
        
        if (new_data == nullptr) {
            return; // Allocation failed, keep old data
        }
        
        for (unsigned int i = 0; i < size; i++) {
            new_data[i] = data[i];
        }
        
        delete[] data;
        data = new_data;
        capacity = new_capacity;
    }
    
public:
    SimpleVector() : data(nullptr), size(0), capacity(0) {
        capacity = 4;
        data = new int[capacity];
    }
    
    // Copy constructor to prevent shallow copies
    SimpleVector(const SimpleVector& other) : data(nullptr), size(0), capacity(0) {
        capacity = other.capacity;
        size = other.size;
        data = new int[capacity];
        
        if (data != nullptr) {
            for (unsigned int i = 0; i < size; i++) {
                data[i] = other.data[i];
            }
        }
    }
    
    // Assignment operator
    SimpleVector& operator=(const SimpleVector& other) {
        if (this != &other) {
            delete[] data;
            
            capacity = other.capacity;
            size = other.size;
            data = new int[capacity];
            
            if (data != nullptr) {
                for (unsigned int i = 0; i < size; i++) {
                    data[i] = other.data[i];
                }
            }
        }
        return *this;
    }
    
    ~SimpleVector() {
        delete[] data;
    }
    
    bool push_back(int value) {
        if (size >= capacity) {
            expand();
            if (size >= capacity) {
                return false; // Expansion failed
            }
        }
        data[size++] = value;
        return true;
    }
    
    int get(unsigned int index) const {
        if (index >= size) {
            return -1; // Out of bounds
        }
        return data[index];
    }
    
    unsigned int get_size() const {
        return size;
    }
    
    bool is_valid() const {
        return data != nullptr;
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
        if (test_data == nullptr || !test_data->is_valid()) {
            terminal_writestring("  Driver initialization failed: memory allocation error\n");
            return;
        }
        
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
        
        if (test_data == nullptr) {
            terminal_writestring("  Test data unavailable!\n");
            return;
        }
        
        terminal_writestring("  Populating test data...\n");
        for (int i = 0; i < 5; i++) {
            if (!test_data->push_back(i * 10)) {
                terminal_writestring("  Warning: failed to add test data\n");
                break;
            }
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
        
        if (num == 0) {
            terminal_putchar('0');
            return;
        }
        
        if (num < 0) {
            negative = true;
            if (num == -2147483648) {
                terminal_writestring("-2147483648");
                return;
            }
            num = -num;
        }
        
        while (num > 0 && i < 11) {
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
    if (global_driver != nullptr) {
        global_driver->init();
    }
}

extern "C" void cpp_driver_test() {
    if (global_driver != nullptr) {
        global_driver->run_test();
    }
}

namespace {
    constexpr __SIZE_TYPE__ HEAP_SIZE = 8192;
    constexpr __SIZE_TYPE__ ALIGNMENT = 8;
    
    char heap[HEAP_SIZE];
    __SIZE_TYPE__ heap_offset = 0;
    
    __SIZE_TYPE__ align_size(__SIZE_TYPE__ size) {
        return (size + ALIGNMENT - 1) & ~(ALIGNMENT - 1);
    }
}

void* operator new(__SIZE_TYPE__ size) noexcept {
    if (size == 0) {
        size = 1;
    }
    
    __SIZE_TYPE__ aligned_size = align_size(size);
    
    if (heap_offset + aligned_size > HEAP_SIZE) {
        return nullptr;
    }
    
    void* ptr = &heap[heap_offset];
    heap_offset += aligned_size;
    
    return ptr;
}

void* operator new[](__SIZE_TYPE__ size) noexcept {
    return operator new(size);
}

void operator delete(void*) noexcept {
}

void operator delete[](void*) noexcept {
}

void operator delete(void*, __SIZE_TYPE__) noexcept {
}

void operator delete[](void*, __SIZE_TYPE__) noexcept {
}
