#include "osdialog_std.h"

#include "osdialog_win.h"

#include <sstream>

#ifdef OS_WIN
#include <ShObjIdl.h>
#include <Windows.h>
#endif

#include <tepp/defer.h>

namespace osdialog
{
	std::string pathToString(std::filesystem::path path) {
#ifdef WIN32
		auto u8Path = osdialog_win_wchar_to_utf8(path.c_str());
		std::string result = u8Path;
		OSDIALOG_FREE(u8Path);
		return result;
#else
		return path.string();
#endif
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
			filterStream << filter.display << " (*." << filter.extension << "):" << filter.extension << ";";
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

#define TRY_OPT(X) \
	if (!(SUCCEEDED(X))) { \
		return std::nullopt; \
	}

	std::optional<std::filesystem::path> openDir(
	    std::filesystem::path const& folder
	) {
#ifdef OS_WIN
		HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
		auto uninitialize = te::defer([] {
			CoUninitialize();
		});
		TRY_OPT(hr);

		IFileDialog* pFileDialog = NULL;
		auto releaseDialog = te::defer([&] {
			if (pFileDialog != NULL) {
				pFileDialog->Release();
			}
		});
		hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pFileDialog));
		TRY_OPT(hr);

		DWORD dwOptions;
		pFileDialog->GetOptions(&dwOptions);
		pFileDialog->SetOptions(dwOptions | FOS_PICKFOLDERS);

		auto currentString = folder.wstring();
		IShellItem* pFolderItem = NULL;
		auto releaseFolder = te::defer([&] {
			if (pFolderItem != NULL) {
				pFolderItem->Release();
			}
		});
		hr = SHCreateItemFromParsingName(currentString.c_str(), NULL, IID_PPV_ARGS(&pFolderItem));
		if (SUCCEEDED(hr) && pFolderItem != NULL) {
			pFileDialog->SetFolder(pFolderItem);
		}

		hr = pFileDialog->Show(NULL);
		TRY_OPT(hr);

		IShellItem* pItem = NULL;
		auto releaseItem = te::defer([&] {
			if (pItem != NULL) {
				pItem->Release();
			}
		});
		hr = pFileDialog->GetResult(&pItem);
		TRY_OPT(hr);

		PWSTR pszFilePath = NULL;
		auto freeString = te::defer([&] {
			if (pszFilePath != NULL) {
				CoTaskMemFree(pszFilePath);
			}
		});
		hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
		TRY_OPT(hr);

		return std::filesystem::path(pszFilePath);

#else
		return osdialog::file(
		    osdialog_file_action::OSDIALOG_OPEN_DIR,
		    folder,
		    {},
		    {},
		    {}
		);
#endif
	}
#undef TRY_OPT
}