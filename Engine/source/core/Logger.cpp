/*!------------------------------------------------------------------------------------------------
	File: Logger.h
	Project: NocturnalStudio

	This file contains the implementation of a Logger class for logging errors, warnings
	and general messages to the console.

	Primary Author:
		Lau Xiang Yang (100%)
		- Created the Logger class template and its specializations.

	All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#include "../include/core/Logger.h"

unsigned int Logger::logCounter = 0;
constexpr std::size_t MAX_LOG_MESSAGE = 50;

void Logger::error(std::string_view message) {
	std::cerr << getCurrentTime() << " [ERROR] " << message << std::endl;
	std::string formatError = getCurrentTime() + " [ERROR] " + message.data() + '\n';
	consoleLogs.push_back(std::move(formatError));

	while (consoleLogs.size() > MAX_LOG_MESSAGE) {
		consoleLogs.pop_front();
	}
}

void Logger::log(std::string_view message) {
	std::cout << getCurrentTime() << " [LOG] " << message << std::endl;
	std::string formatLog = getCurrentTime() + " [LOG] " + message.data() + '\n';
	consoleLogs.push_back(std::move(formatLog));

	while (consoleLogs.size() > MAX_LOG_MESSAGE) {
		consoleLogs.pop_front();
	}
}

void Logger::warning(std::string_view message) {
	std::cout << getCurrentTime() << " [WARNING] " << message << std::endl;
	std::string formatWarning = getCurrentTime() + " [WARNING] " + message.data() + '\n';

	consoleLogs.push_back(std::move(formatWarning));
	while (consoleLogs.size() > MAX_LOG_MESSAGE) {
		consoleLogs.pop_front();
	}
}

std::deque<std::string> Logger::getLogs() const& {
	return consoleLogs;
}

std::string Logger::getCurrentTime() {
	std::stringstream ss;
	auto now = std::chrono::system_clock::now();
	auto time = std::chrono::system_clock::to_time_t(now);

#pragma warning(suppress : 4996)
	ss << std::put_time(std::localtime(&time), logFormat);
#pragma warning(default : 4996)
	return ss.str();
}

std::string Logger::getUniqueTimeId() {
	std::stringstream ss;
	auto now = std::chrono::system_clock::now();
	auto time = std::chrono::system_clock::to_time_t(now);

#pragma warning(suppress : 4996)
	ss << std::put_time(std::localtime(&time), fileNameFormat);
#pragma warning(default : 4996)
	return ss.str() + std::to_string(logCounter++);
}

