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
 * File:   openalbackend.h
 */

#ifndef STMI_OPENAL_BACKEND_H
#define STMI_OPENAL_BACKEND_H

#include <sigc++/connection.h>

#include <memory>
#include <string>
#include <atomic>
#include <thread>
#include <deque>
#include <vector>
#include <utility>
#include <mutex>
#include <condition_variable>

#include <AL/alure.h>

#include <stdint.h>

namespace stmi
{

class OpenAlDeviceManager;

namespace Private
{
namespace OpenAl
{

using std::shared_ptr;
using std::unique_ptr;
using std::weak_ptr;

void openalSoundFinishedCallback(void *p0AlEvent, ALuint nSourceId) noexcept;


////////////////////////////////////////////////////////////////////////////////
class Backend //: public sigc::trackable
{
public:
	// returns backend
	static unique_ptr<Backend> create(::stmi::OpenAlDeviceManager* p0Owner) noexcept;

	// return empty if ok error otherwise
	// This has to be called when the OpenAlDeviceManager is ready to receive callbacks
	std::string createThread() noexcept;

	#ifdef STMI_TESTING_IFACE
	virtual
	#endif
	~Backend() noexcept;

	enum AL_COMMAND_TYPE
	{
		AL_COMMAND_FIRST           = 0
		, AL_COMMAND_PRELOAD       = 0
		, AL_COMMAND_PLAY          = 1
		, AL_COMMAND_PAUSE         = 2
		, AL_COMMAND_RESUME        = 3
		, AL_COMMAND_STOP          = 4
		, AL_COMMAND_PAUSE_DEVICE  = 5
		, AL_COMMAND_RESUME_DEVICE = 6
		, AL_COMMAND_STOP_ALL      = 7
		, AL_COMMAND_SOUND_POS     = 8
		, AL_COMMAND_SOUND_VOL     = 9
		, AL_COMMAND_LISTENER_POS  = 10
		, AL_COMMAND_LISTENER_VOL  = 11
		, AL_COMMAND_LAST          = 11
	};
	struct AlCommand
	{
		int32_t m_nBackendDeviceId;
		AL_COMMAND_TYPE m_eType;
		int32_t m_nSoundId = -1;
		int32_t m_nFileId = -1;
		std::string m_sFileName;
		const uint8_t* m_p0Buffer = nullptr;
		int32_t m_nBufferSize = 0;
		bool m_bLoop = false; /*< Whether sound is looping */
		bool m_bRelative = false; /*< Whether relative to listener. Default is false. */
		double m_fPosX = 0; /*< Used for setting the x position or x direction */
		double m_fPosY = 0; /*< Used for setting the y position or x direction */
		double m_fPosZ = 0; /*< Used for setting the z position or x direction */
		double m_fVolume = 1.0; /*< The volume. Default is 1.0. */
	};

	enum AL_EVENT_TYPE
	{
		AL_EVENT_INVALID          = -1
		, AL_EVENT_FIRST          = 0
		, AL_EVENT_PLAY_FINISHED  = 0
		, AL_EVENT_DEVICE_ADDED   = 1
		, AL_EVENT_DEVICE_REMOVED = 2
		, AL_EVENT_DEVICE_CHANGED = 3 /**< Either the device has become default or no longer is default. */
		, AL_EVENT_PLAY_ERROR     = 4
		, AL_EVENT_LAST           = 4
	};
	struct AlEvent
	{
		std::string m_sDeviceName;
		AL_EVENT_TYPE m_eType = AL_EVENT_INVALID;
		bool m_bDeviceIsDefault = false;
		std::string m_sError;
		int32_t m_nBackendDeviceId = -1;
		int32_t m_nSoundId = -1;
		int32_t m_nFileId = -1;
	private:
		friend class Backend;
		friend void Private::OpenAl::openalSoundFinishedCallback(void *p0AlEvent, ALuint /*nSourceId*/) noexcept;
		Backend* m_p0Backend = nullptr;
	};

	//ConcurrentQueue<AlCommand, true>& getAlCommandQueue() noexcept { return m_oAlCommandQueue; }

	void sendCommand(AlCommand&& oAlCommand) noexcept;
protected:
	explicit Backend(::stmi::OpenAlDeviceManager* p0Owner) noexcept;

private:
	struct ActiveSound
	{
		int32_t m_nSoundId;
		ALuint m_nALSourceId;
		bool m_bPaused = false;
		bool m_bStartedWhenDevicePaused = false;
	};
	struct AlDevice
	{
		std::string m_sDeviceName;
		ALCdevice* m_pDevice = nullptr;
		ALCcontext* m_pContext = nullptr;
		std::vector<std::pair<int32_t, ALuint>> m_aFileToBufferId;
		std::vector<ActiveSound> m_aActiveSounds;
		std::vector<ALuint> m_aUnusedSourceIds;
		bool m_bDevicePaused = false;
		bool m_bDeviceRemoved = false;
	};
private:
	// In general all methods starting with openalXXX()
	// are only executed by the OpenAL thread.
	void openalThreadRun() noexcept;
	void openalExecCommand(const AlCommand& oCommand) noexcept;
	void openalPreload(const AlCommand& oCommand) noexcept;
	void openalPlay(const AlCommand& oCommand) noexcept;
	void openalPause(const AlCommand& oCommand) noexcept;
	void openalResume(const AlCommand& oCommand) noexcept;
	void openalStop(const AlCommand& oCommand) noexcept;
	void openalPauseDevice(const AlCommand& oCommand) noexcept;
	void openalResumeDevice(const AlCommand& oCommand) noexcept;
	void openalStopAll(const AlCommand& oCommand) noexcept;
	void openalSoundPos(const AlCommand& oCommand) noexcept;
	void openalSoundVol(const AlCommand& oCommand) noexcept;
	void openalListenerPos(const AlCommand& oCommand) noexcept;
	//void openalListenerDir(const AlCommand& oCommand) noexcept;
	void openalListenerVol(const AlCommand& oCommand) noexcept;

