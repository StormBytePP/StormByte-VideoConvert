#include "base.hxx"
#include "definitions.h"
#include "ffmpeg_path.h"

using namespace StormByte::VideoConvert;

const std::list<std::string> Task::Execute::FFmpeg::Base::FFMPEG_INIT_OPTIONS = { "-hide_banner", "-y", "-loglevel", "error", "-map_metadata", "0", "-map_chapters", "0" };

Task::Execute::FFmpeg::Base::Base(const VideoConvert::FFmpeg& ffmpeg):Execute::Base(FFMPEG_EXECUTABLE), m_ffmpeg(ffmpeg) {}

Task::Execute::FFmpeg::Base::Base(VideoConvert::FFmpeg&& ffmpeg):Execute::Base(FFMPEG_EXECUTABLE), m_ffmpeg(std::move(ffmpeg)) {}

Task::Execute::FFmpeg::Base::~Base() noexcept {}

Task::STATUS Task::Execute::FFmpeg::Base::pre_run_actions() noexcept {
	std::list<std::string> result = FFMPEG_INIT_OPTIONS;

	for (auto it = m_ffmpeg.get_streams().begin(); it != m_ffmpeg.get_streams().end(); it++) {
		auto parameters = (*it)->ffmpeg_parameters();
		result.insert(result.end(), parameters.begin(), parameters.end());
	}

	result.push_back("-metadata"); result.push_back("title=");

	result.push_back("-metadata:s:v"); result.push_back("encoder=\"" + std::string(PROGRAM_NAME) + " " + std::string(PROGRAM_VERSION) + " ( " + std::string(PROJECT_URI) + " )\"" );

	result.push_back("-map 0:t?"); result.push_back("-c:t copy");

	m_executables[0].m_arguments += " " + boost::algorithm::join(result, " ");

	return Execute::Base::pre_run_actions();
}
