#pragma once

#include "../container.hxx"

namespace Alchemist::Media::Container {
	class DLL_PUBLIC MP4 final: public Base {
		public:
			MP4();

			const std::list<Codec::Type> get_supported_codecs() const override;
	};
}