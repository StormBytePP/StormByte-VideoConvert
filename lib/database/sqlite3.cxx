#include "sqlite3.hxx"
#include "application.hxx"
#include "ffmpeg/ffmpeg.hxx"

#include <stdexcept>

using namespace StormByte::VideoConvert;

const std::string Database::SQLite3::DATABASE_CREATE_SQL =
	"CREATE TABLE films("
		"id INTEGER PRIMARY KEY AUTOINCREMENT,"
		"file VARCHAR NOT NULL,"
		"prio TINYINT,"
		"processing BOOL DEFAULT FALSE,"
		"unsupported BOOL DEFAULT FALSE"
	");"
	"CREATE TABLE streams("
		"id INTEGER,"
        "film_id INTEGER,"
		"codec INTEGER NOT NULL,"
		"is_animation BOOL DEFAULT FALSE,"
        "max_rate VARCHAR DEFAULT NULL,"
        "bitrate VARCHAR DEFAULT NULL,"
        "PRIMARY KEY(id, film_id, codec),"
        "FOREIGN KEY(film_id) REFERENCES films(id) ON DELETE CASCADE"
	");"
	"CREATE TABLE stream_hdr("
		"film_id INTEGER,"
		"stream_id INTEGER,"
		"codec INTEGER,"
		"red_x INTEGER NOT NULL,"
		"red_y INTEGER NOT NULL,"
		"green_x INTEGER NOT NULL,"
		"green_y INTEGER NOT NULL,"
		"blue_x INTEGER NOT NULL,"
		"blue_y INTEGER NOT NULL,"
		"white_point_x INTEGER NOT NULL,"
		"white_point_y INTEGER NOT NULL,"
		"luminance_min INTEGER NOT NULL,"
		"luminance_max INTEGER NOT NULL,"
		"light_level_max INTEGER,"
		"light_level_average INTEGER,"
		"PRIMARY KEY (film_id, stream_id, codec),"
		"FOREIGN KEY (film_id, stream_id, codec) REFERENCES streams(id, film_id, codec) ON DELETE CASCADE"
	");";

const std::map<std::string, std::string> Database::SQLite3::DATABASE_PREPARED_SENTENCES = {
	{"getFilmIDForProcess", 		"SELECT id FROM films WHERE processing = FALSE AND unsupported = FALSE ORDER BY prio ASC LIMIT 1"},
	{"setProcessingStatusForFilm",	"UPDATE films SET processing = ? WHERE id = ?"},
	{"setUnsupportedStatusForFilm",	"UPDATE films SET unsupported = ? WHERE id = ?"},
	{"deleteFilmStreamHDR",			"DELETE FROM stream_hdr WHERE film_id = ?"},
	{"getFilmBasicData",			"SELECT file, prio, processing FROM films WHERE id = ?"},
	{"getFilmStreams",				"SELECT id, codec, is_animation, max_rate, bitrate FROM streams WHERE film_id = ?"},
	{"hasStreamHDR?",				"SELECT COUNT(*)>0 FROM stream_hdr WHERE film_id = ? AND stream_id = ? AND codec = ?"},
	{"getFilmStreamHDR",			"SELECT red_x, red_y, green_x, green_y, blue_x, blue_y, white_point_x, white_point_y, luminance_min, luminance_max, light_level_max, light_level_average FROM stream_hdr WHERE film_id = ? AND stream_id = ? AND codec = ?"},
	{"insertFilm",					"INSERT INTO films(file, prio) VALUES (?, ?) RETURNING id"},
	{"insertStream",				"INSERT INTO streams(id, film_id, codec, is_animation, max_rate, bitrate) VALUES (?, ?, ?, ?, ?, ?)"},
	{"insertHDR",					"INSERT INTO stream_hdr(film_id, stream_id, codec, red_x, red_y, green_x, green_y, blue_x, blue_y, white_point_x, white_point_y, luminance_min, luminance_max, light_level_max, light_level_average) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)"},
	{"resetProcessingFilms",		"UPDATE films SET processing = FALSE, unsupported = FALSE"},
	{"deleteFilm",					"DELETE FROM films WHERE id = ?"},
	{"deleteFilmStream",			"DELETE FROM streams WHERE film_id = ?"},
	{"deleteFilmStreamHDR",			"DELETE FROM stream_hdr WHERE film_id = ?"}
};

