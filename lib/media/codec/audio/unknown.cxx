#include "unknown.hxx"

Alchemist::Media::Codec::Audio::Unknown::Unknown(const std::string& name, const std::string& long_name):Base(Codec::UNKNOWN_AUDIO), m_name(name), m_long_name(long_name) { }

std::string Alchemist::Media::Codec::Audio::Unknown::get_name() const { return m_name; }

std::string Alchemist::Media::Codec::Audio::Unknown::get_long_name() const { return m_long_name; }

std::list<Alchemist::Media::Decoder::Type> Alchemist::Media::Codec::Audio::Unknown::get_available_decoders() const {
	return { };
}

std::list<Alchemist::Media::Encoder::Type> Alchemist::Media::Codec::Audio::Unknown::get_available_encoders() const {
	return { };
}