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
 * File:   sndfinishedevent.h
 */

#ifndef STMI_SND_FINISHED_EVENT_H
#define STMI_SND_FINISHED_EVENT_H

#include <stmm-input/event.h>

#include <cstdint>
#include <memory>

namespace stmi { class Capability; }
namespace stmi { class PlaybackCapability; }

namespace stmi
{

using std::shared_ptr;
using std::weak_ptr;

/** Event generated when sound has finished.
 * Note that the reference to the capability that generated this event is weak.
 */
class SndFinishedEvent : public Event
{
public:
	enum FINISHED_TYPE
	{
		FINISHED_TYPE_FIRST = 0
		, FINISHED_TYPE_COMPLETED = 0 /**< The sound was completed successfully. */
		, FINISHED_TYPE_ABORTED = 1 /**< The sound was aborted (probably device is being removed). */
		, FINISHED_TYPE_LISTENER_REMOVED = 2 /**< The listener receiving this event is being removed.
												* The sound might actually still be playing. */
		, FINISHED_TYPE_FILE_NOT_FOUND = 3 /**< The sound couldn't be started because file not found. */
		, FINISHED_TYPE_LAST = 3
	};
	/** Constructor.
	 * @param nTimeUsec Time from epoch in microseconds.
	 * @param refPlaybackCapability The capability that generated this event. Cannot be null.
	 * @param eFinishedType The type.
	 * @param nSoundId The Sound id that finished playing.
	 */
	SndFinishedEvent(int64_t nTimeUsec, const shared_ptr<PlaybackCapability>& refPlaybackCapability
					, FINISHED_TYPE eFinishedType, int32_t nSoundId) noexcept;
	/** The playback capability that generated the event.
	 * @return The capability or null if the capability was deleted.
	 */
	inline shared_ptr<PlaybackCapability> getPlaybackCapability() const noexcept { return m_refPlaybackCapability.lock(); }
	//
	shared_ptr<Capability> getCapability() const noexcept override;
	/** The type of finish.
	 * @return The type.
	 */
	FINISHED_TYPE getFinishedType() const noexcept { return m_eFinishedType; }
	/** The sound id being finished.
	 * @return The sound id.
	 */
	int32_t getSoundId() const noexcept { return m_nSoundId; }

	//
	static const char* const s_sClassId;
	static const Event::Class& getClass() noexcept
	{
		static const Event::Class s_oSndFinishedClass = s_oInstall.getEventClass();
		return s_oSndFinishedClass;
	}
protected:
	/** Set the finished type.
	 * @param eFinishedType The type.
	 */
	void setFinishedType(FINISHED_TYPE eFinishedType) noexcept;
	/** Sets sound id.
	 * @param nSoundId The sound id.  Must be &gt;= 0.
	 */
	void setSoundId(int32_t nSoundId) noexcept;
	/** Sets the capability.
	 * @param refPlaybackCapability The capability that generated this event. Cannot be null.
	 */
	void setPlaybackCapability(const shared_ptr<PlaybackCapability>& refPlaybackCapability) noexcept;
private:
	FINISHED_TYPE m_eFinishedType;
	int32_t m_nSoundId;
	weak_ptr<PlaybackCapability> m_refPlaybackCapability;
	//
	static RegisterClass<SndFinishedEvent> s_oInstall;
private:
	SndFinishedEvent() = delete;
};

} // namespace stmi

#endif /* STMI_SND_FINISHED_EVENT_H */

