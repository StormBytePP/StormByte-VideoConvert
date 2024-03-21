#pragma once

#include "../container.hxx"

namespace Alchemist::Media::Container {
	class DLL_PUBLIC AVI final: public Base {
		public:
			AVI();

			std::list<Codec::Type> get_supported_codecs() const override;
	};
}