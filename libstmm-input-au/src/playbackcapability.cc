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
 * File:   playbackcapability.cc
 */
/*   @DO_NOT_REMOVE_THIS_LINE_IT_IS_USED_BY_COMMONTESTING_CMAKE@   */

#include "playbackcapability.h"

#include <stmm-input/capability.h>

namespace stmi
{

const char* const PlaybackCapability::s_sClassId = "stmi::Playback";
Capability::RegisterClass<PlaybackCapability> PlaybackCapability::s_oInstall(s_sClassId);

PlaybackCapability::SoundData PlaybackCapability::playSound(const std::string& sFileName) noexcept
{
	return playSound(sFileName, 1.0, false, true, 0.0, 0.0, 0.0);
}
PlaybackCapability::SoundData PlaybackCapability::playSound(uint8_t* p0Buffer, int32_t nBufferSize) noexcept
{
	return playSound(p0Buffer, nBufferSize, 1.0, false, true, 0.0, 0.0, 0.0);
}
int32_t PlaybackCapability::playSound(int32_t nFileId) noexcept
{
	return playSound(nFileId, 1.0, false, true, 0.0, 0.0, 0.0);
}

} // namespace stmi
