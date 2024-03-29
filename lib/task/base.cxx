#include "base.hxx"

#include <assert.h>

using namespace StormByte::VideoConvert;

Task::Base::Base():m_status(STOPPED) {}

Task::STATUS Task::Base::run() noexcept {
	std::optional<pid_t> useless;
	return run(useless);
}

Task::STATUS Task::Base::run(std::optional<pid_t>& worker) noexcept {
	STATUS status = HALT_ERROR;

	m_start = std::chrono::steady_clock::now();
	// Pre run actions
	status = pre_run_actions();
	
	// do work only if pre run actions where ok
	if (status == RUNNING)
		status = do_work(worker);
	
	// update now end time so post run actions can take its elapset time value
	m_end = std::chrono::steady_clock::now();
	
	// Post run action might change status
	status = post_run_actions(status);
	
	assert(status == HALT_OK || status == HALT_ERROR);
	return status;
}

Task::STATUS Task::Base::pre_run_actions() noexcept { return RUNNING; }

Task::STATUS Task::Base::post_run_actions(const STATUS& prev_status) noexcept {
	return prev_status == RUNNING ? HALT_OK : prev_status;
}

std::string Task::Base::elapsed_time_string() const {
	/* NOTE: Until C++20's <format> support is complete, I just use this aproach */
	std::string result = "";

	auto elapsed = std::chrono::hh_mm_ss(m_end - m_start);
	auto h = elapsed.hours().count();
	auto m = elapsed.minutes().count();
	auto s = elapsed.seconds().count();

	result += std::to_string(h) + ":";
	if (m < 10) result += "0";
	result += std::to_string(m) + ":";
	if (s < 10) result += "0";
	result += std::to_string(s);

	return result;;
}