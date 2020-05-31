/*
 * Copyright © 2020  Stefano Marsili, <stemars@gmx.ch>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <http://www.gnu.org/licenses/>
 */
/*
 * File:   wanderwindow.h
 */

#ifndef _EXAMPLE_WANDER_WINDOW_
#define _EXAMPLE_WANDER_WINDOW_

#include "wanderdata.h"
#include "wanderdrawingarea.h"

//#include <stmm-input-au/sndfinishedevent.h>

#include <stmm-input-ev/stmm-input-ev.h>

#include <gtkmm.h>
#include <gdkmm.h>

#include <string>
#include <memory>

namespace stmi { class SndFinishedEvent; }
namespace stmi { class DeviceMgmtEvent; }

namespace example
{

namespace wander
{

using std::shared_ptr;
using std::weak_ptr;

class WanderWindow : public Gtk::Window
{
public:
	WanderWindow(const shared_ptr<stmi::DeviceManager>& refDM, const Glib::ustring sTitle
				, int32_t nTotSources, std::vector<std::string>&& aSoundsPathFiles) noexcept;
private:
	friend class WanderDrawingArea;

	void initSources(int32_t nTotSources) noexcept;
	void initDevices() noexcept;
	int32_t getCapabilityDataIndexFromDeviceId(int32_t nDeviceId) const noexcept;
	int32_t getDeviceIdFromCapability(const stmi::PlaybackCapability* p0Playback) const noexcept;

	void setCapabilityWherePossible() noexcept;

	void handleSndFinishedEvents(const shared_ptr<stmi::Event>& refEvent) noexcept;
	void handleDeviceMgmtEvents(const shared_ptr<stmi::Event>& refEvent) noexcept;

	void printStringToEvents(const std::string& sStr) noexcept;
	void printToEvents(stmi::DeviceMgmtEvent& refEvent) noexcept;
	void printToEvents(stmi::SndFinishedEvent& refEvent) noexcept;

	void recreateDevicesLists() noexcept;
	void recreateSourcesList() noexcept;
	void selectDeviceInDeviceLists() noexcept;

	void refreshCurrentSource() noexcept;
	void refreshCurrentShow() noexcept;

	void redrawContents() noexcept;

	void onSpinAreaXChanged() noexcept;
	void onSpinAreaYChanged() noexcept;
	void onSpinAreaWChanged() noexcept;
	void onSpinAreaHChanged() noexcept;

	void onShowDeviceSelectionChanged() noexcept;

	void onSpinListenerPosXChanged() noexcept;
	void onSpinListenerPosYChanged() noexcept;
	void onSpinListenerPosZChanged() noexcept;

	//~ void onSpinListenerDirXChanged() noexcept;
	//~ void onSpinListenerDirYChanged() noexcept;
	//~ void onSpinListenerDirZChanged() noexcept;

	void onSpinListenerVolChanged() noexcept;

	void onButtonDevicePause() noexcept;
	void onButtonDeviceResume() noexcept;

	void onSourceSelectionChanged() noexcept;
	void onSpinFilenameChanged() noexcept;
	void onSoundLoopingToggled() noexcept;

	void onSpinSourcePosXChanged() noexcept;
	void onSpinSourcePosYChanged() noexcept;
	void onSpinSourcePosZChanged() noexcept;

	void onSoundListenerRelativeToggled() noexcept;

	void onSpinSourceVolChanged() noexcept;

	void onButtonSoundPlay() noexcept;
	void onButtonSoundPause() noexcept;
	void onButtonSoundStop() noexcept;

	void onButtonSoundsStopAll() noexcept;
	void onDeviceSelectionChanged() noexcept;

private:
	const shared_ptr<stmi::DeviceManager>& m_refDM;

	WanderData m_oWanderData;

	WanderData::CapabilityData m_oDummyCapabilityData;

