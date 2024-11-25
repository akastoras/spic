#include <ctime>
#include <fstream>
#include <iostream>
#include "util.h"

void Logger::log(LogLevel level, const std::string& message)
{
	// Get current timestamp
	time_t now = time(0);
	tm* timeinfo = localtime(&now);
	char timestamp[20];
	strftime(timestamp, sizeof(timestamp),
				"%Y-%m-%d %H:%M:%S", timeinfo);

	// Create log entry
	std::ostringstream logEntry;
	logEntry << "[" << timestamp << "] "
				<< levelToString(level) << ": " << message
				<< std::endl;

	// Output to console
	std::cout << logEntry.str();
}

void Logger::log(LogLevel level, std::ostringstream& message)
{
	// Get current timestamp
	time_t now = time(0);
	tm* timeinfo = localtime(&now);
	char timestamp[20];
	strftime(timestamp, sizeof(timestamp),
				"%Y-%m-%d %H:%M:%S", timeinfo);

	std::string line;
	std::istringstream input(message.str());
	while (std::getline(input, line)) {
		// Create log entry
		std::ostringstream logEntry;
		logEntry << "[" << timestamp << "] "
					<< levelToString(level) 
					<< ": " << tabs << line
					<< std::endl;

		// Output to console
		std::cout << logEntry.str();
	}
}
