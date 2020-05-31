/*
 * Copyright © 2020  Stefano Marsili, <stemars@gmx.ch>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <http://www.gnu.org/licenses/>
 */
/*
 * File:   sndmgmtcapability.h
 */

#ifndef STMI_SnD_MGMT_CAPABILITY_H
#define STMI_SnD_MGMT_CAPABILITY_H

//#include "playbackcapability.h"

#include <stmm-input/capability.h>

#include <vector>
#include <memory>
#include <type_traits>
#include <cassert>

namespace stmi { class PlaybackCapability; }

namespace stmi { class DeviceManager; }

namespace stmi
{

using std::shared_ptr;

/** Sound device manager capability.
 * Gives information about capabilities of playback devices.
 */
class SndMgmtCapability : public DeviceManagerCapability
{
public:
	/** The maximum number of playbacks the device manager supports.
	 * This is usually 1 or std::numeric_limits<int32_t>::max(), but
	 * returning 1000 to mean infinite is also valid.
	 *
	 * Example: this is used by the stmm-games library to determine
	 * whether to allow per player playback capability assignment.
	 * @return The maximum number of devices with PlaybackCapability.
	 */
	virtual int32_t getMaxPlaybackDevices() const noexcept = 0;
	/** The default playback capability of the device manager.
	 * @return The default playback or null if unknown or there are no playback devices.
	 */
	virtual shared_ptr<PlaybackCapability> getDefaultPlayback() const noexcept = 0;
	/** Whether the capabilities of the devices of the device manager support sound positioning.
	 * If this method returns false the position parameters of PlaybackCapability::playSound(),
	 * PlaybackCapability::setSoundPos(), PlaybackCapability::setListenerPos() are ignored.
	 * @return Whether sound space is supported.
	 */
	virtual bool supportsSpatialSounds() const noexcept = 0;

	static const char* const s_sClassId;
	static const Capability::Class& getClass() noexcept { return s_oInstall.getCapabilityClass(); }
protected:
	SndMgmtCapability() noexcept;
private:
	static RegisterClass<SndMgmtCapability> s_oInstall;
private:
};

} // namespace stmi

#endif /* STMI_SnD_MGMT_CAPABILITY_H */
