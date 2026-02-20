extern "C" {
    void terminal_writestring(const char* s);
    void terminal_putchar(char c);
    void terminal_setcolor(unsigned char color);
}

inline void* operator new(unsigned int, void* p) { return p; }

enum LogLevel {
    LOG_INFO = 0,
    LOG_WARNING = 1,
    LOG_ERROR = 2,
    LOG_DEBUG = 3
};

class Logger {
private:
    static const unsigned char COLOR_INFO = 0x0B;
    static const unsigned char COLOR_WARNING = 0x0E;
    static const unsigned char COLOR_ERROR = 0x0C;
    static const unsigned char COLOR_DEBUG = 0x08;
    
    const char* module_name;
    bool enabled;
    
    void print_prefix(LogLevel level) {
        terminal_putchar('[');
        
        switch(level) {
            case LOG_INFO:
                terminal_setcolor(COLOR_INFO);
                terminal_writestring("INFO");
                break;
            case LOG_WARNING:
                terminal_setcolor(COLOR_WARNING);
                terminal_writestring("WARN");
                break;
            case LOG_ERROR:
                terminal_setcolor(COLOR_ERROR);
                terminal_writestring("ERR ");
                break;
            case LOG_DEBUG:
                terminal_setcolor(COLOR_DEBUG);
                terminal_writestring("DBG ");
                break;
        }
        
        terminal_setcolor(0x07);
        terminal_writestring("] ");
        terminal_writestring(module_name);
        terminal_writestring(": ");
    }
    
public:
    Logger(const char* name) : module_name(name), enabled(true) {}
    
    void info(const char* message) {
        if(!enabled) return;
        print_prefix(LOG_INFO);
        terminal_writestring(message);
        terminal_putchar('\n');
    }
    
    void warning(const char* message) {
        if(!enabled) return;
        print_prefix(LOG_WARNING);
        terminal_writestring(message);
        terminal_putchar('\n');
    }
    
    void error(const char* message) {
        if(!enabled) return;
        print_prefix(LOG_ERROR);
        terminal_writestring(message);
        terminal_putchar('\n');
    }
    
    void debug(const char* message) {
        if(!enabled) return;
        print_prefix(LOG_DEBUG);
        terminal_writestring(message);
        terminal_putchar('\n');
    }
    
    void set_enabled(bool enable) {
        enabled = enable;
    }
};

static char logger_buf[sizeof(Logger)];
static Logger* system_logger = nullptr;

extern "C" void cpp_logger_init() {
    system_logger = new (logger_buf) Logger("System");
}

extern "C" void cpp_log_info(const char* msg) {
    if(system_logger) {
        system_logger->info(msg);
    }
}

extern "C" void cpp_log_warning(const char* msg) {
    if(system_logger) {
        system_logger->warning(msg);
    }
}

extern "C" void cpp_log_error(const char* msg) {
    if(system_logger) {
        system_logger->error(msg);
    }
}
