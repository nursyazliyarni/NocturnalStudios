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
#include <mutex>            
#include <functional>
#include <vector>

class Logger {
public:
    // Backward-compatible API
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

    // New: log level + sinks (optional to use; no breaking changes)
    enum class Level { Info, Warning, Error };

    // Sink receives formatted line and level
    using Sink = std::function<void(const std::string& /*formatted*/, Level /*level*/)>;

    // Add a sink to receive every message (thread-safe)
    void addSink(Sink sink);

    // Configure in-memory message cap (defaults to 50)
    void setMaxMessages(std::size_t max);

private:
    // Preserve existing storage for UI compatibility
    std::deque<std::string> consoleLogs;

    // New internals
    mutable std::mutex mtx_;
    std::vector<Sink> sinks_;
    std::size_t maxMessages_ = 50;

    // Single write path
    void write(Level level, std::string_view message, bool alsoCrashFile = false);

    // Helpers
    static const char* levelTag(Level level);
    static std::string formatLine(Level level, std::string_view message, const std::string& time);
    static std::string nowString(); // thread-safe time formatting

    static void log_crash_to_file(std::string_view message);
    static void handle_crash_error(int signal);
};

#endif // LOGGER_H