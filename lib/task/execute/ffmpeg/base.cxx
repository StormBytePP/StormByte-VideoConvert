#include "base.hxx"

using namespace StormByte::VideoConvert;

const std::list<std::string> Task::Execute::FFmpeg::Base::FFMPEG_INIT_OPTIONS = { "-hide_banner", "-y", "-map_metadata", "0", "-map_chapters", "0" };

Task::Execute::FFmpeg::Base::Base(const VideoConvert::FFmpeg& ffmpeg):Execute::Base(FFMPEG_EXECUTABLE), m_ffmpeg(ffmpeg) {}

Task::Execute::FFmpeg::Base::Base(VideoConvert::FFmpeg&& ffmpeg):Execute::Base(FFMPEG_EXECUTABLE), m_ffmpeg(std::move(ffmpeg)) {}

Task::Execute::FFmpeg::Base::~Base() {}

std::string Task::Execute::FFmpeg::Base::create_arguments() const {
	std::list<std::string> result = FFMPEG_INIT_OPTIONS;

	for (auto it = m_ffmpeg.get_streams().begin(); it != m_ffmpeg.get_streams().end(); it++) {
		auto parameters = (*it)->ffmpeg_parameters();
		result.insert(result.end(), parameters.begin(), parameters.end());
	}

	if (m_ffmpeg.get_title()) {
		result.push_back("-metadata"); result.push_back("title=\"" + *m_ffmpeg.get_title() + "\"");
	}
	else {
		result.push_back("-metadata"); result.push_back("title=");
	}

	result.push_back("-metadata:s:v"); result.push_back("encoder=\"" + std::string(PROGRAM_NAME) + " " + std::string(PROGRAM_VERSION) + " ( " + std::string(PROJECT_URI) + " )\"" );

	return boost::algorithm::join(result, " ");
}
