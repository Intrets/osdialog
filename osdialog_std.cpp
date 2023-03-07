#include "osdialog_std.h"

#include "osdialog_win.h"

namespace osdialog
{
	std::string pathToString(std::filesystem::path path) {
		auto u8Path = osdialog_win_wchar_to_utf8(path.c_str());
		std::string result = u8Path;
		OSDIALOG_FREE(u8Path);
		return result;
	}

	std::optional<std::filesystem::path> file(
	    osdialog_file_action action,
	    std::filesystem::path const& folderPath,
	    std::optional<std::filesystem::path> const& filePath,
	    std::optional<std::filesystem::path> const& fileExtension,
	    std::initializer_list<filter_file_type> filterTypes
	) {
		std::string folder = pathToString(folderPath);
		std::string file = pathToString(filePath.value_or(""));

		auto folderPtr = folder.empty() ? nullptr : folder.c_str();
		auto filePtr = file.empty() ? nullptr : file.c_str();

		std::stringstream filterStream{};

		for (auto& filter : filterTypes) {
			filterStream << filter.display << " (*." << filter.extension << "):" << filter.extension << "; ";
		}
		filterStream << "All Files (*.*):*";

		auto filter = filterStream.str();

		osdialog_filters* filters = osdialog_filters_parse(filter.c_str());
		auto result = osdialog_file(action, folderPtr, filePtr, filters);
		osdialog_filters_free(filters);

		if (result != nullptr) {
			std::filesystem::path path = result;
			OSDIALOG_FREE(result);

			if (!path.has_extension() && fileExtension.has_value() && !fileExtension->empty()) {
				path += ".";
				path += fileExtension.value();
			}

			return path;
		}
		else {
			return std::nullopt;
		}
	}

	std::optional<std::filesystem::path> save(
	    std::filesystem::path const& folder,
	    std::optional<std::filesystem::path> const& file,
	    std::optional<std::filesystem::path> const& fileExtension,
	    std::initializer_list<filter_file_type> filters
	) {
		return osdialog::file(
		    osdialog_file_action::OSDIALOG_SAVE,
		    folder,
		    file,
		    fileExtension,
		    filters
		);
	}

	std::optional<std::filesystem::path> open(
	    std::filesystem::path const& folder,
	    std::optional<std::filesystem::path> const& file,
	    std::initializer_list<filter_file_type> filters
	) {
		return osdialog::file(
		    osdialog_file_action::OSDIALOG_OPEN,
		    folder,
		    file,
		    {},
		    filters
		);
	}
}