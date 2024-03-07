#pragma once

#include "../image.hxx"

namespace Alchemist::Media::Decoder::Image {
	class DLL_PUBLIC WEBP_DEFAULT final: public Base {
		public:
			WEBP_DEFAULT();
			WEBP_DEFAULT(const WEBP_DEFAULT&)				= default;
			WEBP_DEFAULT(WEBP_DEFAULT&&)					= default;
			WEBP_DEFAULT& operator=(const WEBP_DEFAULT&)	= default;
			WEBP_DEFAULT& operator=(WEBP_DEFAULT&&)			= default;
			~WEBP_DEFAULT()									= default;
	};
}