Database::SQLite3::SQLite3(const std::filesystem::path& dbfile) {
	int rc = sqlite3_open(dbfile.c_str(), &m_database);

	if (rc != SQLITE_OK) {
		const std::string message = "Cannot open database " + dbfile.string() + ": " + std::string(sqlite3_errmsg(m_database));
		sqlite3_close(m_database); // Need to close database here as exception throwing will skip destructor
        throw std::runtime_error(message);
    }
	if (!check_database()) init_database();
	prepare_sentences();
	
}
Database::SQLite3::~SQLite3() {
	for (auto it = m_prepared.begin(); it != m_prepared.end(); it++) {
		sqlite3_finalize(it->second);
	}
	m_prepared.clear();
	sqlite3_close(m_database);
}

std::optional<FFmpeg> Database::SQLite3::get_film_for_process() {
	std::optional<FFmpeg> ffmpeg;
	int film_id = get_film_id_for_process();
	auto logger = Application::get_instance().get_logger();

	if (film_id != -1) {
		Data::film film_data = get_film_basic_data(film_id);
		std::filesystem::path input_path	= Application::get_instance().get_input_folder();
		std::filesystem::path output_path	= Application::get_instance().get_output_folder();
		std::filesystem::path work_path	= Application::get_instance().get_work_folder();
		FFmpeg film(film_id, input_path, film_data.file, work_path, output_path);
		auto streams = get_film_streams(film_id);
		std::list<Data::stream_codec> unsupported_codecs;

		for (auto it = streams.begin(); it != streams.end(); it++) {
			switch (it->codec) {
				case Data::VIDEO_HEVC: {
					#ifdef ENABLE_HEVC
					auto codec = Stream::Video::HEVC(it->id);
					if (it->HDR.has_value()) {
						auto hdr_data = it->HDR.value();
						Stream::Video::HEVC::HDR hdr(	hdr_data.red_x, hdr_data.red_y,
												hdr_data.green_x, hdr_data.green_y,
												hdr_data.blue_x, hdr_data.blue_y,
												hdr_data.white_point_x, hdr_data.white_point_y,
												hdr_data.luminance_min, hdr_data.luminance_max);
						if (hdr_data.light_level_max.has_value() && hdr_data.light_level_average.has_value()) {
							hdr.set_light_level(hdr_data.light_level_max.value(), hdr_data.light_level_average.value());
						}
						codec.set_HDR(hdr);
					}
					if (it->is_animation) codec.set_tune_animation();
					film.add_stream(codec);
					#else
					unsupported_codecs.push_back(Data::VIDEO_HEVC);
					#endif
					break;
				}
				case Data::VIDEO_COPY: {
					film.add_stream(Stream::Video::Copy(it->id));
					break;
				}
				case Data::AUDIO_AAC: {
					#ifdef ENABLE_AAC
					film.add_stream(Stream::Audio::AAC(it->id));
					#else
					unsupported_codecs.push_back(Data::AUDIO_AAC);
					#endif
					break;
				}
				case Data::AUDIO_FDKAAC: {
					#ifdef ENABLE_FDKAAC
					film.add_stream(Stream::Audio::FDKAAC(it->id));
					#else
					unsupported_codecs.push_back(Data::AUDIO_FDKAAC);
					#endif
					break;
				}
				case Data::AUDIO_AC3: {
					#ifdef ENABLE_AC3
					film.add_stream(Stream::Audio::AC3(it->id));
					#else
					unsupported_codecs.push_back(Data::AUDIO_AC3);
					#endif
					break;
				}
				case Data::AUDIO_COPY: {
					film.add_stream(Stream::Audio::Copy(it->id));
					break;
				}
				case Data::AUDIO_EAC3: {
					#ifdef ENABLE_EAC3
					film.add_stream(Stream::Audio::EAC3(it->id));
					#else
					unsupported_codecs.push_back(Data::AUDIO_EAC3);
					#endif
					break;
				}
				case Data::AUDIO_OPUS: {
					#ifdef ENABLE_OPUS
					film.add_stream(Stream::Audio::Opus(it->id));
					#else
					unsupported_codecs.push_back(Data::AUDIO_OPUS);
					#endif
					break;
				}
				case Data::SUBTITLE_COPY: {
					film.add_stream(Stream::Subtitle::Copy(it->id));
					break;
				}
				default: {
					break;
				}
			}
		}

		// We now check if we have unsupported codecs
		if (!unsupported_codecs.empty()) {
			logger->message_part_begin(Utils::Logger::LEVEL_ERROR, "The file " + film.get_input_file().string() + " has the following unsupported codecs: ");
			for (auto it = unsupported_codecs.begin(); it != unsupported_codecs.end(); it++) {
				logger->message_part_continue(Utils::Logger::LEVEL_ERROR, Database::Data::codec_string.at(*it) + ", ");
			}
			logger->message_part_end(Utils::Logger::LEVEL_ERROR, "and therefore could NOT be converted!");
			logger->message_line(Utils::Logger::LEVEL_DEBUG, "Marking file " + film.get_input_file().string() + " as unsupported");
			set_film_unsupported_status(film.get_film_id(), true);
		}
		else
			ffmpeg.emplace(std::move(film));
	}
	return ffmpeg;
}

