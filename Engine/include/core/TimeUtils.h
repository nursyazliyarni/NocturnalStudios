#pragma once

#include <chrono>
#include <functional>
#include <utility>

namespace TimeUtils {
	/// Alias for a monotonic clock suitable for measuring intervals.
	/// Uses steady_clock to avoid issues with system clock adjustments.
	using Clock = std::chrono::steady_clock;

	/**
	 * @brief Scoped timer that writes the elapsed seconds to a reference when destroyed.
	 *
	 * Constructs and stores the start time on creation. On destruction it computes the elapsed
	 * time (in seconds) and assigns it to the provided `out` reference.
	 *
	 * Typical use:
	 * ```
	 * double elapsed = 0.0;
	 * {
	 *     TimeUtils::ScopedTimer timer(elapsed);
	 *     // work to measure...
	 * } // elapsed is now set to the scope duration in seconds
	 * ```
	 *
	 * Notes:
	 * - The measured unit is seconds (double).
	 * - Uses `steady_clock`, so the measurement is monotonic.
	 * - If destruction occurs during stack unwinding (e.g. because of an exception), the destructor
	 *   will still run and `out` will be set.
	 */
	class ScopedTimer {
	public:
		explicit ScopedTimer(double& outSeconds) :
			out(outSeconds), start(Clock::now()) {
		}

		~ScopedTimer() {
			auto end = Clock::now();
			out = std::chrono::duration<double>(end - start).count();
		}

	private:
		double& out;
		Clock::time_point start;
	};

	/**
	 * @brief Scoped accumulator that adds the elapsed seconds to a reference when destroyed.
	 *
	 * Useful when you want to accumulate multiple durations into the same variable:
	 *
	 * ```
	 * double total = 0.0;
	 * {
	 *     TimeUtils::ScopedAccumulator acc(total);
	 *     // part A...
	 * } // total += duration of part A
	 *
	 * {
	 *     TimeUtils::ScopedAccumulator acc(total);
	 *     // part B...
	 * } // total += duration of part B
	 * ```
	 *
	 * Notes:
	 * - The measured unit is seconds (double).
	 * - Uses `steady_clock` for monotonic timing.
	 */
	class ScopedAccumulator {
	public:
		explicit ScopedAccumulator(double& outSeconds) :
			out(outSeconds), start(Clock::now()) {}

		~ScopedAccumulator() {
			const auto end = Clock::now();
			out += std::chrono::duration<double>(end - start).count();
		}

	private:
		double& out;
		Clock::time_point start;
	};

	/**
	 * @brief Measure the execution time (in seconds) of a callable.
	 *
	 * Invokes `func(args...)` and returns the elapsed time in seconds as a double.
	 *
	 * Example:
	 * ```
	 * auto work = [](int a, int b) {
	 *     // ... work ...
	 * };
	 * double seconds = TimeUtils::MeasureSeconds(work, 1, 2);
	 * ```
	 *
	 * Important:
	 * - If the invoked callable throws, `MeasureSeconds` will not return a value (the exception
	 *   propagates) and the duration will not be returned from this function. To ensure timing is
	 *   captured even when exceptions occur, wrap the call in a try/catch and measure manually
	 *   (for example using `ScopedTimer`) or measure inside the callable.
	 *
	 * @tparam Func Callable type.
	 * @tparam Args Argument types forwarded to the callable.
	 * @param func Callable to invoke.
	 * @param args Forwarded arguments to `func`.
	 * @return Elapsed time in seconds as `double`.
	 */
	template <typename Func, typename... Args>
	double MeasureSeconds(Func&& func, Args&&... args) {
		const auto start = Clock::now();
		std::invoke(std::forward<Func>(func), std::forward<Args>(args)...);
		const auto end = Clock::now();
		return std::chrono::duration<double>(end - start).count();
	}
}