	bool m_bRecreateDevicesListInProgress = false;
	bool m_bRecreateSourcesListInProgress = false;

	shared_ptr<stmi::EventListener> m_refListenerSndFinishedEvents;
	shared_ptr<stmi::EventListener> m_refListenerDeviceMgmtEvents;
	shared_ptr<stmi::CallIf> m_refCallIfSndFinished;
	shared_ptr<stmi::CallIf> m_refCallIfDeviceMgmt;

	Glib::RefPtr<Gtk::Adjustment> m_refAdjustmentAreaX;
	Glib::RefPtr<Gtk::Adjustment> m_refAdjustmentAreaY;
	Glib::RefPtr<Gtk::Adjustment> m_refAdjustmentAreaW;
	Glib::RefPtr<Gtk::Adjustment> m_refAdjustmentAreaH;

	Glib::RefPtr<Gtk::Adjustment> m_refAdjustmentListenerPosX;
	Glib::RefPtr<Gtk::Adjustment> m_refAdjustmentListenerPosY;
	Glib::RefPtr<Gtk::Adjustment> m_refAdjustmentListenerPosZ;

	//~ Glib::RefPtr<Gtk::Adjustment> m_refAdjustmentListenerDirX;
	//~ Glib::RefPtr<Gtk::Adjustment> m_refAdjustmentListenerDirY;
	//~ Glib::RefPtr<Gtk::Adjustment> m_refAdjustmentListenerDirZ;

	Glib::RefPtr<Gtk::Adjustment> m_refAdjustmentListenerVol;

	Glib::RefPtr<Gtk::Adjustment> m_refAdjustmentSourcePosX;
	Glib::RefPtr<Gtk::Adjustment> m_refAdjustmentSourcePosY;
	Glib::RefPtr<Gtk::Adjustment> m_refAdjustmentSourcePosZ;

	Glib::RefPtr<Gtk::Adjustment> m_refAdjustmentSourceVol;

	Glib::RefPtr<Gtk::Adjustment> m_refAdjustmentFilename;

