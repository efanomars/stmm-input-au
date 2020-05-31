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
 * File:   sndfinishedevent.cc
 */
/*   @DO_NOT_REMOVE_THIS_LINE_IT_IS_USED_BY_COMMONTESTING_CMAKE@   */

#include "sndfinishedevent.h"

#include "playbackcapability.h"

#include <stmm-input/event.h>

#include <memory>
#include <cassert>

namespace stmi { class Accessor; }

namespace stmi
{

const char* const SndFinishedEvent::s_sClassId = "stmi::Playback:SndFinishedEvent";
Event::RegisterClass<SndFinishedEvent> SndFinishedEvent::s_oInstall(s_sClassId);

SndFinishedEvent::SndFinishedEvent(int64_t nTimeUsec, const shared_ptr<PlaybackCapability>& refPlaybackCapability
									, FINISHED_TYPE eFinishedType, int32_t nSoundId) noexcept
: Event(s_oInstall.getEventClass(), nTimeUsec, (refPlaybackCapability ? refPlaybackCapability->getId() : -1)
			, shared_ptr<Accessor>{})
, m_eFinishedType(eFinishedType)
, m_nSoundId(nSoundId)
, m_refPlaybackCapability(refPlaybackCapability)
{
	assert(nSoundId >= 0);
}

shared_ptr<Capability> SndFinishedEvent::getCapability() const noexcept
{
	return m_refPlaybackCapability.lock();
}

void SndFinishedEvent::setFinishedType(FINISHED_TYPE eFinishedType) noexcept
{
	assert((eFinishedType >= FINISHED_TYPE_FIRST) && (eFinishedType <= FINISHED_TYPE_LAST));
	m_eFinishedType = eFinishedType;
}
void SndFinishedEvent::setSoundId(int32_t nSoundId) noexcept
{
	assert(nSoundId >= 0);
	m_nSoundId = nSoundId;
}
void SndFinishedEvent::setPlaybackCapability(const shared_ptr<PlaybackCapability>& refPlaybackCapability) noexcept
{
	assert(refPlaybackCapability);
	setCapabilityId(refPlaybackCapability->getId());
	m_refPlaybackCapability = refPlaybackCapability;
}

} // namespace stmi

