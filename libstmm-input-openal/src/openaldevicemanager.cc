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
 * File:   openaldevicemanager.cc
 */

#include "openaldevicemanager.h"

#include "playbackdevice.h"
#include "openalbackend.h"

#include <stmm-input-au/playbackcapability.h>
#include <stmm-input-au/sndfinishedevent.h>

#include <stmm-input-ev/devicemgmtevent.h>
#include <stmm-input-ev/stddevicemanager.h>

#include <stmm-input/event.h>
#include <stmm-input/capability.h>
#include <stmm-input/devicemanager.h>

#include <cassert>
#include <algorithm>
#include <type_traits>
#include <limits>

#ifdef STMM_SNAP_PACKAGING
#include <array>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#endif //STMM_SNAP_PACKAGING

namespace stmi { class Accessor; }
namespace stmi { class ChildDeviceManager; }


namespace stmi
{

////////////////////////////////////////////////////////////////////////////////
using Private::OpenAl::Backend;
using Private::OpenAl::PlaybackDevice;
using Private::OpenAl::OpenAlListenerExtraData;


#ifdef STMM_SNAP_PACKAGING
static std::string getEnvString(const char* p0Name) noexcept
{
	const char* p0Value = ::secure_getenv(p0Name);
	std::string sValue{(p0Value == nullptr) ? "" : p0Value};
	return sValue;
}
static bool execCmd(const char* sCmd, std::string& sResult, std::string& sError) noexcept
{
	::fflush(nullptr);
	sError.clear();
	sResult.clear();
	std::array<char, 128> aBuffer;
	FILE* pFile = ::popen(sCmd, "r");
	if (pFile == nullptr) {
		sError = std::string("Error: popen() failed: ") + ::strerror(errno) + "(" + std::to_string(errno) + ")";
		return false; //--------------------------------------------------------
	}
	while (!::feof(pFile)) {
		if (::fgets(aBuffer.data(), sizeof(aBuffer), pFile) != nullptr) {
			sResult += aBuffer.data();
		}
	}
	const auto nRet = ::pclose(pFile);
	return (nRet == 0);
}
#endif //STMM_SNAP_PACKAGING

std::pair<shared_ptr<OpenAlDeviceManager>, std::string> OpenAlDeviceManager::create(const std::string& /*sAppName*/
																					, bool bEnableEventClasses, const std::vector<Event::Class>& aEnDisableEventClasses) noexcept
{
	#ifdef STMM_SNAP_PACKAGING
	{
	std::string sError;
	const std::string sSnapName = getEnvString("SNAP_NAME");
	if (sSnapName.empty()) {
		sError = "SNAP_NAME environment variable not defined!";
		return std::make_pair(shared_ptr<OpenAlDeviceManager>{}, std::move(sError)); //--------
	}
	std::string sResult;
	if (! execCmd("snapctl is-connected audio-playback", sResult, sError)) {
		sError = "Not allowed to use 'audio-playback' interface."
				 "\nPlease grant permission with 'sudo snap connect " + sSnapName + ":audio-playback :audio-playback'";
		return std::make_pair(shared_ptr<OpenAlDeviceManager>{}, std::move(sError)); //--------
	}
	}
	#endif //STMM_SNAP_PACKAGING
	shared_ptr<OpenAlDeviceManager> refInstance(new OpenAlDeviceManager(bEnableEventClasses, aEnDisableEventClasses));
	auto refBackend = Backend::create(refInstance.get());
	Backend* p0Backend = refBackend.get();
	assert(refBackend);
//std::cout << "OpenAlDeviceManager::create ok backend" << '\n';
	auto refSndMgmtImpl = std::make_shared<SndMgmtImpl>(refInstance.get());
	refInstance->init(std::move(refBackend), std::move(refSndMgmtImpl));

	std::string sError = p0Backend->createThread();
	if (! sError.empty()) {
//std::cout << "OpenAlDeviceManager::create error backend" << '\n';
		return std::make_pair(shared_ptr<OpenAlDeviceManager>{}, std::move(sError)); //--------
	}
	return std::make_pair(refInstance, "");
}

OpenAlDeviceManager::OpenAlDeviceManager(bool bEnableEventClasses, const std::vector<Event::Class>& aEnDisableEventClasses) noexcept
: StdDeviceManager({Capability::Class{typeid(PlaybackCapability)}}
					, {Event::Class{typeid(DeviceMgmtEvent)}, Event::Class{typeid(SndFinishedEvent)}}
					, bEnableEventClasses, aEnDisableEventClasses)
, m_nDefaultBackendDeviceId(-1)
, m_nFinishingNestedDepth(0)
, m_nClassIdxSndFinishedEvent(getEventClassIndex(Event::Class{typeid(SndFinishedEvent)}))
{
//std::cout << "OpenAlDeviceManager::OpenAlDeviceManager " << reinterpret_cast<int64_t>(this) << '\n';
}

void OpenAlDeviceManager::init(std::unique_ptr<Private::OpenAl::Backend>&& refBackend, shared_ptr<SndMgmtImpl>&& refSndMgmtImpl) noexcept
{
	assert(refBackend);
	m_refBackend = std::move(refBackend);
	m_refSndMgmtImpl = std::move(refSndMgmtImpl);
}

void OpenAlDeviceManager::enableEventClass(const Event::Class& oEventClass) noexcept
{
	StdDeviceManager::enableEventClass(oEventClass);
}

std::vector<Capability::Class> OpenAlDeviceManager::getCapabilityClasses() const noexcept
{
	auto aCapaClasses = StdDeviceManager::getCapabilityClasses();
	aCapaClasses.push_back(SndMgmtCapability::getClass());
	return aCapaClasses;
}
shared_ptr<Capability> OpenAlDeviceManager::getCapability(const Capability::Class& oClass) const noexcept
{
	if (oClass != typeid(SndMgmtCapability)) {
		return StdDeviceManager::getCapability(oClass);
	}
	return m_refSndMgmtImpl;
}
shared_ptr<Capability> OpenAlDeviceManager::getCapability(int32_t nCapabilityId) const noexcept
{
	const auto nSndMgmtCapaId = m_refSndMgmtImpl->getId();
	if (nCapabilityId != nSndMgmtCapaId) {
		return StdDeviceManager::getCapability(nCapabilityId);
	}
	return m_refSndMgmtImpl;
}

shared_ptr<DeviceManager> OpenAlDeviceManager::SndMgmtImpl::getDeviceManager() const noexcept
{
	shared_ptr<ChildDeviceManager> refChildThis = std::const_pointer_cast<ChildDeviceManager>(m_p1Owner->shared_from_this());
	shared_ptr<OpenAlDeviceManager> refThis = std::static_pointer_cast<OpenAlDeviceManager>(refChildThis);
	return refThis;
}
int32_t OpenAlDeviceManager::SndMgmtImpl::getMaxPlaybackDevices() const noexcept
{
	return std::numeric_limits<int32_t>::max() / 2;
}
shared_ptr<PlaybackCapability> OpenAlDeviceManager::SndMgmtImpl::getDefaultPlayback() const noexcept
{
	//assert((m_nDefaultBackendDeviceId >= 0) && (m_nDefaultBackendDeviceId < static_cast<int32_t>(m_aPlaybackDevices.size())));
	const int32_t nDefaultBackendDeviceId = m_p1Owner->m_nDefaultBackendDeviceId;
	if (nDefaultBackendDeviceId < 0) {
		return shared_ptr<PlaybackCapability>{};
	}
	return m_p1Owner->m_aPlaybackDevices[nDefaultBackendDeviceId];
}
bool OpenAlDeviceManager::SndMgmtImpl::supportsSpatialSounds() const noexcept
{
	return true;
}
void OpenAlDeviceManager::onDeviceAdded(std::string&& sName, int32_t nBackendDeviceId, bool bIsDefault) noexcept
{
//std::cout << "OpenAlDeviceManager::onDeviceAdded " << reinterpret_cast<int64_t>(this) << '\n';
	assert(nBackendDeviceId >= 0);
//std::cout << "OpenAlDeviceManager::onDeviceAdded sName=" << sName << "  nBackendDeviceId=" << nBackendDeviceId << '\n';
	shared_ptr<ChildDeviceManager> refChildThis = shared_from_this();
	assert(std::dynamic_pointer_cast<OpenAlDeviceManager>(refChildThis));
	auto refThis = std::static_pointer_cast<OpenAlDeviceManager>(refChildThis);
	//
	assert(m_refBackend);
	auto refNewDevice = std::make_shared<PlaybackDevice>(sName, refThis, *m_refBackend, nBackendDeviceId, bIsDefault);
	#ifndef NDEBUG
	auto itFind = std::find_if(m_aPlaybackDevices.begin(), m_aPlaybackDevices.end()
					, [&](const shared_ptr<Private::OpenAl::PlaybackDevice>& refDevice)
					{
						return refDevice && (refDevice->getDeviceId() == refNewDevice->getDeviceId());
					});
	#endif
	assert(itFind == m_aPlaybackDevices.end());
	//
	if (nBackendDeviceId >= static_cast<int32_t>(m_aPlaybackDevices.size())) {
		m_aPlaybackDevices.resize(nBackendDeviceId + 1);
	}
	assert(! m_aPlaybackDevices[nBackendDeviceId]);
	m_aPlaybackDevices[nBackendDeviceId] = refNewDevice;
	if (bIsDefault) {
		m_nDefaultBackendDeviceId = nBackendDeviceId;
	}
	//
//std::cout << "OpenAlDeviceManager::onDeviceAdded id=" << refNewDevice->Device::getId() << "  dev id=" << refNewDevice->getDeviceId() << '\n';
	#ifndef NDEBUG
	const bool bAdded = 
	#endif
	StdDeviceManager::addDevice(refNewDevice);
	assert(bAdded);
	sendDeviceMgmtToListeners(DeviceMgmtEvent::DEVICE_MGMT_ADDED, refNewDevice);
}
void OpenAlDeviceManager::onDeviceRemoved(int32_t nBackendDeviceId) noexcept
{
//std::cout << "OpenAlDeviceManager::onDeviceRemoved nBackendDeviceId=" << nBackendDeviceId << '\n';
	assert((nBackendDeviceId >= 0) && (nBackendDeviceId < static_cast<int32_t>(m_aPlaybackDevices.size())));
	shared_ptr<PlaybackDevice>& refPlaybackDevice = m_aPlaybackDevices[nBackendDeviceId];
	assert(refPlaybackDevice);
	//
	finishDeviceSounds(refPlaybackDevice);
	refPlaybackDevice->removingDevice();

//std::cout << "OpenAlDeviceManager::onDeviceRemoved id=" << refPlaybackDevice->Device::getId() << "  dev id=" << refPlaybackDevice->getDeviceId() << '\n';
	auto refRemovedPlaybackDevice = refPlaybackDevice; // copy!
	#ifndef NDEBUG
	const bool bRemoved =
	#endif //NDEBUG
	StdDeviceManager::removeDevice(refPlaybackDevice);
	assert(bRemoved);
	m_aPlaybackDevices[nBackendDeviceId].reset();
	if (nBackendDeviceId == m_nDefaultBackendDeviceId) {
		m_nDefaultBackendDeviceId = -1;
	}
	//
//std::cout << "OpenAlDeviceManager::onDeviceRemoved " << refRemovedPlaybackDevice->getDeviceId() << '\n';
	sendDeviceMgmtToListeners(DeviceMgmtEvent::DEVICE_MGMT_REMOVED, refRemovedPlaybackDevice);
}
void OpenAlDeviceManager::onDeviceChanged(int32_t nBackendDeviceId, bool bIsDefault) noexcept
{
	assert((nBackendDeviceId >= 0) && (nBackendDeviceId < static_cast<int32_t>(m_aPlaybackDevices.size())));
	shared_ptr<PlaybackDevice>& refPlaybackDevice = m_aPlaybackDevices[nBackendDeviceId];
	assert(refPlaybackDevice);

	if (bIsDefault) {
		m_nDefaultBackendDeviceId = nBackendDeviceId;
	} else if (nBackendDeviceId == m_nDefaultBackendDeviceId) {
		m_nDefaultBackendDeviceId = -1;
	}

	refPlaybackDevice->setIsDefault(bIsDefault);

	sendDeviceMgmtToListeners(DeviceMgmtEvent::DEVICE_MGMT_CHANGED, refPlaybackDevice);
}
void OpenAlDeviceManager::onPlayFinished(int32_t nBackendDeviceId, int32_t nSoundId) noexcept
{
	assert((nBackendDeviceId >= 0) && (nBackendDeviceId < static_cast<int32_t>(m_aPlaybackDevices.size())));
	shared_ptr<PlaybackDevice>& refPlaybackDevice = m_aPlaybackDevices[nBackendDeviceId];
	assert(refPlaybackDevice);

	refPlaybackDevice->onSoundFinished(nSoundId);
}
void OpenAlDeviceManager::onDeviceError(int32_t nBackendDeviceId, int32_t nFileId, int32_t nSoundId, std::string&& sError) noexcept
{
	assert((nBackendDeviceId >= 0) && (nBackendDeviceId < static_cast<int32_t>(m_aPlaybackDevices.size())));
	shared_ptr<PlaybackDevice>& refPlaybackDevice = m_aPlaybackDevices[nBackendDeviceId];
	assert(refPlaybackDevice);

	refPlaybackDevice->onDeviceError(nSoundId, nFileId, std::move(sError));
}
bool OpenAlDeviceManager::addAccessor(const shared_ptr<Accessor>& /*refAccessor*/) noexcept
{
	return false;
}
bool OpenAlDeviceManager::removeAccessor(const shared_ptr<Accessor>& /*refAccessor*/) noexcept
{
	return false;
}
bool OpenAlDeviceManager::hasAccessor(const shared_ptr<Accessor>& /*refAccessor*/) noexcept
{
	return false;
}

void OpenAlDeviceManager::finalizeListener(ListenerData& oListenerData) noexcept
{
	++m_nFinishingNestedDepth;
	const int64_t nEventTimeUsec = DeviceManager::getNowTimeMicroseconds();
	for (auto& refPlaybackDevice : m_aPlaybackDevices) {
		refPlaybackDevice->finalizeListener(oListenerData, nEventTimeUsec);
	}
	--m_nFinishingNestedDepth;
	if (m_nFinishingNestedDepth == 0) {
		resetExtraDataOfAllListeners();
	}
}
void OpenAlDeviceManager::finishDeviceSounds(const shared_ptr<Private::OpenAl::PlaybackDevice>& refPlaybackDevice) noexcept
{
//std::cout << "BtGtkDeviceManager::cancelDeviceKeys  m_nFinishingNestedDepth=" << m_nFinishingNestedDepth << '\n';
	++m_nFinishingNestedDepth;
	//
	refPlaybackDevice->finishDeviceSounds();
	//
	--m_nFinishingNestedDepth;
	if (m_nFinishingNestedDepth == 0) {
		resetExtraDataOfAllListeners();
	}
}

} // namespace stmi