bool Database::SQLite3::check_database() {
	char* err_msg = NULL;
	int rc = sqlite3_exec(m_database, "SELECT * FROM films;", 0, 0, &err_msg);
	sqlite3_free(err_msg);
	return rc == SQLITE_OK;
}

void Database::SQLite3::init_database() {
	char* err_msg = NULL;
	int rc = sqlite3_exec(m_database, DATABASE_CREATE_SQL.c_str(), 0, 0, &err_msg);
	if (rc != SQLITE_OK) throw_error(err_msg);
}

void Database::SQLite3::throw_error(char* err_msg) {
	const std::string message = "Error creating database: " + std::string(err_msg);
	sqlite3_free(err_msg);
	throw std::runtime_error(message);
}

void Database::SQLite3::reset_stmt(sqlite3_stmt* stmt) {
	sqlite3_clear_bindings(stmt);
	sqlite3_reset(stmt);
}

void Database::SQLite3::prepare_sentences() {
	for (auto it = DATABASE_PREPARED_SENTENCES.begin(); it != DATABASE_PREPARED_SENTENCES.end(); it++) {
		m_prepared[it->first] = nullptr;
		sqlite3_prepare_v2( m_database, it->second.c_str(), it->second.length(), &m_prepared[it->first], nullptr);
		if (!m_prepared[it->first])
			throw std::runtime_error("Prepared sentence " + (it->first) + " can not be loaded!");
	}
}

int Database::SQLite3::get_film_id_for_process() {
	int result = -1; // No films for converting available
	auto stmt = m_prepared["getFilmIDForProcess"];
	if (sqlite3_step(stmt) == SQLITE_ROW) {
		result = sqlite3_column_int(stmt, 0);
	}
	sqlite3_reset(stmt);
	return result;
}

void Database::SQLite3::set_film_processing_status(int film_id, bool status) {
	auto stmt = m_prepared["setProcessingStatusForFilm"];
	sqlite3_bind_int(stmt, 1, status);
	sqlite3_bind_int(stmt, 2, film_id);
	sqlite3_step(stmt);
	reset_stmt(stmt);
}

void Database::SQLite3::set_film_unsupported_status(int film_id, bool status) {
	auto stmt = m_prepared["setUnsupportedStatusForFilm"];
	sqlite3_bind_int(stmt, 1, status);
	sqlite3_bind_int(stmt, 2, film_id);
	sqlite3_step(stmt);
	reset_stmt(stmt);
}

