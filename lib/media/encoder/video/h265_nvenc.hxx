#pragma once

#include "../video.hxx"

namespace Alchemist::Media::Encoder::Video {
	class DLL_PUBLIC H265_NVENC final: public Base {
		public:
			H265_NVENC();
			H265_NVENC(const H265_NVENC&)				= default;
			H265_NVENC(H265_NVENC&&)					= default;
			H265_NVENC& operator=(const H265_NVENC&)	= default;
			H265_NVENC& operator=(H265_NVENC&&)		= default;
			~H265_NVENC()							= default;
	};
}