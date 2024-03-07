#include "../audio.hxx"

namespace Alchemist::Media::Codec::Audio {
	class DLL_PUBLIC DTS final: public Base {
		public:
			DTS();
			DTS(const DTS&)					= default;
			DTS(DTS&&)						= default;
			DTS& operator=(const DTS&)		= default;
			DTS& operator=(DTS&&)			= default;
			~DTS()							= default;

			std::list<Decoder::Type> get_available_decoders() const;
			std::list<Encoder::Type> get_available_encoders() const;
	};
}