Database::Data::film Database::SQLite3::get_film_basic_data(int film_id) {
	Data::film result;
	auto stmt = m_prepared["getFilmBasicData"];
	sqlite3_bind_int(stmt, 1, film_id);
	if (sqlite3_step(stmt) == SQLITE_ROW) {
		result.file 		= reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
		result.prio 		= sqlite3_column_int(stmt, 1);
		result.processing	= sqlite3_column_int(stmt, 2);
	}
	reset_stmt(stmt);
	return result;
}

std::list<Database::Data::stream> Database::SQLite3::get_film_streams(int film_id) {
	std::list<Data::stream> result;
	auto stmt = m_prepared["getFilmStreams"];
	sqlite3_bind_int(stmt, 1, film_id);
	while (sqlite3_step(stmt) == SQLITE_ROW) {
		Data::stream stream;
		stream.film_id 		= film_id;
		stream.id 			= sqlite3_column_int(stmt, 0);
		stream.codec 		= static_cast<Data::stream_codec>(sqlite3_column_int(stmt, 1));
		stream.is_animation	= sqlite3_column_int(stmt, 2);
		if (sqlite3_column_type(stmt, 3) != SQLITE_NULL) stream.max_rate 	= reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
		if (sqlite3_column_type(stmt, 4) != SQLITE_NULL) stream.bitrate 	= reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
		if (has_film_stream_HDR(stream)) {
			stream.HDR = get_film_stream_HDR(stream);
		}
		result.push_back(stream);
	}
	reset_stmt(stmt);
	return result;
}

bool Database::SQLite3::has_film_stream_HDR(const Data::stream& stream) {
	bool result = false;
	auto stmt = m_prepared["hasStreamHDR?"];
	sqlite3_bind_int(stmt, 1, stream.film_id);
	sqlite3_bind_int(stmt, 2, stream.id);
	sqlite3_bind_int(stmt, 3, stream.codec);
	if (sqlite3_step(stmt) == SQLITE_ROW) {
		result = sqlite3_column_int(stmt, 0);
	}
	reset_stmt(stmt);
	return result;
}

Database::Data::hdr Database::SQLite3::get_film_stream_HDR(const Data::stream& stream) {
	Data::hdr result;
	auto stmt = m_prepared["getFilmStreamHDR"];
	sqlite3_bind_int(stmt, 1, stream.film_id);
	sqlite3_bind_int(stmt, 2, stream.id);
	sqlite3_bind_int(stmt, 3, stream.codec);
	if (sqlite3_step(stmt) == SQLITE_ROW) {
		result.red_x			= sqlite3_column_int(stmt, 0);
		result.red_y			= sqlite3_column_int(stmt, 1);
		result.green_x			= sqlite3_column_int(stmt, 2);
		result.green_y			= sqlite3_column_int(stmt, 3);
		result.blue_x			= sqlite3_column_int(stmt, 4);
		result.blue_y			= sqlite3_column_int(stmt, 5);
		result.white_point_x	= sqlite3_column_int(stmt, 6);
		result.white_point_y	= sqlite3_column_int(stmt, 7);
		result.luminance_min	= sqlite3_column_int(stmt, 8);
		result.luminance_max	= sqlite3_column_int(stmt, 9);
		if (sqlite3_column_type(stmt, 10) != SQLITE_NULL) result.light_level_max		= sqlite3_column_int(stmt, 10);
		if (sqlite3_column_type(stmt, 11) != SQLITE_NULL) result.light_level_average	= sqlite3_column_int(stmt, 11);
	}
	reset_stmt(stmt);
	return result;
}

bool Database::SQLite3::insert_film(Data::film& film) {
	int result = -1;
	auto stmt = m_prepared["insertFilm"];
	sqlite3_bind_text(stmt, 1, film.file.c_str(), -1, SQLITE_STATIC);
	sqlite3_bind_int(stmt, 2, film.prio);
	if (sqlite3_step(stmt) == SQLITE_ROW)
		result = sqlite3_column_int(stmt, 0);
	reset_stmt(stmt);
	if (result>=0) {
		film.film_id = result;
		return true;
	}
	else
		return false;
}

