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
 * File:   openaldevicemanager.h
 */

#ifndef STMI_OPENAL_DEVICE_MANAGER_H
#define STMI_OPENAL_DEVICE_MANAGER_H

#include <stmm-input-au/sndmgmtcapability.h>

#include <stmm-input-ev/stddevicemanager.h>

#include <stmm-input/event.h>

#include <vector>
#include <string>
#include <memory>
#include <utility>

#include <stdint.h>

namespace stmi { class Accessor; }


namespace stmi
{

using std::shared_ptr;
using std::weak_ptr;

namespace Private
{
namespace OpenAl
{
	class Backend;
	class PlaybackDevice;
	class OpenAlListenerExtraData;
} // namespace OpenAl
} // namespace Private

/** Handles OpenAL playback devices.
 * These devices implement the stmi::PlaybackCapability interface.
 *
 * The current implementation is not very efficient.
 */
class OpenAlDeviceManager : public StdDeviceManager //, public sigc::trackable
{
public:
	/** Creates an instance of this class.
	 *
	 * If bEnableEventClasses is `true` then all event classes in aEnDisableEventClasses are enabled, all others disabled,
	 * if `false` then all event classes supported by this instance are enabled except those in aEnDisableEventClasses.
	 * OpenAlDeviceManager doesn't allow disabling event classes once constructed, only enabling.
	 *
	 * Example: To enable all the event classes supported by this instance (currently just SndFinishedEvent) pass
	 *
	 *     bEnableEventClasses = false,  aEnDisableEventClasses = {}
	 *
	 * @param sAppName The application name. Can be empty.
	 * @param bEnableEventClasses Whether to enable or disable all but aEnDisableEventClasses.
	 * @param aEnDisableEventClasses The event classes to be enabled or disabled according to bEnableEventClasses.
	 * @return The created instance and an empty string or null and an error string.
	 */
	static std::pair<shared_ptr<OpenAlDeviceManager>, std::string> create(const std::string& sAppName
																		, bool bEnableEventClasses, const std::vector<Event::Class>& aEnDisableEventClasses) noexcept;

	void enableEventClass(const Event::Class& oEventClass) noexcept override;

	std::vector<Capability::Class> getCapabilityClasses() const noexcept override;
	shared_ptr<Capability> getCapability(const Capability::Class& oClass) const noexcept override;
	shared_ptr<Capability> getCapability(int32_t nCapabilityId) const noexcept override;

	bool addAccessor(const shared_ptr<Accessor>& refAccessor) noexcept override;
	bool removeAccessor(const shared_ptr<Accessor>& refAccessor) noexcept override;
	bool hasAccessor(const shared_ptr<Accessor>& refAccessor) noexcept override;

private:
	friend class SndMgmtImpl;
	class SndMgmtImpl : public SndMgmtCapability
	{
	public:
		SndMgmtImpl(OpenAlDeviceManager* p1Owner) noexcept
		: m_p1Owner(p1Owner)
		{
		}
		int32_t getMaxPlaybackDevices() const noexcept override;
		shared_ptr<PlaybackCapability> getDefaultPlayback() const noexcept override;
		bool supportsSpatialSounds() const noexcept override;
		shared_ptr<DeviceManager> getDeviceManager() const noexcept override;
	private:
		OpenAlDeviceManager* m_p1Owner;
	};
protected:
	void finalizeListener(ListenerData& oListenerData) noexcept override;
	/** Constructor.
	 * @see create()
	 */
	OpenAlDeviceManager(bool bEnableEventClasses, const std::vector<Event::Class>& aEnDisableEventClasses) noexcept;
private:
	void init(std::unique_ptr<Private::OpenAl::Backend>&& refBackend, shared_ptr<SndMgmtImpl>&& refSndMgmtImpl) noexcept;

	friend class Private::OpenAl::Backend;
	void onPlayFinished(int32_t nBackendDeviceId, int32_t nSoundId) noexcept;
	void onDeviceAdded(std::string&& sName, int32_t nBackendDeviceId, bool bIsDefault) noexcept;
	void onDeviceRemoved(int32_t nBackendDeviceId) noexcept;
	void onDeviceChanged(int32_t nBackendDeviceId, bool bIsDefault) noexcept;
	void onDeviceError(int32_t nBackendDeviceId, int32_t nFileId, int32_t nSoundId, std::string&& sError) noexcept;

	void finishDeviceSounds(const shared_ptr<Private::OpenAl::PlaybackDevice>& refPlaybackDevice) noexcept;

	friend class Private::OpenAl::PlaybackDevice;
	friend class Private::OpenAl::OpenAlListenerExtraData;
private:
	std::unique_ptr<Private::OpenAl::Backend> m_refBackend;
	std::shared_ptr<SndMgmtImpl> m_refSndMgmtImpl;
	//
	std::vector< shared_ptr<Private::OpenAl::PlaybackDevice> > m_aPlaybackDevices; // Index: nBackendDeviceId, Value: can be null!

	int32_t m_nDefaultBackendDeviceId; // Index into m_aPlaybackDevices or -1 if no device or no default

	int32_t m_nFinishingNestedDepth;
	//
	const int32_t m_nClassIdxSndFinishedEvent;
private:
	OpenAlDeviceManager(const OpenAlDeviceManager& oSource) = delete;
	OpenAlDeviceManager& operator=(const OpenAlDeviceManager& oSource) = delete;
};

} // namespace stmi

#endif /* STMI_OPENAL_DEVICE_MANAGER_H */