	Gtk::Paned* m_p0HPanedMain = nullptr;
	Gtk::Paned* m_p0HPanedShow = nullptr;
	//Gtk::Box* m_p0HBoxMain = nullptr;
		Gtk::Box* m_p0VBoxListener = nullptr;
			Gtk::Box* m_p0VBoxArea = nullptr;
				Gtk::Label* m_p0LabelShow = nullptr;
				Gtk::Box* m_p0HBoxAreaX = nullptr;
					Gtk::Label* m_p0LabelAreaX = nullptr;
					Gtk::SpinButton* m_p0SpinAreaX = nullptr;
				Gtk::Box* m_p0HBoxAreaY = nullptr;
					Gtk::Label* m_p0LabelAreaY = nullptr;
					Gtk::SpinButton* m_p0SpinAreaY = nullptr;
				Gtk::Box* m_p0HBoxAreaW = nullptr;
					Gtk::Label* m_p0LabelAreaW = nullptr;
					Gtk::SpinButton* m_p0SpinAreaW = nullptr;
				Gtk::Box* m_p0HBoxAreaH = nullptr;
					Gtk::Label* m_p0LabelAreaH = nullptr;
					Gtk::SpinButton* m_p0SpinAreaH = nullptr;
			Gtk::Separator* m_p0HSeparatorArea = nullptr;
			Gtk::Box* m_p0VBoxShowDevicesList = nullptr;
				Gtk::Label* m_p0LabelShowDevicesList = nullptr;
				Gtk::ScrolledWindow* m_p0ScrolledShowDevicesList = nullptr;
					Gtk::TreeView* m_p0TreeViewShowDevicesList = nullptr;
			Gtk::Separator* m_p0HSeparatorShowDevice = nullptr;
			Gtk::Box* m_p0VBoxListenerPos = nullptr;
				Gtk::Label* m_p0LabelListenerPos = nullptr;
				Gtk::Box* m_p0HBoxListenerPosX = nullptr;
					Gtk::Label* m_p0LabelListenerPosX = nullptr;
					Gtk::SpinButton* m_p0SpinListenerPosX = nullptr;
				Gtk::Box* m_p0HBoxListenerPosY = nullptr;
					Gtk::Label* m_p0LabelListenerPosY = nullptr;
					Gtk::SpinButton* m_p0SpinListenerPosY = nullptr;
				Gtk::Box* m_p0HBoxListenerPosZ = nullptr;
					Gtk::Label* m_p0LabelListenerPosZ = nullptr;
					Gtk::SpinButton* m_p0SpinListenerPosZ = nullptr;
			Gtk::Separator* m_p0HSeparatorListenerPosDir = nullptr;
			//~ Gtk::Box* m_p0VBoxListenerDir = nullptr;
				//~ Gtk::Label* m_p0LabelListenerDir = nullptr;
				//~ Gtk::Box* m_p0HBoxListenerDirX = nullptr;
					//~ Gtk::Label* m_p0LabelListenerDirX = nullptr;
					//~ Gtk::SpinButton* m_p0SpinListenerDirX = nullptr;
				//~ Gtk::Box* m_p0HBoxListenerDirY = nullptr;
					//~ Gtk::Label* m_p0LabelListenerDirY = nullptr;
					//~ Gtk::SpinButton* m_p0SpinListenerDirY = nullptr;
				//~ Gtk::Box* m_p0HBoxListenerDirZ = nullptr;
					//~ Gtk::Label* m_p0LabelListenerDirZ = nullptr;
					//~ Gtk::SpinButton* m_p0SpinListenerDirZ = nullptr;
			Gtk::Box* m_p0HBoxListenerVol = nullptr;
				Gtk::Label* m_p0LabelListenerVol = nullptr;
				Gtk::SpinButton* m_p0SpinListenerVol = nullptr;
			Gtk::Separator* m_p0HSeparatorListenerVolEvents = nullptr;
			Gtk::Box* m_p0VBoxDeviceActions = nullptr;
				Gtk::Button* m_p0ButtonPauseDevice = nullptr;
				Gtk::Button* m_p0ButtonResumeDevice = nullptr;
			Gtk::Separator* m_p0HSeparatorPauseDevice = nullptr;
			Gtk::Box* m_p0VBoxEvents = nullptr;
				Gtk::Label* m_p0LabelEvents = nullptr;
				Gtk::ScrolledWindow* m_p0ScrolledEvents = nullptr;
					Gtk::TextView* m_p0TextViewEvents;

		Gtk::Box* m_p0VBoxShowArea = nullptr;
			WanderDrawingArea* m_p0DrawingAreaShow;

