/*!------------------------------------------------------------------------------------------------
	File: Logger.h
	Project: NocturnalStudio

	This file contains the declaration of a Logger class for logging errors, warnings and general
	messages to the console. It also maintains a history of recent log messages.

	Primary Author:
		Lau Xiang Yang (100%)
		- Created the Logger class template and its specializations.

	All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#ifndef LOGGER_H
#define LOGGER_H

#include <iostream>
#include <string>
#include <deque>
#include <chrono>
#include <filesystem>
#include <sstream>
#include <fstream>
#include <csignal>

class Logger {
public:
	void error(std::string_view message);
	void log(std::string_view message);
	void warning(std::string_view message);

	static void setup_crash_handler();
	std::deque<std::string> getLogs() const&;

	static std::string getCurrentTime();
	static std::string getUniqueTimeId();

	static constexpr char logFormat[] = "[%d-%m-%Y] [%H:%M:%S]";
	static constexpr char fileNameFormat[] = "%d%m%Y-%H%I%S-";
	static unsigned int logCounter;

private:
	std::deque<std::string> consoleLogs;

	static void log_crash_to_file(std::string_view message);
	static void handle_crash_error(int signal);
};

#endif // LOGGER_H