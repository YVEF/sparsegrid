#include "sg_log.h"


void sg_log(const char* log, LogLevel level) {
    switch (level) {
    	case LogLevel::Error: std::cout << FRED(log); break;
    	case LogLevel::Warn: std::cout << FYEL(log); break;
    	case LogLevel::Info: std::cout << FGRN(log); break;
    	default: std::cout << FWHT(log); break;
    }
    std::cout << '\n';
}

