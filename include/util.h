#pragma once

#include <ctime>
#include <fstream>
#include <iostream>
#include <sstream>

// Enum to represent log levels
enum LogLevel { DEBUG, INFO, WARNING, ERROR};
#define RESET "\033[0m"
#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define CYAN "\033[36m"

class Logger {
public:
	// Constructor: Opens the log file in append mode
	// Logger(const std::string& filename): tabs("")
	// {
	// 	logFile.open(filename, std::ios::app);
	// 	if (!logFile.is_open()) {
	// 		std::cerr << "Error opening log file." << std::endl;
	// 	}
	// }

	Logger (std::ostream& os) : logFile(os), tabs("") {}

	// Destructor: Closes the log file
	~Logger() {}//{ logFile.close(); }

	// Logs a message with a given log level
	void log(LogLevel level, const std::string& message);
	void log(LogLevel level, std::ostringstream& message);
	void increaseTabs() { tabs += "\t"; }
	void decreaseTabs() { tabs.pop_back(); }
 
private:
	std::ostream& logFile; // File stream for the log file
	std::string tabs; 
	// Converts log level to a string for output 
	std::string levelToString(LogLevel level)
	{
		switch (level) {
		case DEBUG:
			return GREEN "DEBUG" RESET;
		case INFO:
			return BLUE "INFO" RESET;
		case WARNING:
			return YELLOW "WARNING" RESET;
		case ERROR:
			return RED "ERROR" RESET;
		default:
			return "UNKNOWN";
		}
	}
};