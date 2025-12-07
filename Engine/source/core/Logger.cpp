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

void Logger::log_crash_to_file(std::string_view message) {
	std::string_view filename_path{ "crash_message/crash_message_log.txt" };
	std::filesystem::path message_directory = std::filesystem::path(filename_path).parent_path();
	// Create directory if it doesn't exist
	if (!message_directory.empty() && !std::filesystem::exists(message_directory)) {
		std::filesystem::create_directories(message_directory);
	}

	std::ofstream log(filename_path.data(), std::ios::app);
	log << getCurrentTime() << " [CRASH_MESSAGE] " << message << std::endl;
	log.flush();
}

void Logger::setup_crash_handler() {
	std::signal(SIGSEGV, handle_crash_error);
	std::signal(SIGFPE, handle_crash_error);
	std::signal(SIGILL, handle_crash_error);
	std::signal(SIGABRT, handle_crash_error);
}

void Logger::handle_crash_error(int signal) {
	switch (signal) {
	case SIGSEGV: 
		log_crash_to_file("Program crash due to segmentation fault - Invalid memory access"); 
		break;
	case SIGFPE:  
		log_crash_to_file("Program crash due to floating point exception - Divide by zero or overflow"); 
		break;
	case SIGILL:  
		log_crash_to_file("Program crash due to illegal instruction - Invalid CPU instruction"); 
		break;
	case SIGABRT: 
		log_crash_to_file("Program crash due to abort signal - std::abort() or failed assert()"); 
		break;
	default:      
		log_crash_to_file("Program crash due to other signals"); 
		break;
	}
}

// New: safe level tag
const char* Logger::levelTag(Level level) {
	switch (level) {
	case Level::Error:   return "[ERROR]";
	case Level::Warning: return "[WARNING]";
	default:             return "[LOG]";
	}
}

// New: thread-safe current time as string
std::string Logger::nowString() {
	std::stringstream ss;
	auto now = std::chrono::system_clock::now();
	std::time_t t = std::chrono::system_clock::to_time_t(now);
	std::tm tm{};
#if defined(_WIN32)
	localtime_s(&tm, &t);
#else
	localtime_r(&t, &tm);
#endif
	ss << std::put_time(&tm, logFormat);
	return ss.str();
}

// New: unify formatting
std::string Logger::formatLine(Level level, std::string_view message, const std::string& time) {
	std::string line;
	line.reserve(time.size() + 1 + 16 + message.size() + 1);
	line.append(time)
		.append(" ")
		.append(levelTag(level))
		.append(" ")
		.append(message)
		.push_back('\n');
	return line;
}

// New: sinks + buffer + streams (single path)
void Logger::write(Level level, std::string_view message, bool alsoCrashFile) {
	const std::string time = nowString();
	const std::string formatted = formatLine(level, message, time);

	// console stream selection without repeating time formatting
	{
		// stdout for info, stderr for warnings/errors
		std::ostream& os = (level == Level::Error) ? std::cerr
							 : (level == Level::Warning) ? std::cerr
							 : std::cout;
		// Print without trailing extra newline (formatted has '\n')
		os << formatted;
		os.flush();
	}

	{
		std::lock_guard<std::mutex> lock(mtx_);
		consoleLogs.push_back(formatted);
		while (consoleLogs.size() > maxMessages_) {
			consoleLogs.pop_front();
		}
		// Fan-out to sinks
		for (auto const& sink : sinks_) {
			if (sink) sink(formatted, level);
		}
	}

	if (alsoCrashFile) {
		log_crash_to_file(message);
	}
}

// Backward-compatible wrappers
void Logger::error(std::string_view message) {
	write(Level::Error, message, /*alsoCrashFile*/ true);
}

void Logger::log(std::string_view message) {
	write(Level::Info, message, /*alsoCrashFile*/ false);
}

void Logger::warning(std::string_view message) {
	write(Level::Warning, message, /*alsoCrashFile*/ false);
}

std::deque<std::string> Logger::getLogs() const& {
	std::lock_guard<std::mutex> lock(mtx_);
	return consoleLogs; // copy out safely for UI like ImGui overlay
}

void Logger::addSink(Sink sink) {
	std::lock_guard<std::mutex> lock(mtx_);
	sinks_.push_back(std::move(sink));
}

void Logger::setMaxMessages(std::size_t max) {
	std::lock_guard<std::mutex> lock(mtx_);
	maxMessages_ = max > 0 ? max : 1;
	while (consoleLogs.size() > maxMessages_) {
		consoleLogs.pop_front();
	}
}

std::string Logger::getCurrentTime() {
	// Keep signature for compatibility; reuse thread-safe impl
	return nowString();
}

std::string Logger::getUniqueTimeId() {
	std::stringstream ss;
	auto now = std::chrono::system_clock::now();
	std::time_t t = std::chrono::system_clock::to_time_t(now);
	std::tm tm{};
#if defined(_WIN32)
	localtime_s(&tm, &t);
#else
	localtime_r(&t, &tm);
#endif
#pragma warning(suppress : 4996)
	ss << std::put_time(&tm, fileNameFormat);
#pragma warning(default : 4996)
	return ss.str() + std::to_string(logCounter++);
}