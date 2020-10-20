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
 * File:   playbackcapability.h
 */

#ifndef STMI_PLAYBACK_CAPABILITY_H
#define STMI_PLAYBACK_CAPABILITY_H

#include <stmm-input/capability.h>

#include <string>

#include <stdint.h>

namespace stmi
{

/** The playback capability of a device.
 * Devices with this capability can generate "sound finished" events.
 *
 * Note: the reference system is flipped by 180 degrees around the x axis
 * compared with the coordinates of your typical screen. That is you should
 * negate y and z coordinates when setting the position of listener and sounds.
 *
 * This interface is very simple and limited compared to what OpenAL can do.
 * The orientation of the listener cannot be changed, the direction of sounds
 * and their sound cone cannot be set, etc.
 *
 * If the listener is at position (0,0,0), the up direction is (0,1,0), the
 * left direction is (-1,0,0), the behind direction is (0,0,1).
 *
 * The user of this interface can assume that the listener's initial position
 * is (0,0,0) and the listener volume is 1.
 *
 * When one of the playSound methods is called, an event listener (added to the
 * device manager managing the device implementing this capability) can expect
 * to receive a SndFinishedEvent when the sound has finished playing or the
 * device was removed. A listeners being removed will receive the SndFinishedEvent
 * only if it was added with the finalization flag.
 */
class PlaybackCapability : public Capability
{
public:
	/** Return data type.
	 */
	struct SoundData
	{
		int32_t m_nSoundId = -1; /**< The sound id. Can be used to position, pause, resume the sound. If negative, error occurred. Default is -1. */
		int32_t m_nFileId = -1; /**< The file id. Allows to play the file or buffer again
									 * possibly more efficiently (using cache). Negative if error. Default is -1. */
	};
public:
	/** Pre-load sound file.
	 * The returned file id can be used with the playSound method.
	 * @param sFileName The absolute path of the sound file. Cannot be empty.
	 * @return The file id or negative if error (ex. device removed).
	 */
	virtual int32_t preloadSound(const std::string& sFileName) noexcept = 0;
	/** Pre-load sound buffer.
	 * The returned file id can be used with the playSound method.
	 * @param p0Buffer The pointer to a buffer. Cannot be null.
	 * @param nBufferSize The size of the buffer. Cannot be negative.
	 * @return The file id or negative if error (ex. device removed).
	 */
	virtual int32_t preloadSound(uint8_t const* p0Buffer, int32_t nBufferSize) noexcept = 0;
	/** Play sound file at a given position.
	 * @param sFileName The absolute path of the sound file. Cannot be empty.
	 * @param bRelative Whether the position is relative to the listener.
	 * @param fVolume The volume (0.0 inaudible, 1.0 maximum).
	 * @param bLoop Whether to loop.
	 * @param fX The x coord.
	 * @param fY The y coord.
	 * @param fZ The z coord.
	 * @return The sound data.
	 */
	virtual SoundData playSound(const std::string& sFileName, double fVolume, bool bLoop
								, bool bRelative, double fX, double fY, double fZ) noexcept = 0;
	/** Play sound buffer at a given position.
	 * @param p0Buffer The pointer to a buffer. Cannot be null.
	 * @param nBufferSize The size of the buffer. Cannot be negative.
	 * @param fVolume The volume (0.0 inaudible, 1.0 maximum).
	 * @param bLoop Whether to loop.
	 * @param bRelative Whether the position is relative to the listener.
	 * @param fX The x coord.
	 * @param fY The y coord.
	 * @param fZ The z coord.
	 * @return The sound data.
	 */
	virtual SoundData playSound(uint8_t const* p0Buffer, int32_t nBufferSize, double fVolume, bool bLoop
								, bool bRelative, double fX, double fY, double fZ) noexcept = 0;
	/** Play previously played or pre-loaded file or buffer at a given position.
	 * The nFileId is only valid within the instance that created it.
	 * @param nFileId The id of the previously played file or buffer to play as a new sound.
	 * @param fVolume The volume (0.0 inaudible, 1.0 maximum).
	 * @param bLoop Whether to loop.
	 * @param bRelative Whether the position is relative to the listener.
	 * @param fX The x coord.
	 * @param fY The y coord.
	 * @param fZ The z coord.
	 * @return The sound id or negative if error.
	 */
	virtual int32_t playSound(int32_t nFileId, double fVolume, bool bLoop
							, bool bRelative, double fX, double fY, double fZ) noexcept = 0;
	/** Play sound file at current listener position.
	 * The sound is played at maximum volume.
	 * @param sFileName The absolute path of the sound file. Cannot be empty.
	 * @return The sound data.
	 */
	SoundData playSound(const std::string& sFileName) noexcept;
	/** Play sound buffer at current listener position.
	 * The sound is played at maximum volume.
	 * @param p0Buffer The pointer to a buffer. Cannot be null.
	 * @param nBufferSize The size of the buffer. Cannot be negative.
	 * @return The sound data.
	 */
	SoundData playSound(uint8_t* p0Buffer, int32_t nBufferSize) noexcept;
	/** Play previously played or pre-loaded file or buffer at current listener position.
	 * The sound is played at maximum volume.
	 *
	 * The nFileId is only valid within the instance that created it.
	 * @param nFileId The id of the previously played file or buffer to play as a new sound.
	 * @return The sound id or negative if error.
	 */
	int32_t playSound(int32_t nFileId) noexcept;