void Database::SQLite3::insert_stream(const Data::stream& stream) {
	auto stmt = m_prepared["insertStream"];
	sqlite3_bind_int(stmt, 1, stream.id);
	sqlite3_bind_int(stmt, 2, stream.film_id);
	sqlite3_bind_int(stmt, 3, stream.codec);
	sqlite3_bind_int(stmt, 4, stream.is_animation);
	if (stream.max_rate.has_value())
		sqlite3_bind_text(stmt, 5, stream.max_rate.value().c_str(), -1, SQLITE_STATIC);
	else
		sqlite3_bind_null(stmt, 5);
	if (stream.bitrate.has_value())
		sqlite3_bind_text(stmt, 6, stream.bitrate.value().c_str(), -1, SQLITE_STATIC);
	else
		sqlite3_bind_null(stmt, 6);
	sqlite3_step(stmt); // No result
	reset_stmt(stmt);

	if (stream.HDR.has_value()) insert_HDR(stream);
}

void Database::SQLite3::insert_HDR(const Data::stream& stream) {
	auto stmt = m_prepared["insertHDR"];
	sqlite3_bind_int(stmt, 1, stream.film_id);
	sqlite3_bind_int(stmt, 2, stream.id);
	sqlite3_bind_int(stmt, 3, stream.codec);
	sqlite3_bind_int(stmt, 4, stream.HDR.value().red_x);
	sqlite3_bind_int(stmt, 5, stream.HDR.value().red_y);
	sqlite3_bind_int(stmt, 6, stream.HDR.value().green_x);
	sqlite3_bind_int(stmt, 7, stream.HDR.value().green_y);
	sqlite3_bind_int(stmt, 8, stream.HDR.value().blue_x);
	sqlite3_bind_int(stmt, 9, stream.HDR.value().blue_y);
	sqlite3_bind_int(stmt, 10, stream.HDR.value().white_point_x);
	sqlite3_bind_int(stmt, 11, stream.HDR.value().white_point_y);
	sqlite3_bind_int(stmt, 12, stream.HDR.value().luminance_min);
	sqlite3_bind_int(stmt, 13, stream.HDR.value().luminance_max);
	if (stream.HDR.value().light_level_max.has_value())
		sqlite3_bind_int(stmt, 14, stream.HDR.value().light_level_max.value());
	else
		sqlite3_bind_null(stmt, 14);
	if (stream.HDR.value().light_level_average.has_value())
		sqlite3_bind_int(stmt, 15, stream.HDR.value().light_level_average.value());
	else
		sqlite3_bind_null(stmt, 15);
	sqlite3_step(stmt); // No result
	reset_stmt(stmt);
}

void Database::SQLite3::reset_processing_films() {
	auto stmt = m_prepared["resetProcessingFilms"];
	sqlite3_step(stmt); // No result
	reset_stmt(stmt);
}

void Database::SQLite3::delete_film(int film_id) {
	auto stmt = m_prepared["deleteFilm"];
	sqlite3_bind_int(stmt, 1, film_id);
	sqlite3_step(stmt);
	reset_stmt(stmt);
	delete_film_stream(film_id);
	delete_film_stream_HDR(film_id);
}

void Database::SQLite3::delete_film_stream(int film_id) {
	auto stmt = m_prepared["deleteFilmStream"];
	sqlite3_bind_int(stmt, 1, film_id);
	sqlite3_step(stmt);
	reset_stmt(stmt);
}

void Database::SQLite3::delete_film_stream_HDR(int film_id) {
	auto stmt = m_prepared["deleteFilmStreamHDR"];
	sqlite3_bind_int(stmt, 1, film_id);
	sqlite3_step(stmt);
	reset_stmt(stmt);
}
