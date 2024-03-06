#include "audio.hxx"

Alchemist::Media::Decoder::Audio::Audio(const Media::Decoder::Type& decoder_type):Base(decoder_type) { }

Alchemist::Media::Decoder::Audio::Audio(Media::Decoder::Type&& decoder_type):Base(std::move(decoder_type)) { }

Alchemist::Media::Decoder::Audio::~Audio() { }