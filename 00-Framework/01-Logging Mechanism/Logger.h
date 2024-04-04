#ifndef LOGGER_H
#define LOGGER_H
#include <windows.h>
#include <fstream>
#include <string>
#include <stdlib.h> 

class Logger {
public:
    Logger(const std::string& filename) {
        std::string dirName = "Logs/";
        CreateDirectory(dirName.c_str(), NULL);
        std::string fullPath = dirName + filename;
        file.open(fullPath, std::ios::app); // append instead of overwrite
        if (!file.is_open()) {
            MessageBox(NULL, TEXT("Creation of log file failed. Exiting..."), TEXT("File I/O Error"), MB_OK);
            exit(0);
        }
    }

    ~Logger() {
        if (file.is_open()) {
            file.close();
        }
    }

    void debug(const std::string& message) {
        if (!file.is_open()) {
            throw std::runtime_error("Logger: File not open");
        }
        file << message << std::endl;
    }

    template <typename T>
    void debug(const std::string& message, const T& value) {
        if (!file.is_open()) {
            throw std::runtime_error("Logger: File not open");
        }
        file << message << ": " << value << std::endl;
    }

private:
    std::ofstream file;
};

#endif