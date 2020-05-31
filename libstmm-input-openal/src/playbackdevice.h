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
 * File:   playbackdevice.h
 */

#ifndef STMI_OPENAL_PLAYBACK_DEVICE_H
#define STMI_OPENAL_PLAYBACK_DEVICE_H

#include "openaldevicemanager.h"

#include "recycler.h"

#include <stmm-input-au/playbackcapability.h>
#include <stmm-input-au/sndfinishedevent.h>

#include <stmm-input-base/basicdevice.h>
#include <stmm-input/capability.h>
#include <stmm-input/device.h>

#include <memory>
#include <string>
#include <vector>
#include <cstdint>

namespace stmi { class Event; }
namespace stmi { namespace Private { namespace OpenAl { class Backend; } } }

namespace stmi
{

namespace Private
{
namespace OpenAl
{


using std::shared_ptr;
using std::weak_ptr;

class PlaybackDevice final : public BasicDevice<OpenAlDeviceManager>, public PlaybackCapability
						, public std::enable_shared_from_this<PlaybackDevice> //, public sigc::trackable
{
public:
	PlaybackDevice(const std::string& sName, const shared_ptr<OpenAlDeviceManager>& refDeviceManager
					, Backend& oBackend, int32_t nBackendDeviceId, bool bIsDefault) noexcept;
	//
	shared_ptr<Capability> getCapability(const Capability::Class& oClass) const noexcept override;
	shared_ptr<Capability> getCapability(int32_t nCapabilityId) const noexcept override;
	std::vector<int32_t> getCapabilities() const noexcept override;
	std::vector<Capability::Class> getCapabilityClasses() const noexcept override;
	//
	shared_ptr<Device> getDevice() const noexcept override;
	//
	inline int32_t getDeviceId() const noexcept { return Device::getId(); }

	int32_t getBackendDeviceId() const noexcept { return m_nBackendDeviceId; }

	void setIsDefault(bool bIsDefault) noexcept;

	int32_t preloadSound(const std::string& sFileName) noexcept override;
	int32_t preloadSound(uint8_t const* p0Buffer, int32_t nBufferSize) noexcept override;
	SoundData playSound(const std::string& sFileName, double fVolume, bool bLoop
						, bool bRelative, double fX, double fY, double fZ) noexcept override;
	SoundData playSound(const uint8_t* p0Buffer, int32_t nBufferSize, double fVolume, bool bLoop
						, bool bRelative, double fX, double fY, double fZ) noexcept override;
	int32_t playSound(int32_t nFileId, double fVolume, bool bLoop
					, bool bRelative, double fX, double fY, double fZ) noexcept override;

	bool setSoundPos(int32_t nSoundId, bool bRelative, double fX, double fY, double fZ) noexcept override;
	bool setSoundVol(int32_t nSoundId, double fVolume) noexcept override;

	bool setListenerPos(double fX, double fY, double fZ) noexcept override;
	bool setListenerVol(double fVolume) noexcept override;
	//
	bool pauseSound(int32_t nSoundId) noexcept override;
	bool resumeSound(int32_t nSoundId) noexcept override;
	bool stopSound(int32_t nSoundId) noexcept override;

	bool pauseDevice() noexcept override;
	bool resumeDevice() noexcept override;
	void stopAllSounds() noexcept override;

	bool isDefaultDevice() noexcept override;

private:
	int32_t preloadSound(const std::string& sFileName, uint8_t const* p0Buffer, int32_t nBufferSize) noexcept;
	int32_t playSound(OpenAlDeviceManager* p0Owner
				, const std::string& sFileName, const uint8_t* p0Buffer, int32_t nBufferSize, int32_t nFileId
				, double fVolume, bool bLoop, bool bRelative, double fX, double fY, double fZ) noexcept;
	//
	friend class stmi::OpenAlDeviceManager;
	void finishDeviceSounds() noexcept;
	void finalizeListener(OpenAlDeviceManager::ListenerData& oListenerData, int64_t nEventTimeUsec) noexcept;
	void removingDevice() noexcept;
	//
	void onSoundFinished(int32_t nSoundId) noexcept;

	void onDeviceError(int32_t nSoundId, int32_t nFileId, std::string&& sError) noexcept;

	void sendSndFinishedEventToListeners(int32_t nSoundId, SndFinishedEvent::FINISHED_TYPE eFinishedType) noexcept;

	void sendSndFinishedEventToListener(const OpenAlDeviceManager::ListenerData& oListenerData
										, int64_t nEventTimeUsec, uint64_t nSoundStartedTimeStamp
										, SndFinishedEvent::FINISHED_TYPE eFinishedType, int32_t nSoundId
										, const shared_ptr<PlaybackCapability>& refCapability
										, int32_t nClassIdxSoundFinishedEvent
										, shared_ptr<Event>& refEvent) noexcept;

private:
	//
	class ReSndFinishedEvent :public SndFinishedEvent
	{
	public:
		ReSndFinishedEvent(int64_t nTimeUsec, const shared_ptr<PlaybackCapability>& refPlaybackCapability
							, FINISHED_TYPE eFinishedType, int32_t nSoundId) noexcept
		: SndFinishedEvent(nTimeUsec, refPlaybackCapability, eFinishedType, nSoundId)
		{
		}
		void reInit(int64_t nTimeUsec, const shared_ptr<PlaybackCapability>& refPlaybackCapability
					, FINISHED_TYPE eFinishedType, int32_t nSoundId) noexcept
		{
			setTimeUsec(nTimeUsec);
			//setAccessor(refAccessor);
			setFinishedType(eFinishedType);
			setSoundId(nSoundId);
			setPlaybackCapability(refPlaybackCapability);
		}
	};
	Private::Recycler<ReSndFinishedEvent, Event> m_oPlaybackSndFinishedEventRecycler;
private:
	Backend& m_oBackend;
	int32_t m_nBackendDeviceId;
	bool m_bIsDefault;

	struct FileNameToId
	{
		std::string m_sFileName;
		int32_t m_nFileId = -1;
	};
	std::vector< FileNameToId > m_aFileNameToIds;
	struct BufferToId
	{
		const uint8_t* m_p0Buffer = nullptr;
		int32_t m_nBufferSize = 0;
		int32_t m_nFileId = -1;
	};
	std::vector< BufferToId > m_aBufferToIds;

	std::vector< int32_t > m_aActiveSoundIds; // Value: The sound id
	std::vector< uint64_t > m_aActiveSoundStarts; // Value: Timestamp the sound was played,   Size: m_aActiveSoundIds.size()

private:
	PlaybackDevice(const PlaybackDevice& oSource) = delete;
	PlaybackDevice& operator=(const PlaybackDevice& oSource) = delete;
};

} // namespace OpenAl
} // namespace Private

} // namespace stmi

#endif /* STMI_OPENAL_PLAYBACK_DEVICE_H */
