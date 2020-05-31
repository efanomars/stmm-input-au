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
 * File:   openalbackend.cc
 */

#include "openalbackend.h"

#include "openaldevicemanager.h"

#include <glibmm.h>

#include <string>
#include <memory>
#include <cassert>
#include <algorithm>
#include <cstdint>
#include <iostream>
#include <iterator>
#include <chrono>
#include <mutex>
#include <condition_variable>
#include <limits>

#include <AL/alure.h>


namespace stmi
{

namespace Private
{
namespace OpenAl
{

// Main thread
static constexpr const int32_t s_nCheckEventsConnMillisec = 200;

// OpenAl thread
static constexpr const int32_t s_nBaseIntervalMillisec = 10;
static constexpr const double s_fAlUpdateIntervalSeconds = 0.12;
static constexpr const double s_fAlCheckDevicesIntervalSeconds = 1.0;

unique_ptr<Backend> Backend::create(::stmi::OpenAlDeviceManager* p0Owner) noexcept
{
	return std::unique_ptr<Backend>(new Backend(p0Owner));
}

Backend::Backend(::stmi::OpenAlDeviceManager* p0Owner) noexcept
: m_p0Owner(p0Owner)
{
	assert(p0Owner != nullptr);
}
std::string Backend::openalGetDeviceNames(std::vector<std::string>& aDeviceNames, int32_t& nDefaultIdx) noexcept
{
	ALCsizei nTotNames;
	ALCchar const ** pDeviceNames = ::alureGetDeviceNames(true, &nTotNames);
	if (pDeviceNames == nullptr) {
		return std::string{::alureGetErrorString()}; //-------------------------
	}
	const int32_t nTotIdxs = static_cast<int32_t>(nTotNames);
	for (int32_t nIdx = 0; nIdx < nTotIdxs; ++nIdx) {
		aDeviceNames.push_back(pDeviceNames[nIdx]);
	}
	::alureFreeDeviceNames(pDeviceNames);

	if (aDeviceNames.empty()) {
		nDefaultIdx = -1;
		return ""; //-----------------------------------------------------------
	}
	ALCchar const * p0DefaultDeviceName = ::alcGetString(nullptr, ALC_DEFAULT_ALL_DEVICES_SPECIFIER);
	const std::string sDefaultDeviceName{p0DefaultDeviceName};
	const auto itFind = std::find(aDeviceNames.begin(), aDeviceNames.end(), sDefaultDeviceName);
	if (itFind == aDeviceNames.end()) {
		nDefaultIdx = 0;
	} else {
		nDefaultIdx = std::distance(aDeviceNames.begin(), itFind);
	}
	return "";
}
std::string Backend::createThread() noexcept
{
//std::cout << "Backend::createThread  startin thread" << '\n';
	m_oAlThread = std::thread([&]()
	{
		// creates openal devices
		std::vector<std::string> aDummyDeviceNames;
		const std::string sErr = openalCreateAllDevices(false, aDummyDeviceNames, false);
		if (! sErr.empty()) {
			m_sInitialAlError = "alureGetDeviceNames() error: " + sErr;
		}
		// signal main thread to create stmm-input devices and capabilities
		{
			std::lock_guard<std::mutex> oLock(m_oAlCommandMutex);
			m_bInitialDevicesReady = true;
		}
		m_oInitialDevicesReady.notify_one();
		if (! sErr.empty()) {
			return; //----------------------------------------------------------
		}
		// Wait for main thread to create devices
		{
			std::unique_lock<std::mutex> oLock(m_oAlCommandMutex);
			m_oInitialDevicesCreated.wait(oLock, [&](){ return (m_bInitialDevicesCreated != false); });		
		}
		//
//std::cout << "Backend:: openalThread  thread running" << '\n';
		openalThreadRun();
		// shut down everything
//std::cout << "Backend:: openalThread  shutting down" << '\n';
		for (AlDevice& oDev : m_aAlDevices) {
			if (oDev.m_bDeviceRemoved) {
				continue;
			}
			openalShutdownDevice(oDev);
		}
		// now thread is ready to join in the destructor
	});
	// Wait for m_oAlThread to set device names
	{
		std::unique_lock<std::mutex> oLock(m_oAlCommandMutex);
		m_oInitialDevicesReady.wait(oLock, [&](){ return (m_bInitialDevicesReady != false); });
	}

	if (! m_sInitialAlError.empty()) {
		const std::string sTemp = m_sInitialAlError;
		// allow m_oAlThread to join when destructor of this class is called
		{
			std::lock_guard<std::mutex> oLock(m_oAlCommandMutex);
			m_bInitialDevicesCreated = true;
		}
		m_oInitialDevicesCreated.notify_one();
		return sTemp; //--------------------------------------------------------
	}
	// Create corresponding stmm-input devices in device manager
	const int32_t nTotIdxs = static_cast<int32_t>(m_aAlDevices.size());
	for (int32_t nDeviceId = 0; nDeviceId < nTotIdxs ; ++nDeviceId) {
		AlDevice& oDev = m_aAlDevices[nDeviceId];
		if (oDev.m_bDeviceRemoved) {
			continue;
		}
		auto sCopy = oDev.m_sDeviceName;
		m_p0Owner->onDeviceAdded(std::move(sCopy), nDeviceId, (nDeviceId == m_nDefaultDeviceId));
	}
	// now signal to the thread it can start processing commands, updating, checking devices etc.
	{
		std::lock_guard<std::mutex> oLock(m_oAlCommandMutex);
		m_bInitialDevicesCreated = true;
	}
	m_oInitialDevicesCreated.notify_one();

	m_oCheckEventsConn = Glib::signal_timeout().connect(
			sigc::mem_fun(*this, &Backend::onCheckEventsTimeout), s_nCheckEventsConnMillisec);

	return "";
}
Backend::~Backend() noexcept
{
//std::cout << "Backend:: destructor  start shutdown" << '\n';
	// Tell m_oAlThread to stop
	m_bIsRunning = false;
	m_oAlThread.join();
//std::cout << "Backend:: destructor  threadjoined" << '\n';
}
bool Backend::onCheckEventsTimeout() noexcept
{
	assert(m_aReadAlEvents.empty());
	{
		std::lock_guard<std::mutex> oLock(m_oAlEventMutex);
		m_aReadAlEvents = std::move(m_aAlEvents);
		m_aAlEvents.clear();
	}
	if (m_aReadAlEvents.empty()) {
		return true; //---------------------------------------------------------
	}
//std::cout << "Backend::onCheckEventsTimeout() tot AL_EVENTs = " << m_aReadAlEvents.size() << '\n';
	for (AlEvent& oAlEvent : m_aReadAlEvents) {
//std::cout << "Backend::onCheckEventsTimeout() oAlEvent.m_nBackendDeviceId = " << oAlEvent.m_nBackendDeviceId << '\n';
//std::cout << "Backend::onCheckEventsTimeout()         .m_nFileId   = " << oAlEvent.m_nFileId << '\n';
//std::cout << "Backend::onCheckEventsTimeout()         .m_nSoundId  = " << oAlEvent.m_nSoundId << '\n';
//std::cout << "Backend::onCheckEventsTimeout()         .m_eType     = " << static_cast<int32_t>(oAlEvent.m_eType) << '\n';
		switch (oAlEvent.m_eType) {
		case AL_EVENT_PLAY_FINISHED:
		{
			m_p0Owner->onPlayFinished(oAlEvent.m_nBackendDeviceId, oAlEvent.m_nSoundId);
		} break;
		case AL_EVENT_DEVICE_ADDED:
		{
			m_p0Owner->onDeviceAdded(std::move(oAlEvent.m_sDeviceName), oAlEvent.m_nBackendDeviceId, oAlEvent.m_bDeviceIsDefault);
		} break;
		case AL_EVENT_DEVICE_REMOVED:
		{
			m_p0Owner->onDeviceRemoved(oAlEvent.m_nBackendDeviceId);
		} break;
		case AL_EVENT_DEVICE_CHANGED:
		{
			m_p0Owner->onDeviceChanged(oAlEvent.m_nBackendDeviceId, oAlEvent.m_bDeviceIsDefault);
		} break;
		case AL_EVENT_PLAY_ERROR:
		{
			m_p0Owner->onDeviceError(oAlEvent.m_nBackendDeviceId, oAlEvent.m_nFileId, oAlEvent.m_nSoundId, std::move(oAlEvent.m_sError));
		} break;
		default:
		{
			assert(false);
		} break;
		}
	}
	m_aReadAlEvents.clear();
	return true;
}
void Backend::openalThreadRun() noexcept
{
	auto oLastCheckUpdate = std::chrono::steady_clock::now();
	auto oLastCheckDevices = oLastCheckUpdate + std::chrono::milliseconds(73);
	//
	std::unique_lock<std::mutex> oLock(m_oAlCommandMutex);
	auto oExecCommands = [&]()
	{
		m_aReadAlCommands = std::move(m_aAlCommands);
		m_aAlCommands.clear();
		oLock.unlock();
		for (auto& oCommand : m_aReadAlCommands) {
			openalExecCommand(oCommand);
		}
		oLock.lock();
	};

	do {
		m_oAlCommandsNotEmpty.wait_for(oLock, std::chrono::milliseconds(s_nBaseIntervalMillisec)
										, [&]{ return (! m_aAlCommands.empty()) || ! m_bIsRunning; });		
		if (! m_bIsRunning) {
			break;
		}
		auto oNow = std::chrono::steady_clock::now();
		// each 125 millisec alUpdate (check for finished songs)
		std::chrono::duration<double> fDiffUpdate = oNow - oLastCheckUpdate;
		const bool bDoUpdateSounds = (fDiffUpdate > std::chrono::duration<double>(s_fAlUpdateIntervalSeconds));
		// each 1000 millisec check device names
		std::chrono::duration<double> fDiffCheckDevices = oNow - oLastCheckDevices;
		const bool bDoUpdateDevices = (fDiffCheckDevices > std::chrono::duration<double>(s_fAlCheckDevicesIntervalSeconds));

		do {
			oExecCommands();
		} while (! m_aAlCommands.empty());

		bool bIsUnlocked = false;
		if (bDoUpdateSounds) {
			oLock.unlock();
			bIsUnlocked = true;
			::alureUpdate();
			oLastCheckUpdate = oNow;
		}
		if (bDoUpdateDevices) {
			if (! bIsUnlocked) {
				oLock.unlock();
				bIsUnlocked = true;
			}
			openalCheckDeviceNames();
			oLastCheckDevices = oNow;
		}
		if (bIsUnlocked) {
			oLock.lock();
			// If while unlocked commands came in, execute them so that
			// the wait can't miss a notify_one() from the main thread
			while (! m_aAlCommands.empty()) {
				oExecCommands();
			}
		}
	} while (true);
}
void Backend::sendCommand(AlCommand&& oAlCommand) noexcept
{
	{
		std::lock_guard<std::mutex> oLock(m_oAlCommandMutex);
		m_aAlCommands.push_back(std::move(oAlCommand));
	}
	m_oAlCommandsNotEmpty.notify_one();
}
void Backend::openalExecCommand(const AlCommand& oCommand) noexcept
{
	switch (oCommand.m_eType) {
		case AL_COMMAND_PRELOAD:
		{
			openalPreload(oCommand);
		} break;
		case AL_COMMAND_PLAY:
		{
			openalPlay(oCommand);
		} break;
		case AL_COMMAND_PAUSE:
		{
			openalPause(oCommand);
		} break;
		case AL_COMMAND_RESUME:
		{
			openalResume(oCommand);
		} break;
		case AL_COMMAND_STOP:
		{
			openalStop(oCommand);
		} break;
		case AL_COMMAND_PAUSE_DEVICE:
		{
			openalPauseDevice(oCommand);
		} break;
		case AL_COMMAND_RESUME_DEVICE:
		{
			openalResumeDevice(oCommand);
		} break;
		case AL_COMMAND_STOP_ALL:
		{
			openalStopAll(oCommand);
		} break;
		case AL_COMMAND_SOUND_POS:
		{
			openalSoundPos(oCommand);
		} break;
		case AL_COMMAND_SOUND_VOL:
		{
			openalSoundVol(oCommand);
		} break;
		case AL_COMMAND_LISTENER_POS:
		{
			openalListenerPos(oCommand);
		} break;
		//case AL_COMMAND_LISTENER_DIR:
		//{
		//	openalListenerDir(oCommand);
		//} break;
		case AL_COMMAND_LISTENER_VOL:
		{
			openalListenerVol(oCommand);
		} break;
		default:
		{
			assert(false);
		}
	}
}
inline double clamp(double fValue) noexcept
{
	if (fValue > std::numeric_limits<ALfloat>::max()) {
		return std::numeric_limits<ALfloat>::max();
	} else if (fValue < std::numeric_limits<ALfloat>::lowest()) {
		return std::numeric_limits<ALfloat>::lowest();
	}
	return fValue;
}
void Backend::openalSendError(const std::string& sErr, const AlCommand& oCommand) noexcept
{
	AlEvent oEv;
	oEv.m_eType = AL_EVENT_PLAY_ERROR;
	oEv.m_nBackendDeviceId = oCommand.m_nBackendDeviceId;
	oEv.m_nSoundId = oCommand.m_nSoundId;
	oEv.m_nFileId = oCommand.m_nFileId;
	oEv.m_sError = sErr;
	std::lock_guard<std::mutex> oLock(m_oAlEventMutex);
	m_aAlEvents.push_back(std::move(oEv));
}
void Backend::openalPreload(const AlCommand& oCommand) noexcept
{
	AlDevice& oAlDevice = getActiveDevice(oCommand.m_nBackendDeviceId);
	// get or create buffer
	ALuint nALBuffer;
	auto& aFileToBufferId = oAlDevice.m_aFileToBufferId;
	if (oCommand.m_p0Buffer == nullptr) {
		nALBuffer = ::alureCreateBufferFromFile(oCommand.m_sFileName.c_str());
		if (nALBuffer == AL_FALSE) {
			openalSendError(::alureGetErrorString(), oCommand);
			return; //------------------------------------------------------
		}
	} else {
		nALBuffer = ::alureCreateBufferFromMemory(static_cast<ALubyte*>(const_cast<uint8_t*>(oCommand.m_p0Buffer))
												, static_cast<ALsizei>(oCommand.m_nBufferSize));
		if (nALBuffer == AL_NONE) {
			openalSendError(::alureGetErrorString(), oCommand);
			return; //------------------------------------------------------
		}
	}
	aFileToBufferId.emplace_back(oCommand.m_nFileId, nALBuffer);
}
void Backend::openalPlay(const AlCommand& oCommand) noexcept
{
//std::cout << "Backend::openalPlay   oCommand.m_nBackendDeviceId = " << oCommand.m_nBackendDeviceId << '\n';
	{
		// No idea why this is needed but when playing the same sound (file) on two different
		// devices alGetError returns AL_INVALID_NAME for the scond play
		//TODO investigate, check whether the same happens with openalPreload, if it doesn't
		//     it might be an alure bug
		//const ALenum nErr =
		::alGetError(); // reset error so that alureCreateBufferFromFile doesn't fail
		//if (nErr != AL_NO_ERROR) {
		//	std::cout << "Backend::openalPlay   lingering error: " << nErr << '\n';
		//}
	}
	AlDevice& oAlDevice = getActiveDevice(oCommand.m_nBackendDeviceId);
	// get or create buffer
	ALuint nALBuffer;
	{
	auto& aFileToBufferId = oAlDevice.m_aFileToBufferId;
	const auto itFind = std::find_if(aFileToBufferId.begin(), aFileToBufferId.end(), [&](const std::pair<int32_t, ALuint>& oPair)
	{
		return (oPair.first == oCommand.m_nFileId);
	});
	if (itFind == aFileToBufferId.end()) {
		// first time this file is played
		// create AL buffer
		if (oCommand.m_p0Buffer == nullptr) {
//std::cout << "Backend::openalPlay   alureCreateBufferFromFile = " << oCommand.m_sFileName << '\n';
			nALBuffer = ::alureCreateBufferFromFile(oCommand.m_sFileName.c_str());
			if (nALBuffer == AL_NONE) {
				openalSendError(::alureGetErrorString(), oCommand);
				return; //------------------------------------------------------
			}
		} else {
			nALBuffer = ::alureCreateBufferFromMemory(static_cast<ALubyte*>(const_cast<uint8_t*>(oCommand.m_p0Buffer))
													, static_cast<ALsizei>(oCommand.m_nBufferSize));
			if (nALBuffer == AL_NONE) {
				openalSendError(::alureGetErrorString(), oCommand);
				return; //------------------------------------------------------
			}
		}
		aFileToBufferId.emplace_back(oCommand.m_nFileId, nALBuffer);
	} else {
		nALBuffer = itFind->second;
	}
	}
	// get or create source
	ALuint nSourceId;
	{
	auto& aActiveSounds = oAlDevice.m_aActiveSounds;
	#ifndef NDEBUG
	// check sound id not active
	const auto itFind = std::find_if(aActiveSounds.begin(), aActiveSounds.end(), [&](const ActiveSound& oActiveSound)
	{
		return (oActiveSound.m_nSoundId == oCommand.m_nSoundId);
	});
	assert(itFind == aActiveSounds.end());
	#endif //NDEBUG
	auto& aUnusedSourceIds = oAlDevice.m_aUnusedSourceIds;
	if (aUnusedSourceIds.empty()) {
		//::alGetError();
		::alGenSources(1, &nSourceId);
		const ALenum nErr = ::alGetError();
		if (nErr != AL_NO_ERROR) {
			//openalSendError(::alureGetErrorString(), oCommand);
			//return; //----------------------------------------------------------
			std::cout << "Backend::openalPlay   alGenSources error: " << nErr << '\n';
		}
	} else {
		nSourceId = aUnusedSourceIds.back();
		aUnusedSourceIds.pop_back();
	}

	const double fVolume = [](double fVolume)
	{
		if (fVolume < 0.0) {
			return 0.0;
		} else if (fVolume > 1.0) {
			return 1.0;
		}
		return fVolume;
	}(oCommand.m_fVolume);
	::alSourcef(nSourceId, AL_GAIN, fVolume);
	::alSourcei(nSourceId, AL_LOOPING, (oCommand.m_bLoop ? AL_TRUE : AL_FALSE));
	::alSourcei(nSourceId, AL_SOURCE_RELATIVE, (oCommand.m_bRelative ? AL_TRUE : AL_FALSE));
	::alSource3f(nSourceId, AL_POSITION, clamp(oCommand.m_fPosX), clamp(oCommand.m_fPosY), clamp(oCommand.m_fPosZ));
	::alSourcei(nSourceId, AL_BUFFER, nALBuffer);
	{
		const ALenum nErr = ::alGetError();
		if (nErr != AL_NO_ERROR) {
			std::cout << "Backend::openalPlay   assigning buffer to source error: " << nErr << '\n';
		}
	}

	ActiveSound oActiveSound;
	oActiveSound.m_nSoundId = oCommand.m_nSoundId;
	oActiveSound.m_nALSourceId = nSourceId;
	oActiveSound.m_bPaused = false;
	oActiveSound.m_bStartedWhenDevicePaused = oAlDevice.m_bDevicePaused;
	aActiveSounds.emplace_back(std::move(oActiveSound));
	}

	// prepare the finished event
	AlEvent& oAlEvent = getSoundFinishedAlEvent(oCommand);
	//#ifndef NDEBUG
	const ALboolean bRet = ::alurePlaySource(nSourceId, openalSoundFinishedCallback, &oAlEvent);
	if (bRet == AL_FALSE) {
		std::cout << "Backend::openalPlay   alurePlaySource error: " << ::alureGetErrorString() << '\n';
	}
	//{
	//	const ALenum nErr = ::alGetError();
	//	if (nErr != AL_NO_ERROR) {
	//		std::cout << "Backend::openalPlay   alurePlaySource error: " << nErr << '\n';
	//	}
	//}
}
Backend::AlDevice& Backend::getActiveDevice(int32_t nDeviceId) noexcept
{
	assert(nDeviceId >= 0);
	assert(nDeviceId < static_cast<int32_t>(m_aAlDevices.size()));
	AlDevice& oAlDevice = m_aAlDevices[nDeviceId];
	ALCcontext* p0Context = oAlDevice.m_pContext;
	::alcMakeContextCurrent(p0Context);
	//#ifndef NDEBUG
	//const auto nErr = ::alcGetError(oAlDevice.m_pDevice);
	//if (nErr != ALC_NO_ERROR) {
	//	std::cout << "alcMakeContextCurrent error" << '\n';
	//}
	//#endif //NDEBUG
	return oAlDevice;
}
std::vector<Backend::ActiveSound>::iterator Backend::getActiveSoundIt(int32_t nSoundId, AlDevice& oAlDevice) noexcept
{
	auto& aActiveSounds = oAlDevice.m_aActiveSounds;
	const auto itFind = std::find_if(aActiveSounds.begin(), aActiveSounds.end(), [&](const ActiveSound& oActiveSound)
	{
		return (oActiveSound.m_nSoundId == nSoundId);
	});
	return itFind;
}
Backend::ActiveSound* Backend::getActiveSound(const AlCommand& oCommand, AlDevice& oAlDevice) noexcept
{
	auto itFind = getActiveSoundIt(oCommand.m_nSoundId, oAlDevice);
	auto& aActiveSounds = oAlDevice.m_aActiveSounds;
	if (itFind == aActiveSounds.end()) {
		return nullptr;
	}
	return &(*itFind);
}
void Backend::openalPause(const AlCommand& oCommand) noexcept
{
	AlDevice& oAlDevice = getActiveDevice(oCommand.m_nBackendDeviceId);
	ActiveSound* p0ActiveSound = getActiveSound(oCommand, oAlDevice);
	if (p0ActiveSound == nullptr) {
		return; //--------------------------------------------------------------
	}
	auto& oActiveSound = *p0ActiveSound;
	if (oActiveSound.m_bPaused) {
		return; //--------------------------------------------------------------
	}
	oActiveSound.m_bPaused = true;
	if ((! oAlDevice.m_bDevicePaused) || oActiveSound.m_bStartedWhenDevicePaused) {
		::alurePauseSource(oActiveSound.m_nALSourceId);
	}
}
void Backend::openalResume(const AlCommand& oCommand) noexcept
{
	AlDevice& oAlDevice = getActiveDevice(oCommand.m_nBackendDeviceId);
	ActiveSound* p0ActiveSound = getActiveSound(oCommand, oAlDevice);
	if (p0ActiveSound == nullptr) {
		return; //--------------------------------------------------------------
	}
	auto& oActiveSound = *p0ActiveSound;
	if (! oActiveSound.m_bPaused) {
		return; //--------------------------------------------------------------
	}
	oActiveSound.m_bPaused = false;
	if ((! oAlDevice.m_bDevicePaused) || oActiveSound.m_bStartedWhenDevicePaused) {
		::alureResumeSource(oActiveSound.m_nALSourceId);
	}
}
void Backend::removeActiveSound(std::vector<ActiveSound>& aActiveSounds, std::vector<ActiveSound>::iterator itActiveSound) noexcept
{
	//oAlDevice.m_aActiveSounds.erase(itActiveSound);
	const int32_t nTotIdxs = static_cast<int32_t>(aActiveSounds.size());
	const int32_t nIdx = std::distance(aActiveSounds.begin(), itActiveSound);
	if (nIdx < nTotIdxs - 1) {
		aActiveSounds[nIdx] = std::move(aActiveSounds[nTotIdxs - 1]);
	}
	aActiveSounds.pop_back();
}
void openalSoundFinishedCallback(void* p0AlEvent, ALuint
#ifndef NDEBUG
nSourceIdCheck
#endif //NDEBUG
) noexcept
{
//std::cout << "openalSoundFinishedCallback" << '\n';
	Backend::AlEvent& oAlEvent = *(static_cast<Backend::AlEvent*>(p0AlEvent));
	const int64_t nDeviceId = oAlEvent.m_nBackendDeviceId;
	const int32_t nSoundId = oAlEvent.m_nSoundId;
	Backend* p0This = oAlEvent.m_p0Backend;
	// send finished event
	{
		std::lock_guard<std::mutex> oLock(p0This->m_oAlEventMutex);
		p0This->m_aAlEvents.push_back(std::move(oAlEvent));
	}
	//
	Backend::AlDevice& oAlDevice = p0This->m_aAlDevices[nDeviceId];
	// remove from active sounds
	auto itFind = p0This->getActiveSoundIt(nSoundId, oAlDevice);
	auto& aActiveSounds = oAlDevice.m_aActiveSounds;
	assert(itFind != aActiveSounds.end());
	auto& oActiveSound = *itFind;
	const ALuint nSourceId = oActiveSound.m_nALSourceId;
	assert(nSourceId == nSourceIdCheck);

	// detach buffer from source
	::alSourcei(nSourceId, AL_BUFFER, 0);
	// recycle source
	auto& aUnusedSourceIds = oAlDevice.m_aUnusedSourceIds;
	aUnusedSourceIds.push_back(nSourceId);
	// recycle moved from event
	oAlEvent.m_eType = Backend::AL_EVENT_INVALID;
	//
	p0This->removeActiveSound(aActiveSounds, itFind);
}
void Backend::openalStop(const AlCommand& oCommand) noexcept
{
	AlDevice& oAlDevice = getActiveDevice(oCommand.m_nBackendDeviceId);
	auto itActiveSound = getActiveSoundIt(oCommand.m_nSoundId, oAlDevice);
	auto& aActiveSounds = oAlDevice.m_aActiveSounds;
	if (itActiveSound == aActiveSounds.end()) {
		// already stopped
		return; //--------------------------------------------------------------
	}
	auto& oActiveSound = *itActiveSound;
	
	const ALuint nSourceId = oActiveSound.m_nALSourceId;
	::alureStopSource(nSourceId, AL_FALSE);

	// detach buffer from source
	::alSourcei(nSourceId, AL_BUFFER, 0);
	// recycle source
	auto& aUnusedSourceIds = oAlDevice.m_aUnusedSourceIds;
	aUnusedSourceIds.push_back(nSourceId);

	removeActiveSound(aActiveSounds, itActiveSound);
}
void Backend::openalPauseDevice(const AlCommand& oCommand) noexcept
{
//std::cout << "Backend::openalPauseDevice" << '\n';
	AlDevice& oAlDevice = getActiveDevice(oCommand.m_nBackendDeviceId);
	if (oAlDevice.m_bDevicePaused) {
		// already paused
		return; //--------------------------------------------------------------
	}
	auto& aActiveSounds = oAlDevice.m_aActiveSounds;
	for (auto& oActiveSound : aActiveSounds) {
		if (! oActiveSound.m_bPaused) {
			::alurePauseSource(oActiveSound.m_nALSourceId);
		}
	}
	oAlDevice.m_bDevicePaused = true;
}
void Backend::openalResumeDevice(const AlCommand& oCommand) noexcept
{
//std::cout << "Backend::openalResumeDevice" << '\n';
	AlDevice& oAlDevice = getActiveDevice(oCommand.m_nBackendDeviceId);
	if (! oAlDevice.m_bDevicePaused) {
		// not paused
		return; //--------------------------------------------------------------
	}
	auto& aActiveSounds = oAlDevice.m_aActiveSounds;
	for (auto& oActiveSound : aActiveSounds) {
		if (! oActiveSound.m_bPaused) {
			if (! oActiveSound.m_bStartedWhenDevicePaused) {
				::alureResumeSource(oActiveSound.m_nALSourceId);
			} else {
				oActiveSound.m_bStartedWhenDevicePaused = false;
			}
		}
	}
	oAlDevice.m_bDevicePaused = false;
}
void Backend::openalStopAll(const AlCommand& oCommand) noexcept
{
	AlDevice& oAlDevice = getActiveDevice(oCommand.m_nBackendDeviceId);
	auto& aActiveSounds = oAlDevice.m_aActiveSounds;
	while (! aActiveSounds.empty()) {
		ActiveSound& oActiveSound = aActiveSounds[0];
		//
		const ALuint nSourceId = oActiveSound.m_nALSourceId;
		::alureStopSource(nSourceId, AL_FALSE);

		// detach buffer from source
		::alSourcei(nSourceId, AL_BUFFER, 0);
		// recycle source
		auto& aUnusedSourceIds = oAlDevice.m_aUnusedSourceIds;
		aUnusedSourceIds.push_back(nSourceId);

		removeActiveSound(aActiveSounds, aActiveSounds.begin());
	}
}
void Backend::openalSoundPos(const AlCommand& oCommand) noexcept
{
//std::cout << "Backend::openalSoundPos   oCommand.m_nBackendDeviceId = " << oCommand.m_nBackendDeviceId << '\n';
	AlDevice& oAlDevice = getActiveDevice(oCommand.m_nBackendDeviceId);
	auto itActiveSound = getActiveSoundIt(oCommand.m_nSoundId, oAlDevice);
	auto& aActiveSounds = oAlDevice.m_aActiveSounds;
	if (itActiveSound == aActiveSounds.end()) {
		// already stopped
		return; //--------------------------------------------------------------
	}
	auto& oActiveSound = *itActiveSound;

	const ALuint nSourceId = oActiveSound.m_nALSourceId;
	::alSource3f(nSourceId, AL_POSITION, clamp(oCommand.m_fPosX), clamp(oCommand.m_fPosY), clamp(oCommand.m_fPosZ));
//std::cout << "Backend::openalSoundPos   oCommand.m_fPosX = " << oCommand.m_fPosX << '\n';
//std::cout << "Backend::openalSoundPos   oCommand.m_bRelative = " << oCommand.m_bRelative << '\n';
	::alSourcei(nSourceId, AL_SOURCE_RELATIVE, (oCommand.m_bRelative ? AL_TRUE : AL_FALSE));
}
void Backend::openalSoundVol(const AlCommand& oCommand) noexcept
{
	AlDevice& oAlDevice = getActiveDevice(oCommand.m_nBackendDeviceId);
	auto itActiveSound = getActiveSoundIt(oCommand.m_nSoundId, oAlDevice);
	auto& aActiveSounds = oAlDevice.m_aActiveSounds;
	if (itActiveSound == aActiveSounds.end()) {
		// already stopped
		return; //--------------------------------------------------------------
	}
	auto& oActiveSound = *itActiveSound;

	const ALuint nSourceId = oActiveSound.m_nALSourceId;
	const double fVolume = [](double fVolume)
	{
		if (fVolume < 0.0) {
			return 0.0;
		} else if (fVolume > 1.0) {
			return 1.0;
		}
		return fVolume;
	}(oCommand.m_fVolume);
	::alSourcef(nSourceId, AL_GAIN, fVolume);
}
void Backend::openalListenerPos(const AlCommand& oCommand) noexcept
{
	// sets device context
	getActiveDevice(oCommand.m_nBackendDeviceId);

	::alListener3f(AL_POSITION, clamp(oCommand.m_fPosX), clamp(oCommand.m_fPosY), clamp(oCommand.m_fPosZ));
//std::cout << "Backend::openalListenerPos  oCommand.m_fPosX =" << oCommand.m_fPosX << '\n';
}
void Backend::openalListenerVol(const AlCommand& oCommand) noexcept
{
	// sets device context
	getActiveDevice(oCommand.m_nBackendDeviceId);

	const double fVolume = [](double fVolume)
	{
		if (fVolume < 0.0) {
			return 0.0;
		} else if (fVolume > 1.0) {
			return 1.0;
		}
		return fVolume;
	}(oCommand.m_fVolume);
	::alListenerf(AL_GAIN, fVolume);
}
	//void Backend::openalListenerDir(const AlCommand& oCommand) noexcept
	//{
	//	// sets device context
	//	getActiveDevice(oCommand.m_nBackendDeviceId);
	//
	//	std::array<ALfloat, 6> aDir;
	//	::alGetListenerfv(AL_ORIENTATION, aDir.data());
	//	aDir[3] = static_cast<ALfloat>(oCommand.m_fPosX);
	//	aDir[4] = static_cast<ALfloat>(oCommand.m_fPosY);
	//	aDir[5] = static_cast<ALfloat>(oCommand.m_fPosZ);
	//	//const auto fDirX = static_cast<ALfloat>(oCommand.m_fPosX);
	//	//const auto fDirY = static_cast<ALfloat>(oCommand.m_fPosY);
	//	//const auto fDirZ = static_cast<ALfloat>(oCommand.m_fPosZ);
	//	//const std::array<ALfloat, 3> aDir{fDirX, fDirY, fDirZ};
	//	::alListenerfv(AL_ORIENTATION, aDir.data());
	//}
Backend::AlEvent& Backend::getOrCreateAlEvent() noexcept
{
	auto itFind = std::find_if(m_aToFinishAlEvents.begin(), m_aToFinishAlEvents.end(), [&](AlEvent& oAlEvent)
	{
		return (oAlEvent.m_eType == AL_EVENT_INVALID);
	});
	AlEvent* p0AlEvent = [&]()
	{
		if (itFind == m_aToFinishAlEvents.end()) {
			m_aToFinishAlEvents.emplace_back();
			AlEvent& oAlEvent = m_aToFinishAlEvents.back();
			return &oAlEvent;
		} else {
			AlEvent& oAlEvent = *itFind;
			return &oAlEvent;
		}
	}();
	p0AlEvent->m_p0Backend = this;
	return *p0AlEvent;
}
Backend::AlEvent& Backend::getSoundFinishedAlEvent(const AlCommand& oCommand) noexcept
{
	AlEvent& oAlEvent = getOrCreateAlEvent();
	oAlEvent.m_eType = AL_EVENT_PLAY_FINISHED;
	oAlEvent.m_nBackendDeviceId = oCommand.m_nBackendDeviceId;
	oAlEvent.m_nSoundId = oCommand.m_nSoundId;
	return oAlEvent;
}

Backend::AlDevice& Backend::getOrCreateAlDevice(int32_t& nDeviceId) noexcept
{
	auto itFind = std::find_if(m_aAlDevices.begin(), m_aAlDevices.end(), [&](AlDevice& oAlDevice)
	{
		return oAlDevice.m_bDeviceRemoved;
	});
	if (itFind == m_aAlDevices.end()) {
		nDeviceId = static_cast<int32_t>(m_aAlDevices.size());
		m_aAlDevices.emplace_back();
		AlDevice& oAlDevice = m_aAlDevices.back();
		return oAlDevice;
	} else {
		nDeviceId = static_cast<int32_t>(std::distance(m_aAlDevices.begin(), itFind));
		AlDevice& oAlDevice = *itFind;
		oAlDevice.m_bDeviceRemoved = false;
		return oAlDevice;
	}
}
int32_t Backend::openalCreateDevice(const std::string& sDeviceName) noexcept
{
	int32_t nDeviceId;
	AlDevice& oDev = getOrCreateAlDevice(nDeviceId);
	oDev.m_sDeviceName = sDeviceName;
	oDev.m_pContext = ::alcGetCurrentContext();
	oDev.m_pDevice = ::alcGetContextsDevice(oDev.m_pContext);
	return nDeviceId;
}
void Backend::sendDeviceAddedAlEvent(int32_t nDeviceId, const std::string& sDeviceName) noexcept
{
	AlEvent oAlEvent;
	oAlEvent.m_eType = AL_EVENT_DEVICE_ADDED;
	oAlEvent.m_sDeviceName = sDeviceName;
	oAlEvent.m_nBackendDeviceId = nDeviceId;
	oAlEvent.m_bDeviceIsDefault = (nDeviceId == m_nDefaultDeviceId);
	{
		std::lock_guard<std::mutex> oLock(m_oAlEventMutex);
		m_aAlEvents.push_back(std::move(oAlEvent));
	}
}
std::string Backend::openalCreateAllDevices(bool bUseDeviceNames, std::vector<std::string>& aDeviceNames, bool bSendEvent) noexcept
{
//std::cout << "Backend::openalCreateAllDevices bSendEvent=" << bSendEvent << '\n';
	m_nTotAlDevices = 0;
	int32_t nDefaultIdx;
	if (! bUseDeviceNames) {
		aDeviceNames.clear();
		const std::string sErr = openalGetDeviceNames(aDeviceNames, nDefaultIdx);
		if (! sErr.empty()) {
			return sErr; //-----------------------------------------------------
		}
	}
	const int32_t nTotIdxs = static_cast<int32_t>(aDeviceNames.size());
	m_aAlDevices.reserve(nTotIdxs);
	for (int32_t nIdx = 0; nIdx < nTotIdxs; ++nIdx) {
		const std::string& sDeviceName = aDeviceNames[nIdx];
		const ALboolean bRet = ::alureInitDevice(sDeviceName.data(), nullptr);
		if (bRet == AL_FALSE) {
			continue;
		}
		const int32_t nDeviceId = openalCreateDevice(sDeviceName);
		if (nDeviceId < 0) {
			::alureShutdownDevice();
			continue;
		}
		if (nDefaultIdx == nIdx) {
			m_nDefaultDeviceId = m_nTotAlDevices;
		}
		++m_nTotAlDevices;
		if (bSendEvent) {
			sendDeviceAddedAlEvent(nDeviceId, sDeviceName);
		}
	}
	return "";
}
void Backend::sendDeviceChangedAlEvent(int32_t nDeviceId) noexcept
{
	AlEvent oAlEvent;
	oAlEvent.m_eType = AL_EVENT_DEVICE_CHANGED;
	oAlEvent.m_nBackendDeviceId = nDeviceId;
	{
		std::lock_guard<std::mutex> oLock(m_oAlEventMutex);
		m_aAlEvents.push_back(std::move(oAlEvent));
	}
}
void Backend::sendDeviceRemovedAlEvent(int32_t nDeviceId) noexcept
{
	AlEvent oAlEvent;
	oAlEvent.m_eType = AL_EVENT_DEVICE_REMOVED;
	oAlEvent.m_nBackendDeviceId = nDeviceId;
	{
		std::lock_guard<std::mutex> oLock(m_oAlEventMutex);
		m_aAlEvents.push_back(std::move(oAlEvent));
	}
}
void Backend::openalRemoveAllDevices() noexcept
{
//std::cout << "Backend::openalRemoveAllDevices" << '\n';
	const int32_t nTotOldIdxs = static_cast<int32_t>(m_aAlDevices.size());
	for (int32_t nDeviceId = 0; nDeviceId < nTotOldIdxs; ++nDeviceId) {
		AlDevice& oAlDevice = m_aAlDevices[nDeviceId];
		if (oAlDevice.m_bDeviceRemoved) {
			continue;
		}
		sendDeviceRemovedAlEvent(nDeviceId);
		openalShutdownDevice(oAlDevice);
	}
}
void Backend::openalCheckDeviceNames() noexcept
{
//std::cout << "Backend::openalCheckDeviceNames  m_nTotAlDevices=" << m_nTotAlDevices << '\n';
	std::vector<std::string> aDeviceNames;
	int32_t nDefaultIdx;
	openalGetDeviceNames(aDeviceNames, nDefaultIdx);

	const int32_t nTotNewIdxs = static_cast<int32_t>(aDeviceNames.size());

//std::cout << "Backend::openalCheckDeviceNames  nTotNewIdxs=" << nTotNewIdxs << '\n';
//std::cout << "Backend::openalCheckDeviceNames  m_nTotAlDevices=" << m_nTotAlDevices << '\n';
//for (auto& sDevName : aDeviceNames) {
//std::cout << "Backend::openalCheckDeviceNames  new name= " << sDevName << '\n';
//}
	if (nTotNewIdxs != m_nTotAlDevices) {
		// When a new device is added or removed all the names get screwed up.
		// Since OpenAl doesn't provide a way to tell the current name of a ALCdevice*
		// and alcOpenDevice doesn't fail if a device with the same name is already open,
		// we have to remove all old devices and recreate all new devices from the new names.
		openalRemoveAllDevices();
		openalCreateAllDevices(true, aDeviceNames, true);
	} else {
		// check whether the names have changed
		bool bAtLeastOneChanged = false;
		for (const AlDevice& oAlDevice : m_aAlDevices) {
			if (oAlDevice.m_bDeviceRemoved) {
				continue;
			}
//std::cout << "Backend::openalCheckDeviceNames  old oAlDevice.m_sDeviceName=" << oAlDevice.m_sDeviceName << '\n';
			const auto itFind = std::find(aDeviceNames.begin(), aDeviceNames.end(), oAlDevice.m_sDeviceName);
			if (itFind == aDeviceNames.end()) {
//std::cout << "              -> not found" << '\n';
				bAtLeastOneChanged = true;
				break;
			}
		}
		if (bAtLeastOneChanged) {
			openalRemoveAllDevices();
			openalCreateAllDevices(true, aDeviceNames, true);
		} else {
			// do nothing: if a device is added and removed quickly it might 
			// detect it!
		}
	}
}
void Backend::openalShutdownDevice(AlDevice& oDev) noexcept
{
//std::cout << "Backend::openalShutdownDevice  name=" << oDev.m_sDeviceName << " " << oDev.m_bDeviceRemoved << '\n';
	#ifndef NDEBUG
	ALboolean bRet =
	#endif //NDEBUG
	::alcMakeContextCurrent(oDev.m_pContext);
	assert(bRet == AL_TRUE);

	// sources must be unbuffered to remove buffers
	for (const ActiveSound& oActiveSound : oDev.m_aActiveSounds) {
		::alureStopSource(oActiveSound.m_nALSourceId, AL_FALSE);
		::alSourcei(oActiveSound.m_nALSourceId, AL_BUFFER, 0);
	}
	// after shutdown source ids are no longer valid
	oDev.m_aUnusedSourceIds.clear();
	oDev.m_aActiveSounds.clear();

	// delete buffers since not deleted by alureShutdownDevice()
	for (const auto& oPair : oDev.m_aFileToBufferId) {
		::alDeleteBuffers(1, &oPair.second);
	}
	oDev.m_aFileToBufferId.clear();
	oDev.m_bDevicePaused = false;
	oDev.m_bDeviceRemoved = true;
	//
	#ifndef NDEBUG
	bRet =
	#endif //NDEBUG
	::alureShutdownDevice();
	assert(bRet == AL_TRUE);

	--m_nTotAlDevices;
}


} // namespace OpenAl
} // namespace Private

} // namespace stmi