		Gtk::Box* m_p0VBoxSources = nullptr;
			Gtk::Box* m_p0VBoxSourcesList = nullptr;
				Gtk::Label* m_p0LabelSourcesList = nullptr;
				Gtk::ScrolledWindow* m_p0ScrolledSourcesList = nullptr;
					Gtk::TreeView* m_p0TreeViewSourcesList = nullptr;
			Gtk::Separator* m_p0HSeparatorSourcesList = nullptr;
			Gtk::Box* m_p0VBoxSelSource = nullptr;
				Gtk::Label* m_p0LabelSourceName = nullptr;
				Gtk::Label* m_p0LabelDeviceId = nullptr;
				Gtk::Box* m_p0VBoxFilename = nullptr;
					Gtk::Box* m_p0HBoxFilenameIdx = nullptr;
						Gtk::Label* m_p0LabelFilenameIdx = nullptr;
						Gtk::SpinButton* m_p0SpinFilenameIdx = nullptr;
					Gtk::Label* m_p0LabelFilePath = nullptr;
				Gtk::CheckButton* m_p0CheckButtonLooping;
				Gtk::Box* m_p0HBoxSoundId = nullptr;
					Gtk::Label* m_p0LabelSoundIdLabel = nullptr;
					Gtk::Label* m_p0LabelSoundId = nullptr;
				Gtk::Box* m_p0VBoxSelSourcePos = nullptr;
					Gtk::Label* m_p0LabelSourcePos = nullptr;
					Gtk::Box* m_p0HBoxSourcePosX = nullptr;
						Gtk::Label* m_p0LabelSourcePosX = nullptr;
						Gtk::SpinButton* m_p0SpinSourcePosX = nullptr;
					Gtk::Box* m_p0HBoxSourcePosY = nullptr;
						Gtk::Label* m_p0LabelSourcePosY = nullptr;
						Gtk::SpinButton* m_p0SpinSourcePosY = nullptr;
					Gtk::Box* m_p0HBoxSourcePosZ = nullptr;
						Gtk::Label* m_p0LabelSourcePosZ = nullptr;
						Gtk::SpinButton* m_p0SpinSourcePosZ = nullptr;
					Gtk::CheckButton* m_p0CheckButtonListenerRelative;
				Gtk::Box* m_p0HBoxSourceVol = nullptr;
					Gtk::Label* m_p0LabelSourceVol = nullptr;
					Gtk::SpinButton* m_p0SpinSourceVol = nullptr;
			Gtk::Box* m_p0VBoxSoundActions = nullptr;
				Gtk::Button* m_p0ButtonPlaySound = nullptr;
				Gtk::Button* m_p0ButtonPauseSound = nullptr;
				Gtk::Button* m_p0ButtonStopSound = nullptr;
			Gtk::Separator* m_p0HSeparatorStopAll = nullptr;
			Gtk::Box* m_p0VBoxDevicesList = nullptr;
				Gtk::Label* m_p0LabelDevicesList = nullptr;
				Gtk::ScrolledWindow* m_p0ScrolledDevicesList = nullptr;
					Gtk::TreeView* m_p0TreeViewDevicesList = nullptr;
			Gtk::Separator* m_p0HSeparatorSource = nullptr;
			Gtk::Button* m_p0ButtonStopAllSounds = nullptr;


	Glib::RefPtr<Gtk::TextBuffer> m_refTextBufferEvents;
	Glib::RefPtr<Gtk::TextBuffer::Mark> m_refTextBufferEventsBottomMark;
	int32_t m_nTextBufferEventsTotLines = 0;
	static constexpr int32_t s_nTextBufferEventsMaxLines = 1000;

	class SourcesColumns : public Gtk::TreeModel::ColumnRecord
	{
	public:
		SourcesColumns()
		{
			add(m_oColSourceId);
			add(m_oColSoundId);
			add(m_oColSourceIdHidden);
		}
		Gtk::TreeModelColumn<Glib::ustring> m_oColSourceId;
		Gtk::TreeModelColumn<int32_t> m_oColSoundId;
		Gtk::TreeModelColumn<int32_t> m_oColSourceIdHidden;
	};
	SourcesColumns m_oSourceColumns;
	Glib::RefPtr<Gtk::ListStore> m_refTreeModelSources;

	class DevicesColumns : public Gtk::TreeModel::ColumnRecord
	{
	public:
		DevicesColumns()
		{
			add(m_oColDeviceId);
			add(m_oColDeviceName);
		}
		Gtk::TreeModelColumn<int32_t> m_oColDeviceId;
		Gtk::TreeModelColumn<Glib::ustring> m_oColDeviceName;
	};
	DevicesColumns m_oDeviceColumns;
	Glib::RefPtr<Gtk::ListStore> m_refTreeModelDevices;

	static constexpr int32_t s_nInitialWindowSizeW = 800;
	static constexpr int32_t s_nInitialWindowSizeH = 500;
};

} // namespace wander

} // namespace example

#endif /* _EXAMPLE_WANDER_WINDOW_ */
