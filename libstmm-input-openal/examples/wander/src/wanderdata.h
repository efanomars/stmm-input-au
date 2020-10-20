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
 * File:   wanderdata.h
 */

#ifndef STMI_WANDER_DATA_H
#define STMI_WANDER_DATA_H

#include <stmm-input-au/playbackcapability.h>

//#include <cassert>
#include <string>
#include <algorithm>
#include <utility>
#include <vector>
#include <memory>

#include <stdint.h>

namespace example
{

namespace wander
{

using std::shared_ptr;

struct Vec3f
{
	double m_fX = 0.0;
	double m_fY = 0.0;
	double m_fZ = 0.0;
};

struct Vec2f
{
	double m_fX = 0.0;
	double m_fY = 0.0;
};

struct Size2f
{
	double m_fW = 0.0;
	double m_fH = 0.0;
};

class WanderData
{
public:
	int32_t m_nCurrentShowDeviceIdx = -1;
	int32_t m_nCurrentSource = 0;

	Vec2f m_oAreaPos;
	Size2f m_oAreaSize;

	std::vector<std::string> m_aSoundPaths; // Absolute file path of the fixed sounds found at startup

	struct SourceData
	{
		Vec3f m_oPos; // The position of the sound

		shared_ptr<stmi::PlaybackCapability> m_refPlayback; // The capability that is playing the sound or null if not playing
		int32_t m_nPlaybackDeviceId = -1; // is -1 exactly when m_refPlayback is null

		int32_t m_nSoundPathIdx = -1; // The chosen sound file. Index into m_aSoundPaths or -1

		int32_t m_nSoundId = -1; // The sound id of the playing sound or -1

		double m_fVol = 1.0;

		bool m_bLooping = false;
		bool m_bListenerRelative = false;

		bool m_bPaused = false;

		bool isPlaying() const noexcept { return (m_nSoundId >= 0); }
	};
	std::vector<SourceData> m_aSourceDatas; // Size: m_nTotSources

	struct CapabilityData
	{
		shared_ptr<stmi::PlaybackCapability> m_refCapability; // The capability: if null, m_nDeviceId is -1
		int32_t m_nDeviceId = -1; // The id of the device the capability belongs to or -1 (which implies m_refCapability is null)
		std::vector<std::pair<std::string, int32_t>> m_aFilePathToId; // The filepaths that have a file id for the device capability

		Vec3f m_oListenerPos;
		//~ Vec3f m_oListenerDir{0.0, 1.0, 0.0};

		double m_fListenerVol = 1.0;

		bool m_bDevicePaused = false;
	};
	std::vector<CapabilityData> m_aCapabilityDatas; // The available playback capabilities (devices)
};

} // namespace wander

} // namespace example

#endif /* STMI_WANDER_DATA_H */
