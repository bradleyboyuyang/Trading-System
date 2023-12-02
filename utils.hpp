/**
 * utils.hpp
 * A collection of utility functions
 *
 * author Boyu Yang
 */

#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <vector>
#include <iomanip>
#include <iostream>
#include <chrono>

using namespace std;

// Function to get the system time in milliseconds and return it as a string
std::string getTime() {
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);

    // change the system time to local time
    std::tm now_tm = *std::localtime(&now_c);

    // use stringstream to format the time
    std::stringstream ss;
    ss << std::put_time(&now_tm, "%Y-%m-%d %H:%M:%S");

    // get the milliseconds separately
    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
    ss << '.' << std::setfill('0') << std::setw(3) << milliseconds.count();

    return ss.str();
}


enum class LogLevel {
    INFO,
    WARNING,
    ERROR
};

// Function to log messages
void log(LogLevel level, const std::string& message) {
    std::string curTime = getTime();

    std::string logLevelStr;
    switch (level) {
        case LogLevel::INFO:
            logLevelStr = "INFO";
            break;
        case LogLevel::WARNING:
            logLevelStr = "WARNING";
            break;
        case LogLevel::ERROR:
            logLevelStr = "ERROR";
            break;
    }

    std::cout << "[" << curTime << "] [" << logLevelStr << "] " << message << std::endl;
}







#endif
