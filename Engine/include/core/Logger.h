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
    /**
     * @brief Log an error message.
     *
     * Logs the message with Error level, writes to stderr, appends to the in-memory log
     * buffer and also writes the message to the persistent crash file.
     *
     * @param message Short text describing the error.
     */
    void error(std::string_view message);

    /**
     * @brief Log an informational message.
     *
     * Logs the message with Info level, writes to stdout and appends the formatted
     * line to the in-memory log buffer and any registered sinks.
     *
     * @param message Message text to record.
     */
    void log(std::string_view message);

    /**
     * @brief Log a warning message.
     *
     * Logs the message with Warning level, writes to stderr and appends to the
     * in-memory buffer and sinks (no crash file write).
     *
     * @param message Warning text to record.
     */
    void warning(std::string_view message);

    /**
     * @brief Install OS signal handlers for common crash conditions.
     *
     * Registers handlers for SIGSEGV, SIGFPE, SIGILL and SIGABRT which will
     * write a brief crash description to the crash log file. Safe to call once
     * during application initialization.
     */
    static void setup_crash_handler();

    /**
     * @brief Return a copy of the recent in-memory console logs.
     *
     * Returns a deque copy holding formatted log lines (with timestamps and level).
     * Thread-safe.
     *
     * @return deque of formatted log lines.
     */
    std::deque<std::string> getLogs() const&;

    /**
     * @brief Return a thread-safe formatted current time string.
     *
     * Kept as a static compatibility helper; delegates to the internal safe time
     * formatting implementation.
     *
     * @return Formatted time string suitable for log prefixes.
     */
    static std::string getCurrentTime();

    /**
     * @brief Produce a short unique time-based id for filenames.
     *
     * Combines a time stamp formatted for filenames with an incrementing counter.
     *
     * @return Unique string id.
     */
    static std::string getUniqueTimeId();

    static constexpr char logFormat[] = "[%d-%m-%Y] [%H:%M:%S]";
    static constexpr char fileNameFormat[] = "%d%m%Y-%H%I%S-";
    static unsigned int logCounter;

    // New: log level + sinks (optional to use; no breaking changes)
    enum class Level { Info, Warning, Error };

    // Sink receives formatted line and level
    using Sink = std::function<void(const std::string& /*formatted*/, Level /*level*/)>;

    /**
     * @brief Add an external sink to receive every formatted log line.
     *
     * The sink will be invoked on the same thread that issues the log call.
     * Thread-safe.
     *
     * @param sink Callable receiving (formattedLine, level).
     */
    void addSink(Sink sink);

    /**
     * @brief Configure the in-memory log message cap.
     *
     * Controls how many formatted lines are kept in memory for UI inspection.
     * Values <= 0 are clamped to 1.
     *
     * @param max Maximum number of messages to retain.
     */
    void setMaxMessages(std::size_t max);

private:
    // Preserve existing storage for UI compatibility
    std::deque<std::string> consoleLogs;

    // New internals
    mutable std::mutex mtx_;
    std::vector<Sink> sinks_;
    std::size_t maxMessages_ = 50;

    /**
     * @brief Central single-path write routine.
     *
     * Formats the message with timestamp and level, writes to stdout/stderr,
     * appends to the in-memory buffer, fans out to sinks and optionally writes
     * a copy to the crash file (used for errors).
     *
     * This function is thread-safe.
     *
     * @param level Log level (Info/Warning/Error).
     * @param message Raw message text (unformatted).
     * @param alsoCrashFile If true, additionally append the raw message to the persistent crash log.
     */
    void write(Level level, std::string_view message, bool alsoCrashFile = false);

    // Helpers
    /**
     * @brief Return textual tag for a log Level (e.g. "[ERROR]").
     *
     * Cheap, pure helper used by the formatter.
     */
    static const char* levelTag(Level level);

    /**
     * @brief Build a full formatted line from components.
     *
     * Uses an already-obtained time string to avoid duplicate time formatting
     * when called from write().
     *
     * @param level Log level.
     * @param message Raw message text.
     * @param time Preformatted time prefix.
     * @return Single-line formatted string ready for output.
     */
    static std::string formatLine(Level level, std::string_view message, const std::string& time);

    /**
     * @brief Thread-safe time string generator used internally.
     *
     * Produces a string suitable for inclusion in formatted log lines.
     */
    static std::string nowString(); // thread-safe time formatting

    /**
     * @brief Append a message to the persistent crash file.
     *
     * Creates parent directory if required. Intended for fatal errors / crash hooks.
     *
     * @param message Message text to persist.
     */
    static void log_crash_to_file(std::string_view message);

    /**
     * @brief Signal handler invoked on abnormal termination signals.
     *
     * Translates common signals to human-readable messages and writes them to the crash file.
     *
     * @param signal The OS signal number received.
     */
    static void handle_crash_error(int signal);
};

#endif // LOGGER_H