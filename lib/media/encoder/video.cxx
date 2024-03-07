#include "video.hxx"

Alchemist::Media::Encoder::Video::Base::Base(const Type& encoder_type):Encoder::Base(encoder_type) { }

Alchemist::Media::Encoder::Video::Base::Base(Type&& encoder_type):Encoder::Base(std::move(encoder_type)) { }

Alchemist::Media::Encoder::Video::Base::~Base() { }