	/** Set position of a currently playing sound.
	 * @param nSoundId The sound id.
	 * @param bRelative Whether the position is relative to the listener.
	 * @param fX The x coord.
	 * @param fY The y coord.
	 * @param fZ The z coord.
	 * @return Whether could set the position.
	 */
	virtual bool setSoundPos(int32_t nSoundId, bool bRelative, double fX, double fY, double fZ) noexcept = 0;
	/** Set volume of a currently playing sound.
	 * @param nSoundId The sound id.
	 * @param fVolume The volume (0.0 inaudible, 1.0 maximum).
	 * @return Whether could set the volume.
	 */
	virtual bool setSoundVol(int32_t nSoundId, double fVolume) noexcept = 0;

	/** Set listener position.
	 * @param fX The x coord.
	 * @param fY The y coord.
	 * @param fZ The z coord.
	 * @return Whether could set the position.
	 */
	virtual bool setListenerPos(double fX, double fY, double fZ) noexcept = 0;
	/** Set listener volume.
	 * Values outside the 0.0 to 1.0 range are clamped.
	 * @param fVolume The volume. A number from 0.0 (inaudible) to 1.0 (maximum).
	 * @return Whether could set the volume.
	 */
	virtual bool setListenerVol(double fVolume) noexcept = 0;
	//
	/** Pause a sound.
	 * If called when the device is paused with pauseDevice(), the sound will
	 * not resume if resumeAllSounds() is called.
	 * @param nSoundId The sound id to pause.
	 * @return Whether the sound was not paused (independent from device pausing).
	 */
	virtual bool pauseSound(int32_t nSoundId) noexcept = 0;
	/** Resume a sound.
	 * If called when the device is paused with pauseDevice(), the sound will
	 * resume only when resumeAllSounds() is called.
	 * @param nSoundId The sound id to resume.
	 * @return Whether the sound was paused (independent from device pausing).
	 */
	virtual bool resumeSound(int32_t nSoundId) noexcept = 0;
	/** Stop a sound.
	 * Note: no SndFinishedEvent is sent to listeners.
	 * @param nSoundId The sound id to stop.
	 * @return Whether the sound existed.
	 */
	virtual bool stopSound(int32_t nSoundId) noexcept = 0;

	/** Pause playback of device.
	 * Subsequent resumeSound on already paused sounds have effect as soon as
	 * resumeAllSounds is called.
	 * @return If playback was already paused false, true otherwise.
	 */
	virtual bool pauseDevice() noexcept = 0;
	/** Resume playback of device.
	 * @return If playback was not paused false, true otherwise.
	 */
	virtual bool resumeDevice() noexcept = 0;
	/** Stops all sounds of device.
	 * Note: no SndFinishedEvent is sent to listeners.
	 */
	virtual void stopAllSounds() noexcept = 0;

	/** Tells whether this is the capability of the default playback device.
	 * Only one of the playback devices of a device manager should be default.
	 * @return Whether default.
	 */
	virtual bool isDefaultDevice() noexcept = 0;
	//
	static const char* const s_sClassId;
	static const Capability::Class& getClass() noexcept { return s_oInstall.getCapabilityClass(); }
protected:
	PlaybackCapability() noexcept : Capability(s_oInstall.getCapabilityClass()) {}
private:
	static RegisterClass<PlaybackCapability> s_oInstall;
};

} // namespace stmi

#endif /* STMI_PLAYBACK_CAPABILITY_H */
