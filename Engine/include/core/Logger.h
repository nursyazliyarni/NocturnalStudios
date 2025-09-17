#ifndef LOGGER_H
#define LOGGER_H

#include <iostream>
#include <string>
#include <deque>

class Logger {
public:
	void error(std::string message);
	void log(std::string message);
	void warning(std::string message);

	std::deque<std::string> getLogs() const;

	static std::string getCurrentTime();
	static std::string getUniqueTimeId();

	static constexpr char logFormat[] = "[%d-%m-%Y] [%H:%M:%S]";
	static constexpr char fileNameFormat[] = "%d%m%Y-%H%I%S-";
	static unsigned int logCounter;

private:
	std::deque<std::string> consoleLogs;
};

#endif // LOGGER_H