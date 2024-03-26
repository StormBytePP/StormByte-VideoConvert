#pragma once

#include "../container.hxx"

namespace Alchemist::Media::Container {
	class DLL_PUBLIC MKV final: public Base {
		public:
			MKV();
			
			const std::string get_extension() const override;
			std::list<Codec::Type> get_supported_codecs() const override;
	};
}