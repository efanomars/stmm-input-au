/*
 * Copyright © 2020  Stefano Marsili, <stemars@gmx.ch>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, see <http://www.gnu.org/licenses/>
 */
/*
 * File:   main.cc
 */

#include "wanderwindow.h"

#include <stmm-input/event.h>
#include <stmm-input-openal/openaldevicemanager.h>

#include <gtkmm.h>

#include <iostream>
#include <vector>
#include <string>
#include <array>

namespace example
{

namespace wander
{

std::string getPathFromDirAndName(const std::string& sPath, const std::string& sName) noexcept
{
	return sPath + ((sPath == "/") ? "" : "/") + sName;
}

std::vector<std::string> getPathsSounds(std::vector<std::string>&& aSoundsPaths) noexcept
{
	const Glib::ustring sMp3Upper = Glib::ustring{".mp3"}.uppercase();
	const Glib::ustring sWavUpper = Glib::ustring{".wav"}.uppercase();
	const Glib::ustring sOggUpper = Glib::ustring{".ogg"}.uppercase();
	std::array<Glib::ustring, 3> aTypes{sMp3Upper, sWavUpper, sOggUpper};
	std::vector<std::string> aSoundsPathFiles;
	for (const std::string& sSoundPath : aSoundsPaths) {
//std::cout << "sSoundPath=" << sSoundPath << '\n';
		try {
			Glib::Dir oDir(sSoundPath);
			for (const auto& sFileName : oDir) {
				if (Glib::file_test(sFileName, Glib::FILE_TEST_IS_DIR)) {
					continue;
				}
				const Glib::ustring sUtf8FileName =  Glib::filename_to_utf8(sFileName);
				const auto nNameLen = sUtf8FileName.size();
				if (nNameLen <= 4) {
					continue;
				}
//std::cout << "  sUtf8FileName=" << sUtf8FileName << '\n';
				const Glib::ustring sEnd4 = sUtf8FileName.substr(nNameLen - 4, 4);
				if (std::find(aTypes.begin(), aTypes.end(), sEnd4.uppercase()) != aTypes.end()) {
					std::string sSoundPathFile = getPathFromDirAndName(sSoundPath, sFileName);
					aSoundsPathFiles.push_back(std::move(sSoundPathFile));
				}
			}
		} catch (const Glib::FileError& oErr) {
			std::cout << "Couldn't open directory " << sSoundPath << '\n';
			std::cout << "  Error: " << oErr.what() << '\n';
		}
	}
	return aSoundsPathFiles;
}

int wanderAppMain(int nArgc, char** p0Argv)
{
	std::vector<std::string> aSoundsPaths;

	char aAbs[PATH_MAX + 1];
	auto p0Abs = ::getcwd(aAbs, PATH_MAX);
	if (p0Abs == nullptr) {
		std::cerr << ::strerror(errno) << '\n';
		return EXIT_FAILURE; //---------------------------------------------
	}
	aSoundsPaths.push_back(aAbs);

	int32_t nCurArg = 1;
	while (nCurArg < nArgc) {
		if (p0Argv[nCurArg] != nullptr) {
			std::string sPath = p0Argv[nCurArg];
			if (sPath.substr(0,1) != "-") {
				aSoundsPaths.push_back(sPath);
			}
		}
		++nCurArg;
	}

	const Glib::ustring sAppName = "net.exampleappsnirvana.wander";
	const Glib::ustring sWindowTitle = "Show libstmm-input-au events";
	// Also initializes Gtk
	Glib::RefPtr<Gtk::Application> refApp = Gtk::Application::create(sAppName);
	//
	auto aSoundsPathFiles = getPathsSounds(std::move(aSoundsPaths));
	if (aSoundsPathFiles.empty()) {
		std::cout << "Error: No sound files (mp3, wav or ogg) found in the current path or" << '\n';
		std::cout << "       in the absolute paths passed as parameter (if any)." << '\n';
		std::cout << "       Try some subdirectory of '/usr/share/games'." << '\n';
		return EXIT_FAILURE;
	}
	//
	const auto oPairDM = stmi::OpenAlDeviceManager::create("wander", false, std::vector<stmi::Event::Class>{});
	const shared_ptr<stmi::DeviceManager>& refDM = oPairDM.first;
	if (!refDM) {
		std::cout << "Error: Couldn't create device manager" << '\n';
		return EXIT_FAILURE;
	}
	auto refWanderWindow = Glib::RefPtr<WanderWindow>(new WanderWindow(refDM, sWindowTitle, 10, std::move(aSoundsPathFiles)));

	return refApp->run(*(refWanderWindow.operator->()));
}

} // namespace wander

} // namespace example

int main(int argc, char** argv)
{
	return example::wander::wanderAppMain(argc, argv);
}

