#pragma once

#include "item.hxx"

#include <filesystem>

namespace Alchemist::Media {
	class DLL_PUBLIC File: public Item {
		public:
			File(const File&)					= default;
			File(File&&)						= default;
			File& operator=(const File&)		= default;
			File& operator=(File&&)				= default;
			~File()								= default;

			static File read(const std::filesystem::path&);
			static File read(std::filesystem::path&&);

		private:
			File(const std::filesystem::path&);
			File(std::filesystem::path&&);
			File& init();

			std::filesystem::path m_media_path;
			unsigned int m_features:4;
	};
}