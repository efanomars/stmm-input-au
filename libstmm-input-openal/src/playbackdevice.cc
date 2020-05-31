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
 * File:   playbackdevice.cc
 */

#include "playbackdevice.h"

#include "openalbackend.h"
#include "openallistenerextradata.h"

#include <stmm-input-base/basicdevicemanager.h>
#include <stmm-input-base/basicdevice.h>

#include <stmm-input/devicemanager.h>
#include <stmm-input/event.h>
#include <stmm-input/capability.h>

#include <algorithm>
#include <cstdint>
#include <utility>
#include <cassert>
#include <iostream>
#include <iterator>

namespace stmi { class Device; }

namespace stmi
{

namespace Private
{
namespace OpenAl
{

static int32_t s_nFileId = 0;
static int32_t s_nSoundId = 0;

PlaybackDevice::PlaybackDevice(const std::string& sName, const shared_ptr<OpenAlDeviceManager>& refDeviceManager
								, Backend& oBackend, int32_t nBackendDeviceId, bool bIsDefault) noexcept
: BasicDevice(sName, refDeviceManager)
, m_oBackend(oBackend)
, m_nBackendDeviceId(nBackendDeviceId)
, m_bIsDefault(bIsDefault)
{
}
shared_ptr<Device> PlaybackDevice::getDevice() const noexcept
{
	shared_ptr<const PlaybackDevice> refConstThis = shared_from_this();
	shared_ptr<PlaybackDevice> refThis = std::const_pointer_cast<PlaybackDevice>(refConstThis);
//std::cout << "PlaybackDevice::getDevice()  id=" << getDeviceId() << '\n';
	return refThis;
}
shared_ptr<Capability> PlaybackDevice::getCapability(const Capability::Class& oClass) const noexcept
{
	shared_ptr<Capability> refCapa;
	if (oClass == typeid(PlaybackCapability)) {
		shared_ptr<const PlaybackDevice> refConstThis = shared_from_this();
		shared_ptr<PlaybackDevice> refThis = std::const_pointer_cast<PlaybackDevice>(refConstThis);
		refCapa = refThis;
	}
	return refCapa;
}
shared_ptr<Capability> PlaybackDevice::getCapability(int32_t nCapabilityId) const noexcept
{
	const auto nKeyCapaId = PlaybackCapability::getId();
	if (nCapabilityId != nKeyCapaId) {
		return shared_ptr<Capability>{};
	}
	shared_ptr<const PlaybackDevice> refConstThis = shared_from_this();
	shared_ptr<PlaybackDevice> refThis = std::const_pointer_cast<PlaybackDevice>(refConstThis);
	return refThis;
}
std::vector<int32_t> PlaybackDevice::getCapabilities() const noexcept
{
	return {PlaybackCapability::getId()};
}
std::vector<Capability::Class> PlaybackDevice::getCapabilityClasses() const noexcept
{
	return {PlaybackCapability::getClass()};
}
void PlaybackDevice::removingDevice() noexcept
{
	resetOwnerDeviceManager();
}

void PlaybackDevice::setIsDefault(bool bIsDefault) noexcept
{
	m_bIsDefault = bIsDefault;
}

int32_t PlaybackDevice::preloadSound(const std::string& sFileName, uint8_t const* p0Buffer, int32_t nBufferSize) noexcept
{
	const int32_t nFileId = s_nFileId;
	++s_nFileId;
	m_aFileNameToIds.push_back(FileNameToId{sFileName, nFileId});

	Backend::AlCommand oAlCommand;
	oAlCommand.m_nBackendDeviceId = m_nBackendDeviceId;
	oAlCommand.m_eType = Backend::AL_COMMAND_PRELOAD;
	if (p0Buffer == nullptr) {
		oAlCommand.m_sFileName = sFileName;
	} else {
		oAlCommand.m_p0Buffer = p0Buffer;
		oAlCommand.m_nBufferSize = nBufferSize;
	}
	//oAlCommand.m_bLoop = bLoop;
	oAlCommand.m_nFileId = nFileId;
	//oAlCommand.m_nSoundId = nSoundId;
	//oAlCommand.m_fVolume = fVolume;
	//oAlCommand.m_bRelative = bRelative;
	//oAlCommand.m_fPosX = fX;
	//oAlCommand.m_fPosY = fY;
	//oAlCommand.m_fPosZ = fZ;

	m_oBackend.sendCommand(std::move(oAlCommand));
	return nFileId;
}
int32_t PlaybackDevice::preloadSound(const std::string& sFileName) noexcept
{
	auto refOwner = getOwnerDeviceManager();
	if (!refOwner) {
		return -1; //-----------------------------------------------------------
	}
	const auto itFind = std::find_if(m_aFileNameToIds.begin(), m_aFileNameToIds.end(), [&](const FileNameToId& oFileNameToId)
	{
		return oFileNameToId.m_sFileName == sFileName;
	});
	if (itFind != m_aFileNameToIds.end()) {
		return itFind->m_nFileId; //--------------------------------------------
	}
	return preloadSound(sFileName, nullptr, 0);
}
int32_t PlaybackDevice::preloadSound(uint8_t const* p0Buffer, int32_t nBufferSize) noexcept
{
	auto refOwner = getOwnerDeviceManager();
	if (!refOwner) {
		return -1; //-----------------------------------------------------------
	}
	const auto itFind = std::find_if(m_aBufferToIds.begin(), m_aBufferToIds.end(), [&](const BufferToId& oBufferToId)
	{
		return oBufferToId.m_p0Buffer == p0Buffer;
	});
	if (itFind != m_aBufferToIds.end()) {
		return itFind->m_nFileId; //--------------------------------------------
	}
	return preloadSound("", p0Buffer, nBufferSize);
}
int32_t PlaybackDevice::playSound(OpenAlDeviceManager* p0Owner
							, const std::string& sFileName, const uint8_t* p0Buffer, int32_t nBufferSize, int32_t nFileId
							, double fVolume, bool bLoop, bool bRelative, double fX, double fY, double fZ) noexcept
{
	const int32_t nSoundId = s_nSoundId;
	++s_nSoundId;
	m_aActiveSoundIds.push_back(nSoundId);
	m_aActiveSoundStarts.push_back(p0Owner->getUniqueTimeStamp());

	Backend::AlCommand oAlCommand;
	oAlCommand.m_nBackendDeviceId = m_nBackendDeviceId;
	oAlCommand.m_eType = Backend::AL_COMMAND_PLAY;
	if (p0Buffer == nullptr) {
		oAlCommand.m_sFileName = sFileName;
	} else {
		oAlCommand.m_p0Buffer = p0Buffer;
		oAlCommand.m_nBufferSize = nBufferSize;
	}
	oAlCommand.m_bLoop = bLoop;
	oAlCommand.m_nFileId = nFileId;
	oAlCommand.m_nSoundId = nSoundId;
	oAlCommand.m_fVolume = fVolume;
	oAlCommand.m_bRelative = bRelative;
	oAlCommand.m_fPosX = fX;
	oAlCommand.m_fPosY = fY;
	oAlCommand.m_fPosZ = fZ;

	m_oBackend.sendCommand(std::move(oAlCommand));

	return nSoundId;
}
PlaybackCapability::SoundData PlaybackDevice::playSound(const std::string& sFileName, double fVolume, bool bLoop
														, bool bRelative, double fX, double fY, double fZ) noexcept
{
	auto refOwner = getOwnerDeviceManager();
	if (!refOwner) {
		return PlaybackCapability::SoundData{}; //------------------------------
	}
	//
	OpenAlDeviceManager* p0Owner = refOwner.get();

	const auto itFind = std::find_if(m_aFileNameToIds.begin(), m_aFileNameToIds.end(), [&](const FileNameToId& oFileNameToId)
	{
		return oFileNameToId.m_sFileName == sFileName;
	});
	int32_t nFileId;
	if (itFind == m_aFileNameToIds.end()) {
		nFileId = s_nFileId;
		++s_nFileId;
		m_aFileNameToIds.push_back(FileNameToId{sFileName, nFileId});
	} else {
		nFileId = itFind->m_nFileId;
	}

	const int32_t nSoundId = playSound(p0Owner, sFileName, nullptr, 0, nFileId, fVolume, bLoop, bRelative, fX, fY, fZ);
	return SoundData{nSoundId, nFileId};
}
PlaybackCapability::SoundData PlaybackDevice::playSound(const uint8_t* p0Buffer, int32_t nBufferSize, double fVolume, bool bLoop
														, bool bRelative, double fX, double fY, double fZ) noexcept
{
	auto refOwner = getOwnerDeviceManager();
	if (!refOwner) {
		return PlaybackCapability::SoundData{}; //------------------------------
	}
	//
	OpenAlDeviceManager* p0Owner = refOwner.get();

	const auto itFind = std::find_if(m_aBufferToIds.begin(), m_aBufferToIds.end(), [&](const BufferToId& oBufferToId)
	{
		return oBufferToId.m_p0Buffer == p0Buffer;
	});
	int32_t nFileId;
	if (itFind == m_aBufferToIds.end()) {
		nFileId = s_nFileId;
		++s_nFileId;
		m_aBufferToIds.push_back(BufferToId{p0Buffer, nBufferSize, nFileId});
	} else {
		nFileId = itFind->m_nFileId;
		nBufferSize = itFind->m_nBufferSize;
	}

	const int32_t nSoundId = playSound(p0Owner, "", p0Buffer, nBufferSize, nFileId, fVolume, bLoop, bRelative, fX, fY, fZ);
	return SoundData{nSoundId, nFileId};
}
int32_t PlaybackDevice::playSound(int32_t nFileId, double fVolume, bool bLoop, bool bRelative, double fX, double fY, double fZ) noexcept
{
	auto refOwner = getOwnerDeviceManager();
	if (!refOwner) {
		return -1; //-----------------------------------------------------------
	}
	//
	OpenAlDeviceManager* p0Owner = refOwner.get();

	const auto itFindName = std::find_if(m_aFileNameToIds.begin(), m_aFileNameToIds.end(), [&](const FileNameToId& oFileNameToId)
	{
		return oFileNameToId.m_nFileId == nFileId;
	});
	if (itFindName == m_aFileNameToIds.end()) {
		
		const auto itFind = std::find_if(m_aBufferToIds.begin(), m_aBufferToIds.end(), [&](const BufferToId& oBufferToId)
		{
			return oBufferToId.m_nFileId == nFileId;
		});
		if (itFind == m_aBufferToIds.end()) {
			return -1; //-------------------------------------------------------
		}
		return playSound(p0Owner, "", itFind->m_p0Buffer, itFind->m_nBufferSize, nFileId, fVolume, bLoop, bRelative, fX, fY, fZ);
	} else {
		return playSound(p0Owner, itFindName->m_sFileName, nullptr, 0, nFileId, fVolume, bLoop, bRelative, fX, fY, fZ);
	}
}

bool PlaybackDevice::setSoundPos(int32_t nSoundId, bool bRelative, double fX, double fY, double fZ) noexcept
{
	auto refOwner = getOwnerDeviceManager();
	if (!refOwner) {
		return false; //--------------------------------------------------------
	}

	const auto itFind = std::find(m_aActiveSoundIds.begin(), m_aActiveSoundIds.end(), nSoundId);
	if (itFind == m_aActiveSoundIds.end()) {
		return false; //--------------------------------------------------------
	}

	Backend::AlCommand oAlCommand;
	oAlCommand.m_nBackendDeviceId = m_nBackendDeviceId;
	oAlCommand.m_eType = Backend::AL_COMMAND_SOUND_POS;
	oAlCommand.m_nSoundId = nSoundId;
	oAlCommand.m_bRelative = bRelative;
	oAlCommand.m_fPosX = fX;
	oAlCommand.m_fPosY = fY;
	oAlCommand.m_fPosZ = fZ;

	m_oBackend.sendCommand(std::move(oAlCommand));
	return true;
}
bool PlaybackDevice::setSoundVol(int32_t nSoundId, double fVolume) noexcept
{
	auto refOwner = getOwnerDeviceManager();
	if (!refOwner) {
		return false; //--------------------------------------------------------
	}

	const auto itFind = std::find(m_aActiveSoundIds.begin(), m_aActiveSoundIds.end(), nSoundId);
	if (itFind == m_aActiveSoundIds.end()) {
		return false; //--------------------------------------------------------
	}

	Backend::AlCommand oAlCommand;
	oAlCommand.m_nBackendDeviceId = m_nBackendDeviceId;
	oAlCommand.m_eType = Backend::AL_COMMAND_SOUND_VOL;
	oAlCommand.m_nSoundId = nSoundId;
	oAlCommand.m_fVolume = fVolume;

	m_oBackend.sendCommand(std::move(oAlCommand));
	return true;
}

bool PlaybackDevice::setListenerPos(double fX, double fY, double fZ) noexcept
{
	auto refOwner = getOwnerDeviceManager();
	if (!refOwner) {
		return false; //--------------------------------------------------------
	}

	Backend::AlCommand oAlCommand;
	oAlCommand.m_nBackendDeviceId = m_nBackendDeviceId;
	oAlCommand.m_eType = Backend::AL_COMMAND_LISTENER_POS;
	oAlCommand.m_fPosX = fX;
	oAlCommand.m_fPosY = fY;
	oAlCommand.m_fPosZ = fZ;

	m_oBackend.sendCommand(std::move(oAlCommand));

	return true;
}
bool PlaybackDevice::setListenerVol(double fVolume) noexcept
{
	auto refOwner = getOwnerDeviceManager();
	if (!refOwner) {
		return false; //--------------------------------------------------------
	}

	Backend::AlCommand oAlCommand;
	oAlCommand.m_nBackendDeviceId = m_nBackendDeviceId;
	oAlCommand.m_eType = Backend::AL_COMMAND_LISTENER_VOL;
	oAlCommand.m_fVolume = fVolume;

	m_oBackend.sendCommand(std::move(oAlCommand));

	return true;
}
//
bool PlaybackDevice::pauseSound(int32_t nSoundId) noexcept
{
	auto refOwner = getOwnerDeviceManager();
	if (!refOwner) {
		return false; //--------------------------------------------------------
	}

	const auto itFind = std::find(m_aActiveSoundIds.begin(), m_aActiveSoundIds.end(), nSoundId);
	if (itFind == m_aActiveSoundIds.end()) {
		return false; //--------------------------------------------------------
	}

	Backend::AlCommand oAlCommand;
	oAlCommand.m_nBackendDeviceId = m_nBackendDeviceId;
	oAlCommand.m_eType = Backend::AL_COMMAND_PAUSE;
	oAlCommand.m_nSoundId = nSoundId;

	m_oBackend.sendCommand(std::move(oAlCommand));

	return true;
}
bool PlaybackDevice::resumeSound(int32_t nSoundId) noexcept
{
	auto refOwner = getOwnerDeviceManager();
	if (!refOwner) {
		return false; //--------------------------------------------------------
	}

	const auto itFind = std::find(m_aActiveSoundIds.begin(), m_aActiveSoundIds.end(), nSoundId);
	if (itFind == m_aActiveSoundIds.end()) {
		return false; //--------------------------------------------------------
	}

	Backend::AlCommand oAlCommand;
	oAlCommand.m_nBackendDeviceId = m_nBackendDeviceId;
	oAlCommand.m_eType = Backend::AL_COMMAND_RESUME;
	oAlCommand.m_nSoundId = nSoundId;

	m_oBackend.sendCommand(std::move(oAlCommand));

	return true;
}
bool PlaybackDevice::stopSound(int32_t nSoundId) noexcept
{
	auto refOwner = getOwnerDeviceManager();
	if (!refOwner) {
		return false; //--------------------------------------------------------
	}

	const auto itFind = std::find(m_aActiveSoundIds.begin(), m_aActiveSoundIds.end(), nSoundId);
	if (itFind == m_aActiveSoundIds.end()) {
		return false; //--------------------------------------------------------
	}

	Backend::AlCommand oAlCommand;
	oAlCommand.m_nBackendDeviceId = m_nBackendDeviceId;
	oAlCommand.m_eType = Backend::AL_COMMAND_STOP;
	oAlCommand.m_nSoundId = nSoundId;

	m_oBackend.sendCommand(std::move(oAlCommand));

	return true;
}

bool PlaybackDevice::pauseDevice() noexcept
{
	auto refOwner = getOwnerDeviceManager();
	if (!refOwner) {
		return false; //--------------------------------------------------------
	}

	Backend::AlCommand oAlCommand;
	oAlCommand.m_nBackendDeviceId = m_nBackendDeviceId;
	oAlCommand.m_eType = Backend::AL_COMMAND_PAUSE_DEVICE;

	m_oBackend.sendCommand(std::move(oAlCommand));

	return true;
}
bool PlaybackDevice::resumeDevice() noexcept
{
	auto refOwner = getOwnerDeviceManager();
	if (!refOwner) {
		return false; //--------------------------------------------------------
	}

	Backend::AlCommand oAlCommand;
	oAlCommand.m_nBackendDeviceId = m_nBackendDeviceId;
	oAlCommand.m_eType = Backend::AL_COMMAND_RESUME_DEVICE;

	m_oBackend.sendCommand(std::move(oAlCommand));

	return true;
}
void PlaybackDevice::stopAllSounds() noexcept
{
	auto refOwner = getOwnerDeviceManager();
	if (!refOwner) {
		return; //--------------------------------------------------------------
	}

	Backend::AlCommand oAlCommand;
	oAlCommand.m_nBackendDeviceId = m_nBackendDeviceId;
	oAlCommand.m_eType = Backend::AL_COMMAND_STOP_ALL;

	m_oBackend.sendCommand(std::move(oAlCommand));
}

bool PlaybackDevice::isDefaultDevice() noexcept
{
	auto refOwner = getOwnerDeviceManager();
	if (!refOwner) {
		return false; //--------------------------------------------------------
	}
	return m_bIsDefault;
}

void PlaybackDevice::onSoundFinished(int32_t nSoundId) noexcept
{
	sendSndFinishedEventToListeners(nSoundId, SndFinishedEvent::FINISHED_TYPE_COMPLETED);
}
void PlaybackDevice::onDeviceError(int32_t nSoundId, int32_t nFileId, std::string&& sError) noexcept
{
	const auto itFindName = std::find_if(m_aFileNameToIds.begin(), m_aFileNameToIds.end(), [&](const FileNameToId& oFileNameToId)
	{
		return oFileNameToId.m_nFileId == nFileId;
	});
	if (itFindName == m_aFileNameToIds.end()) {
		
		const auto itFind = std::find_if(m_aBufferToIds.begin(), m_aBufferToIds.end(), [&](const BufferToId& oBufferToId)
		{
			return oBufferToId.m_nFileId == nFileId;
		});
		if (itFind == m_aBufferToIds.end()) {
			return; //----------------------------------------------------------
		}
		std::cout << "Sound file buffer error (adr: " << reinterpret_cast<int64_t>(itFind->m_p0Buffer) << ")" << '\n';
	} else {
		std::cout << "Sound file path error (" << itFindName->m_sFileName << ")" << '\n';
	}
	std::cout << " -> " << sError << '\n';

	sendSndFinishedEventToListeners(nSoundId, SndFinishedEvent::FINISHED_TYPE_FILE_NOT_FOUND);
}
void PlaybackDevice::sendSndFinishedEventToListeners(int32_t nSoundId, SndFinishedEvent::FINISHED_TYPE eFinishedType) noexcept
{
	const auto itFind = std::find(m_aActiveSoundIds.begin(), m_aActiveSoundIds.end(), nSoundId);
	assert(itFind != m_aActiveSoundIds.end());
	const int32_t nIdx = static_cast<int32_t>(std::distance(m_aActiveSoundIds.begin(), itFind));
	const uint64_t nSoundStartedTimeStamp = m_aActiveSoundStarts[nIdx];
	// remove
	const int32_t nTotSounds = static_cast<int32_t>(m_aActiveSoundIds.size());
	if (nIdx < nTotSounds - 1) {
		m_aActiveSoundIds[nIdx] = m_aActiveSoundIds[nTotSounds - 1];
		m_aActiveSoundStarts[nIdx] = m_aActiveSoundStarts[nTotSounds - 1];
	}
	m_aActiveSoundIds.pop_back();
	m_aActiveSoundStarts.pop_back();
	//
	auto refOwner = getOwnerDeviceManager();
	if (!refOwner) {
		return; //--------------------------------------------------------------
	}
	OpenAlDeviceManager* p0Owner = refOwner.get();

	auto refListeners = p0Owner->getListeners();
	shared_ptr<PlaybackDevice> refPlaybackDevice = shared_from_this();
	shared_ptr<PlaybackCapability> refCapability = refPlaybackDevice;

	shared_ptr<Event> refEvent;
	const int64_t nEventTimeUsec = DeviceManager::getNowTimeMicroseconds();
	for (auto& p0ListenerData : *refListeners) {
		sendSndFinishedEventToListener(*p0ListenerData, nEventTimeUsec, nSoundStartedTimeStamp
										, eFinishedType, nSoundId
										, refCapability, p0Owner->m_nClassIdxSndFinishedEvent, refEvent);
	}
}
void PlaybackDevice::sendSndFinishedEventToListener(const OpenAlDeviceManager::ListenerData& oListenerData
													, int64_t nEventTimeUsec, uint64_t nSoundStartedTimeStamp
													, SndFinishedEvent::FINISHED_TYPE eFinishedType, int32_t nSoundId
													, const shared_ptr<PlaybackCapability>& refCapability
													, int32_t nClassIdxSoundFinishedEvent
													, shared_ptr<Event>& refEvent) noexcept
{
	const auto nAddTimeStamp = oListenerData.getAddedTimeStamp();
//std::cout << "PlaybackDevice::sendSndFinishedEventToListener nSoundStartedTimeStamp=" << nSoundStartedTimeStamp << "  nAddTimeStamp = " << nAddTimeStamp << '\n';
	if (nSoundStartedTimeStamp < nAddTimeStamp) {
		// The listener was added after the key was pressed
		return;
	}
	if (!refEvent) {
		m_oPlaybackSndFinishedEventRecycler.create(refEvent, nEventTimeUsec, refCapability, eFinishedType, nSoundId);
	}
	oListenerData.handleEventCallIf(nClassIdxSoundFinishedEvent, refEvent);
		// no need to reset because KeyEvent cannot be modified.
}
void PlaybackDevice::finishDeviceSounds() noexcept
{
	auto refOwner = getOwnerDeviceManager();
	if (!refOwner) {
		return;
	}
	OpenAlDeviceManager* p0Owner = refOwner.get();
	if (!p0Owner->isEventClassEnabled(Event::Class{typeid(SndFinishedEvent)})) {
		return; //--------------------------------------------------------------
	}

	auto refListeners = p0Owner->getListeners();

	shared_ptr<PlaybackDevice> refThis = shared_from_this();
	shared_ptr<PlaybackDevice> refCapability = refThis;

	const int64_t nEventTimeUsec = DeviceManager::getNowTimeMicroseconds();

	auto aActiveSoundIds = m_aActiveSoundIds;
	auto aActiveSoundStarts = m_aActiveSoundStarts;
	const int32_t nTotSoundIds = static_cast<int32_t>(aActiveSoundIds.size());
	for (int32_t nIdx = 0; nIdx < nTotSoundIds; ++nIdx) {
		const int32_t nSoundId = aActiveSoundIds[nIdx];
		const auto nSoundStarted = aActiveSoundStarts[nIdx];

		shared_ptr<Event> refEvent;
		for (auto& p0ListenerData : *refListeners) {
			OpenAlListenerExtraData* p0ExtraData = nullptr;
			p0ListenerData->getExtraData(p0ExtraData);
			if (p0ExtraData->isSoundFinished(nSoundId)) {
				continue; // for itListenerData ------------
			}
			p0ExtraData->setSoundFinished(nSoundId);

			sendSndFinishedEventToListener(*p0ListenerData, nEventTimeUsec, nSoundStarted
											, SndFinishedEvent::FINISHED_TYPE_LISTENER_REMOVED
											, nSoundId, refCapability, p0Owner->m_nClassIdxSndFinishedEvent, refEvent);
		}
	}
}
void PlaybackDevice::finalizeListener(OpenAlDeviceManager::ListenerData& oListenerData, int64_t nEventTimeUsec) noexcept
{
	auto refOwner = getOwnerDeviceManager();
	if (!refOwner) {
		return;
	}
	OpenAlDeviceManager* p0Owner = refOwner.get();
	if (!p0Owner->isEventClassEnabled(Event::Class{typeid(SndFinishedEvent)})) {
		return; //--------------------------------------------------------------
	}

	shared_ptr<PlaybackDevice> refThis = shared_from_this();
	shared_ptr<PlaybackDevice> refCapability = refThis;

	OpenAlListenerExtraData* p0ExtraData = nullptr;
	oListenerData.getExtraData(p0ExtraData);

	auto aActiveSoundIds = m_aActiveSoundIds;
	auto aActiveSoundStarts = m_aActiveSoundStarts;
	const int32_t nTotSoundIds = static_cast<int32_t>(aActiveSoundIds.size());
	for (int32_t nIdx = 0; nIdx < nTotSoundIds; ++nIdx) {
		const int32_t nSoundId = aActiveSoundIds[nIdx];
		const auto nSoundStarted = aActiveSoundStarts[nIdx];
		//
		if (p0ExtraData->isSoundFinished(nSoundId)) {
			continue; // for ------------
		}
		p0ExtraData->setSoundFinished(nSoundId);
		//
		shared_ptr<Event> refEvent;
		sendSndFinishedEventToListener(oListenerData, nEventTimeUsec, nSoundStarted
										, SndFinishedEvent::FINISHED_TYPE_ABORTED
										, nSoundId, refCapability, p0Owner->m_nClassIdxSndFinishedEvent, refEvent);
	}
}


} // namespace OpenAl
} // namespace Private

} // namespace stmi
