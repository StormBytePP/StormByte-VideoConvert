#include "decoder.hxx"

const std::list<StormByte::Alchemist::Media::Decoder::Type> StormByte::Alchemist::Media::Decoder::Available() {
	return {
		// Video
		AV1_DEFAULT,
		#ifdef ENABLE_DAV1D
		AV1_DAV1D,
		#endif
		#ifdef ENABLE_LIBAOM
		AV1_AOM,
		#endif
		#ifdef ENABLE_CUDA
		AV1_CUVID,
		#endif
		H264_DEFAULT,
		#ifdef ENABLE_OPENH264
		H264_OPENH264,
		#endif
		#ifdef ENABLE_CUDA
		H264_CUVID,
		#endif
		H265_DEFAULT,
		#ifdef ENABLE_CUDA
		H265_CUVID,
		#endif
		MPEG2_DEFAULT,
		#ifdef ENABLE_CUDA
		MPEG2_CUVID,
		#endif
		VP8_DEFAULT,
		#ifdef ENABLE_LIBVPX
		VP8_LIBVPX,
		#endif
		#ifdef ENABLE_CUDA
		VP8_CUVID,
		#endif
		VP9_DEFAULT,
		#ifdef ENABLE_LIBVPX
		VP9_LIBVPX,
		#endif
		#ifdef ENABLE_CUDA
		VP9_CUVID,
		#endif
		MPEG4_DEFAULT,
		#ifdef ENABL_CUDA
		MPEG4_CUVID,
		#endif

		// Audio
		AAC_DEFAULT,
		#ifdef ENABLE_FDK
		AAC_FRAUNHOFFER,
		#endif
		AC3_DEFAULT,
		DTS_DEFAULT,
		EAC3_DEFAULT,
		FLAC_DEFAULT,
		MP2_DEFAULT,
		MP3_DEFAULT,
		OPUS_DEFAULT,
		#ifdef ENABLE_LIBOPUS
		OPUS_LIBOPUS,
		#endif
		VORBIS_DEFAULT,
		#ifdef ENABLE_LIBVORBIS
		VORBIS_LIBVORBIS,
		#endif

		// Image
		BMP_DEFAULT,
		GIF_DEFAULT,
		JPG_DEFAULT,
		PNG_DEFAULT,
		TIFF_DEFAULT,
		WEBP_DEFAULT
	};
}