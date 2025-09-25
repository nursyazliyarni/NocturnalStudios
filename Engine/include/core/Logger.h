#ifndef LOGGER_H
#define LOGGER_H

#include <iostream>
#include <string>
#include <deque>
#include <chrono>
#include <filesystem>
#include <sstream>
#include <fstream>

class Logger {
public:
	void error(std::string_view message);
	void log(std::string_view message);
	void warning(std::string_view message);

	std::deque<std::string> getLogs() const&;

	static std::string getCurrentTime();
	static std::string getUniqueTimeId();

	static constexpr char logFormat[] = "[%d-%m-%Y] [%H:%M:%S]";
	static constexpr char fileNameFormat[] = "%d%m%Y-%H%I%S-";
	static unsigned int logCounter;

private:
	std::deque<std::string> consoleLogs;
};

#endif // LOGGER_H