	std::string openalGetDeviceNames(std::vector<std::string>& aDeviceNames, int32_t& nDefaultIdx) noexcept;
	// return device is or -1 if failed
	int32_t openalCreateDevice(const std::string& sDeviceName) noexcept;
	std::string openalCreateAllDevices(bool bUseDeviceNames, std::vector<std::string>& aDeviceNames, bool bSendEvent) noexcept;
	void openalRemoveAllDevices() noexcept;
	void openalShutdownDevice(AlDevice& oDev) noexcept;

	AlDevice& getActiveDevice(int32_t nDeviceId) noexcept;
	std::vector<ActiveSound>::iterator getActiveSoundIt(int32_t nSoundId, AlDevice& oAlDevice) noexcept;
	// returns null if sound was removed in the mean time
	ActiveSound* getActiveSound(const AlCommand& oCommand, AlDevice& oAlDevice) noexcept;
	void removeActiveSound(std::vector<ActiveSound>& aActiveSounds, std::vector<ActiveSound>::iterator itActiveSound) noexcept;
	AlDevice& getOrCreateAlDevice(int32_t& nDeviceId) noexcept;
	AlEvent& getOrCreateAlEvent() noexcept;
	AlEvent& getSoundFinishedAlEvent(const AlCommand& oCommand) noexcept;
	void sendDeviceChangedAlEvent(int32_t nDeviceId) noexcept;
	void sendDeviceRemovedAlEvent(int32_t nDeviceId) noexcept;
	void sendDeviceAddedAlEvent(int32_t nDeviceId, const std::string& sDeviceName) noexcept;

	void openalCheckDeviceNames() noexcept;

	void openalSendError(const std::string& sErr, const AlCommand& oCommand) noexcept;

	friend void Private::OpenAl::openalSoundFinishedCallback(void *p0AlEvent, ALuint /*nSourceId*/) noexcept;

	bool onCheckEventsTimeout() noexcept;

private:
	OpenAlDeviceManager* m_p0Owner;

	std::thread m_oAlThread;

	// Set by m_oAlThread when it has initially filled the m_aDeviceNames field,
	// it's also set if an error occurred (OpenAL doesn't support device enumeration)
	std::atomic<bool> m_bInitialDevicesReady = ATOMIC_VAR_INIT(false);
	std::condition_variable m_oInitialDevicesReady;
	// This is set by the main thread when it has created the devices from the
	// names provided by m_oAlThread in m_aDeviceNames
	// It is also set if m_sInitialAlError is not empty to give m_oAlThread a chance
	// to stop and join
	std::atomic<bool> m_bInitialDevicesCreated = ATOMIC_VAR_INIT(false);
	std::condition_variable m_oInitialDevicesCreated;

	// When false tells m_oAlThread to stop and join
	std::atomic<bool> m_bIsRunning = ATOMIC_VAR_INIT(true);

	std::mutex m_oAlCommandMutex;
	std::condition_variable m_oAlCommandsNotEmpty;

	std::mutex m_oAlEventMutex;

	// only accessed under m_oAlCommandMutex
	std::vector<AlCommand> m_aAlCommands;
	// only accessed under m_oAlEventMutex
	std::vector<AlEvent> m_aAlEvents;

	// The m_aAlDevices field is only for the m_oAlThread except
	// when m_bInitialDevicesReady is set to true and m_bInitialDevicesCreated is still false
	// during initialization handshake.
	// The nDeviceId variables in the code are indexes into this vector.
	// Removed devices are not erased from the vector but marked as removed
	// so that device ids are not invalidated.
	// Added devices recycle marked as removed entries (and therefore device ids).
	std::vector<AlDevice> m_aAlDevices;
	// Index into m_aAlDevices. Is -1 if no active devices.
	// Only used by m_oAlThread thread!
	int32_t m_nDefaultDeviceId = -1;
	// The current number of active devices in m_aAlDevices.
	// Only used by m_oAlThread thread!
	int32_t m_nTotAlDevices = 0;
	// The m_sInitialAlError is set by m_oAlThread and should be read by the main thread
	// when it is allowed to read m_aDeviceNames.
	// It tells whether there was an OpenAL error when retrieving the initial devices.
	std::string m_sInitialAlError;

	// When a sound is played an entry is created (or recycled in this deque)
	// and it's address is passed to the finished event callback
	// where the entry will be put (moved) into m_aAlEvents queue and
	// set to AL_EVENT_INVALID to be recycled
	// Only used by m_oAlThread thread!
	std::deque<AlEvent> m_aToFinishAlEvents;

	sigc::connection m_oCheckEventsConn;

	// Used by the main thread to avoid reallocating at each timeout
	std::vector<AlEvent> m_aReadAlEvents;
	// Used by openAL thread to avoid reallocating
	std::vector<AlCommand> m_aReadAlCommands;
private:
	Backend() = delete;
	Backend(const Backend& oSource) = delete;
	Backend& operator=(const Backend& oSource) = delete;
};

} // namespace OpenAl
} // namespace Private

} // namespace stmi

#endif /* STMI_OPENAL_BACKEND_H */
