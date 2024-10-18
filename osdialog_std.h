#include "osdialog.h"

#include <filesystem>
#include <optional>

namespace osdialog
{
	struct filter_file_type
	{
		std::string display{};
		std::string extension{};
	};

	std::optional<std::filesystem::path> file(
	    osdialog_file_action action,
	    std::filesystem::path const& folder,
	    std::optional<std::filesystem::path> const& file,
	    std::optional<std::filesystem::path> const& fileExtension,
	    std::initializer_list<filter_file_type> filters
	);

	std::optional<std::filesystem::path> save(
	    std::filesystem::path const& folder,
	    std::optional<std::filesystem::path> const& file,
	    std::optional<std::filesystem::path> const& fileExtension,
	    std::initializer_list<filter_file_type> filters
	);

	std::optional<std::filesystem::path> open(
	    std::filesystem::path const& folder,
	    std::optional<std::filesystem::path> const& file,
	    std::initializer_list<filter_file_type> filters
	);

	std::optional<std::filesystem::path> openDir(
	    std::filesystem::path const& folder
	);
}