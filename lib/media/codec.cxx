#include "codec.hxx"

const std::list<StormByte::Alchemist::Media::Codec::Type> StormByte::Alchemist::Media::Codec::Available() {
	return {
		AV1, H264, HEVC, MPEG2, VP8, VP9, MPEG4,			// Video
		AAC, AC3, DTS, EAC3, FLAC, MP2, MP3, OPUS, VORBIS,	// Audio
		BMP, GIF, JPG, PNG, TIFF, WEBP						// Image
	};
}
