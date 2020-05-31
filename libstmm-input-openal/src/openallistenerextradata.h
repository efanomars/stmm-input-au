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
 * File:   openallistenerextradata.h
 */

#ifndef STMI_OPENAL_LISTENER_EXTRA_DATA_H
#define STMI_OPENAL_LISTENER_EXTRA_DATA_H

#include "openaldevicemanager.h"

#include <vector>
#include <algorithm>

namespace stmi
{

namespace Private
{
namespace OpenAl
{

class OpenAlListenerExtraData final : public stmi::OpenAlDeviceManager::ListenerExtraData
{
public:
	void reset() noexcept override
	{
		m_aFinishedSounds.clear();
	}
	inline bool isSoundFinished(int32_t nSoundId) const noexcept
	{
		return std::find(m_aFinishedSounds.begin(), m_aFinishedSounds.end(), nSoundId) != m_aFinishedSounds.end();
	}
	inline void setSoundFinished(int32_t nSoundId) noexcept
	{
		m_aFinishedSounds.push_back(nSoundId);
	}
private:
	std::vector<int32_t> m_aFinishedSounds;
};

} // namespace OpenAl
} // namespace Private

} // namespace stmi

#endif /* STMI_OPENAL_LISTENER_EXTRA_DATA_H */
