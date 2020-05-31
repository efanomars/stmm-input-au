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
 * File:   wanderwindow.cc
 */

#include "wanderwindow.h"

#include <stmm-input-au/sndfinishedevent.h>

#include <cassert>
#include <iostream>
#include <algorithm>
#include <vector>
#include <unordered_map>

namespace example
{

namespace wander
{


static constexpr const int32_t s_nTextBufferEventsMaxLines = 1000;

WanderWindow::WanderWindow(const shared_ptr<stmi::DeviceManager>& refDM, const Glib::ustring sTitle
							, int32_t nTotSources, std::vector<std::string>&& aSoundsPathFiles) noexcept
: m_refDM(refDM)
{
	m_oWanderData.m_aSoundPaths = std::move(aSoundsPathFiles);
	m_oWanderData.m_oAreaPos.m_fX = -10;
	m_oWanderData.m_oAreaPos.m_fY = -10;
	m_oWanderData.m_oAreaSize.m_fW = 20;
	m_oWanderData.m_oAreaSize.m_fH = 20;
	//
	set_title(sTitle);
	set_default_size(s_nInitialWindowSizeW, s_nInitialWindowSizeH);
	set_resizable(true);

	m_refAdjustmentAreaX = Gtk::Adjustment::create(m_oWanderData.m_oAreaPos.m_fX, -1000, 1000, 0.1, 10.0, 0);
	m_refAdjustmentAreaY = Gtk::Adjustment::create(m_oWanderData.m_oAreaPos.m_fY, -1000, 1000, 0.1, 10.0, 0);
	m_refAdjustmentAreaW = Gtk::Adjustment::create(m_oWanderData.m_oAreaSize.m_fW, 0.1, 1000, 0.1, 10.0, 0);
	m_refAdjustmentAreaH = Gtk::Adjustment::create(m_oWanderData.m_oAreaSize.m_fH, 0.1, 1000, 0.1, 10.0, 0);

	m_refAdjustmentListenerPosX = Gtk::Adjustment::create(0, -1000, 1000, 0.1, 10.0, 0);
	m_refAdjustmentListenerPosY = Gtk::Adjustment::create(0, -1000, 1000, 0.1, 10.0, 0);
	m_refAdjustmentListenerPosZ = Gtk::Adjustment::create(0, -1000, 1000, 0.1, 10.0, 0);

	//~ m_refAdjustmentListenerDirX = Gtk::Adjustment::create(0, -1000, 1000, 0.1, 10.0, 0);
	//~ m_refAdjustmentListenerDirY = Gtk::Adjustment::create(0, -1000, 1000, 0.1, 10.0, 0);
	//~ m_refAdjustmentListenerDirZ = Gtk::Adjustment::create(0, -1000, 1000, 0.1, 10.0, 0);

	m_refAdjustmentListenerVol = Gtk::Adjustment::create(1.0, 0.0, 1.0, 0.1, 0.1, 0);

	m_refAdjustmentSourcePosX = Gtk::Adjustment::create(0, -1000, 1000, 0.1, 10.0, 0);
	m_refAdjustmentSourcePosY = Gtk::Adjustment::create(0, -1000, 1000, 0.1, 10.0, 0);
	m_refAdjustmentSourcePosZ = Gtk::Adjustment::create(0, -1000, 1000, 0.1, 10.0, 0);

	m_refAdjustmentSourceVol = Gtk::Adjustment::create(1.0, 0.0, 1.0, 0.1, 0.1, 0);

	m_refAdjustmentFilename = Gtk::Adjustment::create(-1, -1, m_oWanderData.m_aSoundPaths.size() - 1, 1, 10, 0);

	m_refTreeModelDevices = Gtk::ListStore::create(m_oDeviceColumns);

	Glib::RefPtr<Gtk::TreeSelection> refTreeSelection;

	Pango::FontDescription oMonoFont("Mono");

	Gtk::Paned* m_p0HPanedMain = Gtk::manage(new Gtk::Paned(Gtk::ORIENTATION_HORIZONTAL));
	Gtk::Window::add(*m_p0HPanedMain);

	Gtk::Box* m_p0VBoxListener = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL));
	m_p0HPanedMain->pack1(*m_p0VBoxListener, false, false);

		Gtk::Box* m_p0VBoxArea = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL));
		m_p0VBoxListener->pack_start(*m_p0VBoxArea, false, false);
			Gtk::Label* m_p0LabelShow = Gtk::manage(new Gtk::Label("Shown area"));
			m_p0VBoxArea->pack_start(*m_p0LabelShow);
			Gtk::Box* m_p0HBoxAreaX = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL));
			m_p0VBoxArea->pack_start(*m_p0HBoxAreaX);
				Gtk::Label* m_p0LabelAreaX = Gtk::manage(new Gtk::Label("X:"));
				m_p0HBoxAreaX->pack_start(*m_p0LabelAreaX);
				m_p0SpinAreaX = Gtk::manage(new Gtk::SpinButton(m_refAdjustmentAreaX));
				m_p0HBoxAreaX->pack_start(*m_p0SpinAreaX);
					m_p0SpinAreaX->set_digits(1);
					m_p0SpinAreaX->signal_value_changed().connect(
									sigc::mem_fun(*this, &WanderWindow::onSpinAreaXChanged) );
			Gtk::Box* m_p0HBoxAreaY = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL));
			m_p0VBoxArea->pack_start(*m_p0HBoxAreaY);
				Gtk::Label* m_p0LabelAreaY = Gtk::manage(new Gtk::Label("Y:"));
				m_p0HBoxAreaY->pack_start(*m_p0LabelAreaY);
				m_p0SpinAreaY = Gtk::manage(new Gtk::SpinButton(m_refAdjustmentAreaY));
				m_p0HBoxAreaY->pack_start(*m_p0SpinAreaY);
					m_p0SpinAreaY->set_digits(1);
					m_p0SpinAreaY->signal_value_changed().connect(
									sigc::mem_fun(*this, &WanderWindow::onSpinAreaYChanged) );
			Gtk::Box* m_p0HBoxAreaW = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL));
			m_p0VBoxArea->pack_start(*m_p0HBoxAreaW);
				Gtk::Label* m_p0LabelAreaW = Gtk::manage(new Gtk::Label("W:"));
				m_p0HBoxAreaW->pack_start(*m_p0LabelAreaW);
				m_p0SpinAreaW = Gtk::manage(new Gtk::SpinButton(m_refAdjustmentAreaW));
				m_p0HBoxAreaW->pack_start(*m_p0SpinAreaW);
					m_p0SpinAreaW->set_digits(1);
					m_p0SpinAreaW->signal_value_changed().connect(
									sigc::mem_fun(*this, &WanderWindow::onSpinAreaWChanged) );
			Gtk::Box* m_p0HBoxAreaH = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL));
			m_p0VBoxArea->pack_start(*m_p0HBoxAreaH);
				Gtk::Label* m_p0LabelAreaH = Gtk::manage(new Gtk::Label("H:"));
				m_p0HBoxAreaH->pack_start(*m_p0LabelAreaH);
				m_p0SpinAreaH = Gtk::manage(new Gtk::SpinButton(m_refAdjustmentAreaH));
				m_p0HBoxAreaH->pack_start(*m_p0SpinAreaH);
					m_p0SpinAreaH->set_digits(1);
					m_p0SpinAreaH->signal_value_changed().connect(
									sigc::mem_fun(*this, &WanderWindow::onSpinAreaHChanged) );

		Gtk::Separator* m_p0HSeparatorArea = Gtk::manage(new Gtk::Separator(Gtk::ORIENTATION_HORIZONTAL));
		m_p0VBoxListener->pack_start(*m_p0HSeparatorArea, false, false);

		Gtk::Box* m_p0VBoxShowDevicesList = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL));
		m_p0VBoxListener->pack_start(*m_p0VBoxShowDevicesList, true, true);

			Gtk::Label* m_p0LabelShowDevicesList = Gtk::manage(new Gtk::Label("Show Device"));
			m_p0VBoxShowDevicesList->pack_start(*m_p0LabelShowDevicesList, false, false);

			Gtk::ScrolledWindow* m_p0ScrolledShowDevicesList = Gtk::manage(new Gtk::ScrolledWindow());
			m_p0VBoxShowDevicesList->pack_start(*m_p0ScrolledShowDevicesList, true, true, 3);
				m_p0ScrolledShowDevicesList->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_ALWAYS);
				m_p0ScrolledShowDevicesList->set_border_width(5);

				m_p0TreeViewShowDevicesList = Gtk::manage(new Gtk::TreeView(m_refTreeModelDevices));
				m_p0ScrolledShowDevicesList->add(*m_p0TreeViewShowDevicesList);
					m_p0TreeViewShowDevicesList->append_column("Id", m_oDeviceColumns.m_oColDeviceId);
					m_p0TreeViewShowDevicesList->append_column("Name", m_oDeviceColumns.m_oColDeviceName);
					//
					refTreeSelection = m_p0TreeViewShowDevicesList->get_selection();
					refTreeSelection->signal_changed().connect(
												sigc::mem_fun(*this, &WanderWindow::onShowDeviceSelectionChanged));

		Gtk::Separator* m_p0HSeparatorShowDevice = Gtk::manage(new Gtk::Separator(Gtk::ORIENTATION_HORIZONTAL));
		m_p0VBoxListener->pack_start(*m_p0HSeparatorShowDevice, false, false);

		Gtk::Box* m_p0VBoxListenerPos = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL));
		m_p0VBoxListener->pack_start(*m_p0VBoxListenerPos, false, false);
			Gtk::Label* m_p0LabelListenerPos = Gtk::manage(new Gtk::Label("Listener position"));
			m_p0VBoxListenerPos->pack_start(*m_p0LabelListenerPos);
			Gtk::Box* m_p0HBoxListenerPosX = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL));
			m_p0VBoxListenerPos->pack_start(*m_p0HBoxListenerPosX);
				Gtk::Label* m_p0LabelListenerPosX = Gtk::manage(new Gtk::Label("X:"));
				m_p0HBoxListenerPosX->pack_start(*m_p0LabelListenerPosX);
				m_p0SpinListenerPosX = Gtk::manage(new Gtk::SpinButton(m_refAdjustmentListenerPosX));
				m_p0HBoxListenerPosX->pack_start(*m_p0SpinListenerPosX);
					m_p0SpinListenerPosX->set_digits(1);
					m_p0SpinListenerPosX->signal_value_changed().connect(
									sigc::mem_fun(*this, &WanderWindow::onSpinListenerPosXChanged) );
			Gtk::Box* m_p0HBoxListenerPosY = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL));
			m_p0VBoxListenerPos->pack_start(*m_p0HBoxListenerPosY);
				Gtk::Label* m_p0LabelListenerPosY = Gtk::manage(new Gtk::Label("Y:"));
				m_p0HBoxListenerPosY->pack_start(*m_p0LabelListenerPosY);
				m_p0SpinListenerPosY = Gtk::manage(new Gtk::SpinButton(m_refAdjustmentListenerPosY));
				m_p0HBoxListenerPosY->pack_start(*m_p0SpinListenerPosY);
					m_p0SpinListenerPosY->set_digits(1);
					m_p0SpinListenerPosY->signal_value_changed().connect(
									sigc::mem_fun(*this, &WanderWindow::onSpinListenerPosYChanged) );
			Gtk::Box* m_p0HBoxListenerPosZ = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL));
			m_p0VBoxListenerPos->pack_start(*m_p0HBoxListenerPosZ);
				Gtk::Label* m_p0LabelListenerPosZ = Gtk::manage(new Gtk::Label("Z:"));
				m_p0HBoxListenerPosZ->pack_start(*m_p0LabelListenerPosZ);
				m_p0SpinListenerPosZ = Gtk::manage(new Gtk::SpinButton(m_refAdjustmentListenerPosZ));
				m_p0HBoxListenerPosZ->pack_start(*m_p0SpinListenerPosZ);
					m_p0SpinListenerPosZ->set_digits(1);
					m_p0SpinListenerPosZ->signal_value_changed().connect(
									sigc::mem_fun(*this, &WanderWindow::onSpinListenerPosZChanged) );

		//~ Gtk::Separator* m_p0HSeparatorAreaListenerDir = Gtk::manage(new Gtk::Separator(Gtk::ORIENTATION_HORIZONTAL));
		//~ m_p0VBoxListener->pack_start(*m_p0HSeparatorAreaListenerDir, false, false);

		//~ Gtk::Box* m_p0VBoxListenerDir = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL));
		//~ m_p0VBoxListener->pack_start(*m_p0VBoxListenerDir, false, false);
			//~ Gtk::Label* m_p0LabelListenerDir = Gtk::manage(new Gtk::Label("Listener direction"));
			//~ m_p0VBoxListenerDir->pack_start(*m_p0LabelListenerDir);
			//~ Gtk::Box* m_p0HBoxListenerDirX = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL));
			//~ m_p0VBoxListenerDir->pack_start(*m_p0HBoxListenerDirX);
				//~ Gtk::Label* m_p0LabelListenerDirX = Gtk::manage(new Gtk::Label("X:"));
				//~ m_p0HBoxListenerDirX->pack_start(*m_p0LabelListenerDirX);
				//~ m_p0SpinListenerDirX = Gtk::manage(new Gtk::SpinButton(m_refAdjustmentListenerDirX));
				//~ m_p0HBoxListenerDirX->pack_start(*m_p0SpinListenerDirX);
					//~ m_p0SpinListenerDirX->set_digits(1);
					//~ m_p0SpinListenerDirX->signal_value_changed().connect(
									//~ sigc::mem_fun(*this, &WanderWindow::onSpinListenerDirXChanged) );
			//~ Gtk::Box* m_p0HBoxListenerDirY = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL));
			//~ m_p0VBoxListenerDir->pack_start(*m_p0HBoxListenerDirY);
				//~ Gtk::Label* m_p0LabelListenerDirY = Gtk::manage(new Gtk::Label("Y:"));
				//~ m_p0HBoxListenerDirY->pack_start(*m_p0LabelListenerDirY);
				//~ m_p0SpinListenerDirY = Gtk::manage(new Gtk::SpinButton(m_refAdjustmentListenerDirY));
				//~ m_p0HBoxListenerDirY->pack_start(*m_p0SpinListenerDirY);
					//~ m_p0SpinListenerDirY->set_digits(1);
					//~ m_p0SpinListenerDirY->signal_value_changed().connect(
									//~ sigc::mem_fun(*this, &WanderWindow::onSpinListenerDirYChanged) );
			//~ Gtk::Box* m_p0HBoxListenerDirZ = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL));
			//~ m_p0VBoxListenerDir->pack_start(*m_p0HBoxListenerDirZ);
				//~ Gtk::Label* m_p0LabelListenerDirZ = Gtk::manage(new Gtk::Label("Z:"));
				//~ m_p0HBoxListenerDirZ->pack_start(*m_p0LabelListenerDirZ);
				//~ m_p0SpinListenerDirZ = Gtk::manage(new Gtk::SpinButton(m_refAdjustmentListenerDirZ));
				//~ m_p0HBoxListenerDirZ->pack_start(*m_p0SpinListenerDirZ);
					//~ m_p0SpinListenerDirZ->set_digits(1);
					//~ m_p0SpinListenerDirZ->signal_value_changed().connect(
									//~ sigc::mem_fun(*this, &WanderWindow::onSpinListenerDirZChanged) );

		Gtk::Box* m_p0HBoxListenerVol = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL));
		m_p0VBoxListener->pack_start(*m_p0HBoxListenerVol, false, false);
			Gtk::Label* m_p0LabelListenerVol = Gtk::manage(new Gtk::Label("Vol:"));
			m_p0HBoxListenerVol->pack_start(*m_p0LabelListenerVol);
			m_p0SpinListenerVol = Gtk::manage(new Gtk::SpinButton(m_refAdjustmentListenerVol));
			m_p0HBoxListenerVol->pack_start(*m_p0SpinListenerVol, false, false);
				m_p0SpinListenerVol->set_digits(2);
				m_p0SpinListenerVol->signal_value_changed().connect(
								sigc::mem_fun(*this, &WanderWindow::onSpinListenerVolChanged) );

		Gtk::Separator* m_p0HSeparatorListenerVolEvents = Gtk::manage(new Gtk::Separator(Gtk::ORIENTATION_HORIZONTAL));
		m_p0VBoxListener->pack_start(*m_p0HSeparatorListenerVolEvents, false, false);

		Gtk::Box* m_p0VBoxDeviceActions = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL));
		m_p0VBoxListener->pack_start(*m_p0VBoxDeviceActions, false, false);

			m_p0ButtonPauseDevice = Gtk::manage(new Gtk::Button("Pause device"));
			m_p0VBoxDeviceActions->pack_start(*m_p0ButtonPauseDevice, false, false);
				m_p0ButtonPauseDevice->signal_clicked().connect(
								sigc::mem_fun(*this, &WanderWindow::onButtonDevicePause) );

			m_p0ButtonResumeDevice = Gtk::manage(new Gtk::Button("Resume device"));
			m_p0VBoxDeviceActions->pack_start(*m_p0ButtonResumeDevice, false, false);
				m_p0ButtonResumeDevice->signal_clicked().connect(
								sigc::mem_fun(*this, &WanderWindow::onButtonDeviceResume) );

		Gtk::Separator* m_p0HSeparatorPauseDevice = Gtk::manage(new Gtk::Separator(Gtk::ORIENTATION_HORIZONTAL));
		m_p0VBoxListener->pack_start(*m_p0HSeparatorPauseDevice, false, false);

		Gtk::Box* m_p0VBoxEvents = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL));
		m_p0VBoxListener->pack_start(*m_p0VBoxEvents, true, true);

			Gtk::Label* m_p0LabelEvents = Gtk::manage(new Gtk::Label("Events"));
			m_p0VBoxEvents->pack_start(*m_p0LabelEvents, false, false);

			Gtk::ScrolledWindow* m_p0ScrolledEvents = Gtk::manage(new Gtk::ScrolledWindow());
			m_p0VBoxEvents->pack_start(*m_p0ScrolledEvents, true, true);
				m_p0ScrolledEvents->set_border_width(5);
				m_p0ScrolledEvents->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_ALWAYS);

				m_p0TextViewEvents = Gtk::manage(new Gtk::TextView());
				m_p0ScrolledEvents->add(*m_p0TextViewEvents);
					m_refTextBufferEvents = Gtk::TextBuffer::create();
					m_refTextBufferEventsBottomMark = Gtk::TextBuffer::Mark::create("Bottom");
					m_p0TextViewEvents->set_editable(false);
					m_p0TextViewEvents->set_buffer(m_refTextBufferEvents);
					m_p0TextViewEvents->override_font(oMonoFont);
					m_refTextBufferEvents->add_mark(m_refTextBufferEventsBottomMark, m_refTextBufferEvents->end());

	Gtk::Paned* m_p0HPanedShow = Gtk::manage(new Gtk::Paned(Gtk::ORIENTATION_HORIZONTAL));
	m_p0HPanedMain->pack2(*m_p0HPanedShow, true, true);

	Gtk::Box* m_p0VBoxShowArea = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL));
	m_p0HPanedShow->pack1(*m_p0VBoxShowArea, true, true);
		m_p0DrawingAreaShow = Gtk::manage(new WanderDrawingArea(m_oWanderData, this));
		m_p0VBoxShowArea->pack_start(*m_p0DrawingAreaShow, true, true);

	Gtk::Box* m_p0VBoxSources = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL));
	m_p0HPanedShow->pack2(*m_p0VBoxSources, false, false);
		Gtk::Box* m_p0VBoxSourcesList = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL));
		m_p0VBoxSources->pack_start(*m_p0VBoxSourcesList, true, true);

			Gtk::Label* m_p0LabelSourcesList = Gtk::manage(new Gtk::Label("Sources"));
			m_p0VBoxSourcesList->pack_start(*m_p0LabelSourcesList, false, false);

			Gtk::ScrolledWindow* m_p0ScrolledSourcesList = Gtk::manage(new Gtk::ScrolledWindow());
			m_p0VBoxSourcesList->pack_start(*m_p0ScrolledSourcesList, true, true, 3);
				m_p0ScrolledSourcesList->set_policy(Gtk::POLICY_ALWAYS, Gtk::POLICY_ALWAYS);
				m_p0ScrolledSourcesList->set_border_width(5);

				m_refTreeModelSources = Gtk::ListStore::create(m_oSourceColumns);
				m_p0TreeViewSourcesList = Gtk::manage(new Gtk::TreeView(m_refTreeModelSources));
				m_p0ScrolledSourcesList->add(*m_p0TreeViewSourcesList);
					m_p0TreeViewSourcesList->append_column("Nr", m_oSourceColumns.m_oColSourceId);
					m_p0TreeViewSourcesList->append_column("SndId", m_oSourceColumns.m_oColSoundId);
					//
					refTreeSelection = m_p0TreeViewSourcesList->get_selection();
					refTreeSelection->signal_changed().connect(
												sigc::mem_fun(*this, &WanderWindow::onSourceSelectionChanged));

		Gtk::Separator* m_p0HSeparatorSourcesList = Gtk::manage(new Gtk::Separator(Gtk::ORIENTATION_HORIZONTAL));
		m_p0VBoxSources->pack_start(*m_p0HSeparatorSourcesList, false, false);

		Gtk::Box* m_p0VBoxSelSource = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL));
		m_p0VBoxSources->pack_start(*m_p0VBoxSelSource, false, false);

			m_p0LabelSourceName = Gtk::manage(new Gtk::Label("Source:"));
			m_p0VBoxSelSource->pack_start(*m_p0LabelSourceName, false, false);

			m_p0LabelDeviceId = Gtk::manage(new Gtk::Label("Device:"));
			m_p0VBoxSelSource->pack_start(*m_p0LabelDeviceId, false, false);

			Gtk::Box* m_p0VBoxFilename = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL));
			m_p0VBoxSelSource->pack_start(*m_p0VBoxFilename, false, false);
				Gtk::Box* m_p0HBoxFilenameIdx = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL));
				m_p0VBoxFilename->pack_start(*m_p0HBoxFilenameIdx, false, false);
					Gtk::Label* m_p0LabelFilenameIdx = Gtk::manage(new Gtk::Label("Filename: "));
					m_p0HBoxFilenameIdx->pack_start(*m_p0LabelFilenameIdx, false, false);
					m_p0SpinFilenameIdx = Gtk::manage(new Gtk::SpinButton(m_refAdjustmentFilename));
					m_p0HBoxFilenameIdx->pack_start(*m_p0SpinFilenameIdx);
						m_p0SpinFilenameIdx->signal_value_changed().connect(
										sigc::mem_fun(*this, &WanderWindow::onSpinFilenameChanged) );
				m_p0LabelFilePath = Gtk::manage(new Gtk::Label("-"));
				m_p0VBoxFilename->pack_start(*m_p0LabelFilePath, false, false);
					m_p0LabelFilePath->set_ellipsize(Pango::ELLIPSIZE_START);

			m_p0CheckButtonLooping = Gtk::manage(new Gtk::CheckButton("Looping"));
			m_p0VBoxSelSource->pack_start(*m_p0CheckButtonLooping, false, false);
				m_p0CheckButtonLooping->signal_toggled().connect(
								sigc::mem_fun(*this, &WanderWindow::onSoundLoopingToggled) );

			Gtk::Box* m_p0HBoxSoundId = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL));
			m_p0VBoxSelSource->pack_start(*m_p0HBoxSoundId, false, false);
				m_p0HBoxSoundId->set_border_width(5);

				m_p0LabelSoundIdLabel = Gtk::manage(new Gtk::Label("Sound id: "));
				m_p0HBoxSoundId->pack_start(*m_p0LabelSoundIdLabel, false, false);

				m_p0LabelSoundId = Gtk::manage(new Gtk::Label("-"));
				m_p0HBoxSoundId->pack_start(*m_p0LabelSoundId, true, true);
					m_p0LabelSoundId->set_halign(Gtk::ALIGN_END);

			Gtk::Box* m_p0VBoxSelSourcePos = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL));
			m_p0VBoxSelSource->pack_start(*m_p0VBoxSelSourcePos, false, false);

				Gtk::Label* m_p0LabelSourcePos = Gtk::manage(new Gtk::Label("Source position:"));
				m_p0VBoxSelSource->pack_start(*m_p0LabelSourcePos, false, false);

				Gtk::Box* m_p0HBoxSourcePosX = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL));
				m_p0VBoxSelSource->pack_start(*m_p0HBoxSourcePosX);
					Gtk::Label* m_p0LabelSourcePosX = Gtk::manage(new Gtk::Label("X:"));
					m_p0HBoxSourcePosX->pack_start(*m_p0LabelSourcePosX);
					m_p0SpinSourcePosX = Gtk::manage(new Gtk::SpinButton(m_refAdjustmentSourcePosX));
					m_p0HBoxSourcePosX->pack_start(*m_p0SpinSourcePosX);
						m_p0SpinSourcePosX->set_digits(1);
						m_p0SpinSourcePosX->signal_value_changed().connect(
										sigc::mem_fun(*this, &WanderWindow::onSpinSourcePosXChanged) );

				Gtk::Box* m_p0HBoxSourcePosY = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL));
				m_p0VBoxSelSource->pack_start(*m_p0HBoxSourcePosY);
					Gtk::Label* m_p0LabelSourcePosY = Gtk::manage(new Gtk::Label("Y:"));
					m_p0HBoxSourcePosY->pack_start(*m_p0LabelSourcePosY);
					m_p0SpinSourcePosY = Gtk::manage(new Gtk::SpinButton(m_refAdjustmentSourcePosY));
					m_p0HBoxSourcePosY->pack_start(*m_p0SpinSourcePosY);
						m_p0SpinSourcePosY->set_digits(1);
						m_p0SpinSourcePosY->signal_value_changed().connect(
										sigc::mem_fun(*this, &WanderWindow::onSpinSourcePosYChanged) );

				Gtk::Box* m_p0HBoxSourcePosZ = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL));
				m_p0VBoxSelSource->pack_start(*m_p0HBoxSourcePosZ);
					Gtk::Label* m_p0LabelSourcePosZ = Gtk::manage(new Gtk::Label("Z:"));
					m_p0HBoxSourcePosZ->pack_start(*m_p0LabelSourcePosZ);
					m_p0SpinSourcePosZ = Gtk::manage(new Gtk::SpinButton(m_refAdjustmentSourcePosZ));
					m_p0HBoxSourcePosZ->pack_start(*m_p0SpinSourcePosZ);
						m_p0SpinSourcePosZ->set_digits(1);
						m_p0SpinSourcePosZ->signal_value_changed().connect(
										sigc::mem_fun(*this, &WanderWindow::onSpinSourcePosZChanged) );

			m_p0CheckButtonListenerRelative = Gtk::manage(new Gtk::CheckButton("Listener relative"));
			m_p0VBoxSelSource->pack_start(*m_p0CheckButtonListenerRelative, false, false);
				m_p0CheckButtonListenerRelative->signal_toggled().connect(
								sigc::mem_fun(*this, &WanderWindow::onSoundListenerRelativeToggled) );

			Gtk::Box* m_p0HBoxSourceVol = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL));
			m_p0VBoxSelSource->pack_start(*m_p0HBoxSourceVol);
				Gtk::Label* m_p0LabelSourceVol = Gtk::manage(new Gtk::Label("Vol:"));
				m_p0HBoxSourceVol->pack_start(*m_p0LabelSourceVol);
				m_p0SpinSourceVol = Gtk::manage(new Gtk::SpinButton(m_refAdjustmentSourceVol));
				m_p0HBoxSourceVol->pack_start(*m_p0SpinSourceVol);
					m_p0SpinSourceVol->set_digits(2);
					m_p0SpinSourceVol->signal_value_changed().connect(
									sigc::mem_fun(*this, &WanderWindow::onSpinSourceVolChanged) );

			Gtk::Box* m_p0VBoxSoundActions = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL));
			m_p0VBoxSelSource->pack_start(*m_p0VBoxSoundActions, false, false);
				m_p0ButtonPlaySound = Gtk::manage(new Gtk::Button(Gtk::Stock::MEDIA_PLAY));
				m_p0VBoxSoundActions->pack_start(*m_p0ButtonPlaySound, false, false);
					m_p0ButtonPlaySound->signal_clicked().connect(
									sigc::mem_fun(*this, &WanderWindow::onButtonSoundPlay) );
				m_p0ButtonPauseSound = Gtk::manage(new Gtk::Button(Gtk::Stock::MEDIA_PAUSE));
				m_p0VBoxSoundActions->pack_start(*m_p0ButtonPauseSound, false, false);
					m_p0ButtonPauseSound->signal_clicked().connect(
									sigc::mem_fun(*this, &WanderWindow::onButtonSoundPause) );
				m_p0ButtonStopSound = Gtk::manage(new Gtk::Button(Gtk::Stock::MEDIA_STOP));
				m_p0VBoxSoundActions->pack_start(*m_p0ButtonStopSound, false, false);
					m_p0ButtonStopSound->signal_clicked().connect(
									sigc::mem_fun(*this, &WanderWindow::onButtonSoundStop) );

		Gtk::Separator* m_p0HSeparatorStopAll = Gtk::manage(new Gtk::Separator(Gtk::ORIENTATION_HORIZONTAL));
		m_p0VBoxSources->pack_start(*m_p0HSeparatorStopAll, false, false);

		Gtk::Box* m_p0VBoxDevicesList = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL));
		m_p0VBoxSources->pack_start(*m_p0VBoxDevicesList, true, true);

			Gtk::Label* m_p0LabelDevicesList = Gtk::manage(new Gtk::Label("Devices"));
			m_p0VBoxDevicesList->pack_start(*m_p0LabelDevicesList, false, false);

			Gtk::ScrolledWindow* m_p0ScrolledDevicesList = Gtk::manage(new Gtk::ScrolledWindow());
			m_p0VBoxDevicesList->pack_start(*m_p0ScrolledDevicesList, true, true, 3);
				m_p0ScrolledDevicesList->set_policy(Gtk::POLICY_ALWAYS, Gtk::POLICY_ALWAYS);
				m_p0ScrolledDevicesList->set_border_width(5);

				m_p0TreeViewDevicesList = Gtk::manage(new Gtk::TreeView(m_refTreeModelDevices));
				m_p0ScrolledDevicesList->add(*m_p0TreeViewDevicesList);
					m_p0TreeViewDevicesList->append_column("Id", m_oDeviceColumns.m_oColDeviceId);
					m_p0TreeViewDevicesList->append_column("Name", m_oDeviceColumns.m_oColDeviceName);
					//
					refTreeSelection = m_p0TreeViewDevicesList->get_selection();
					refTreeSelection->signal_changed().connect(
												sigc::mem_fun(*this, &WanderWindow::onDeviceSelectionChanged));

		Gtk::Separator* m_p0HSeparatorSource = Gtk::manage(new Gtk::Separator(Gtk::ORIENTATION_HORIZONTAL));
		m_p0VBoxSources->pack_start(*m_p0HSeparatorSource, false, false);
			m_p0HSeparatorSource->set_size_request(-1, 5);

		Gtk::Button* m_p0ButtonStopAllSounds = Gtk::manage(new Gtk::Button("Stop all sounds"));
		m_p0VBoxSources->pack_start(*m_p0ButtonStopAllSounds, false, false);
			m_p0ButtonStopAllSounds->signal_clicked().connect(
							sigc::mem_fun(*this, &WanderWindow::onButtonSoundsStopAll) );



	m_refCallIfSndFinished = std::make_shared<stmi::CallIfEventClass>(stmi::SndFinishedEvent::getClass());
	m_refListenerSndFinishedEvents = std::make_shared<stmi::EventListener>(
						std::bind( &WanderWindow::handleSndFinishedEvents, this, std::placeholders::_1 ) );
	m_refDM->addEventListener(m_refListenerSndFinishedEvents, m_refCallIfSndFinished);

	m_refCallIfDeviceMgmt = std::make_shared<stmi::CallIfEventClass>(stmi::DeviceMgmtEvent::getClass());
	m_refListenerDeviceMgmtEvents = std::make_shared<stmi::EventListener>(
						std::bind( &WanderWindow::handleDeviceMgmtEvents, this, std::placeholders::_1 ) );
	m_refDM->addEventListener(m_refListenerDeviceMgmtEvents, m_refCallIfDeviceMgmt);

	show_all_children();

	initDevices();
	initSources(nTotSources);

	recreateSourcesList();
	recreateDevicesLists();

	refreshCurrentShow();
	refreshCurrentSource();
}
void WanderWindow::initDevices() noexcept
{
	const auto aDeviceIds = m_refDM->getDevicesWithCapabilityClass(stmi::PlaybackCapability::getClass());
	for (int32_t nDeviceId : aDeviceIds) {
		auto refDevice = m_refDM->getDevice(nDeviceId);
		assert(refDevice);
		//const std::string& sDeviceName = refDevice->getName();
		WanderData::CapabilityData oCapaData;
		#ifndef NDEBUG
		const bool bHasIt =
		#endif //NDEBUG
		refDevice->getCapability(oCapaData.m_refCapability);
		assert(bHasIt);
		assert(oCapaData.m_refCapability);
		oCapaData.m_nDeviceId = refDevice->getId();
		m_oWanderData.m_aCapabilityDatas.push_back(std::move(oCapaData));
	}
}
void WanderWindow::initSources(int32_t nTotSources) noexcept
{
	assert(nTotSources > 0);
	m_oWanderData.m_aSourceDatas.resize(nTotSources);

	setCapabilityWherePossible();
}
void WanderWindow::setCapabilityWherePossible() noexcept
{
	if (m_oWanderData.m_aCapabilityDatas.empty()) {
		m_oWanderData.m_nCurrentShowDeviceIdx = -1;
		return; //------------------------------------------------------
	}
	// find default device
	const int32_t nCapaIdx = [&]()
	{
		int32_t nIdx = 0;
		for (auto& oCD : m_oWanderData.m_aCapabilityDatas) {
			auto& refCurPlayback = oCD.m_refCapability;
			if (refCurPlayback->isDefaultDevice()) {
				return nIdx;
			}
			++nIdx;
		}
		return 0;
	}();
	auto& refPlayback = m_oWanderData.m_aCapabilityDatas[nCapaIdx].m_refCapability;
	auto& nDeviceId = m_oWanderData.m_aCapabilityDatas[nCapaIdx].m_nDeviceId;
//std::cout << "WanderWindow::initSources " << (refPlayback ? "has playback" : "NO PLAYBACK") << '\n';
	for (WanderData::SourceData& oSourceData : m_oWanderData.m_aSourceDatas) {
		if (! oSourceData.m_refPlayback) {
			assert(oSourceData.m_nPlaybackDeviceId < 0);
			oSourceData.m_refPlayback = refPlayback;
			oSourceData.m_nPlaybackDeviceId = nDeviceId;
		}
	}
	if (m_oWanderData.m_nCurrentShowDeviceIdx < 0){
		m_oWanderData.m_nCurrentShowDeviceIdx = nCapaIdx;
	}
}
int32_t WanderWindow::getDeviceIdFromCapability(const stmi::PlaybackCapability* p0Playback) const noexcept
{
	if (p0Playback == nullptr) {
		return -1;
	}
	auto refDevice = p0Playback->getDevice();
	if (! refDevice) {
		return -1;
	}
	return refDevice->getId();
}
int32_t WanderWindow::getCapabilityDataIndexFromDeviceId(int32_t nDeviceId) const noexcept
{
	auto& aCapabilityDatas = m_oWanderData.m_aCapabilityDatas;
	auto itFind = std::find_if(aCapabilityDatas.begin(), aCapabilityDatas.end(), [&](const WanderData::CapabilityData& oCD)
	{
		return (oCD.m_nDeviceId == nDeviceId);
	});
	if (itFind == aCapabilityDatas.end()) {
		return -1; //---------------------------------------------------
	}
	return std::distance(aCapabilityDatas.begin(), itFind);
}
void WanderWindow::handleDeviceMgmtEvents(const shared_ptr<stmi::Event>& refEvent) noexcept
{
//std::cout << "WanderWindow::handleDeviceMgmtEvents " << '\n';
	assert(std::dynamic_pointer_cast<stmi::DeviceMgmtEvent>(refEvent));
	auto refDeviceMgmtEvent = std::static_pointer_cast<stmi::DeviceMgmtEvent>(refEvent);
	printToEvents(*refDeviceMgmtEvent);
	const auto eType = refDeviceMgmtEvent->getDeviceMgmtType();
	if (eType == stmi::DeviceMgmtEvent::DEVICE_MGMT_CHANGED) {
		return; //------------------------------------------------------
	}
	auto refDevice = refDeviceMgmtEvent->getDevice();
	const int32_t nDeviceId = refDevice->getId();
	if (eType == stmi::DeviceMgmtEvent::DEVICE_MGMT_ADDED) {
		#ifndef NDEBUG
		const int32_t nIdx = getCapabilityDataIndexFromDeviceId(nDeviceId);
		assert(nIdx < 0);
		#endif //NDEBUG
		WanderData::CapabilityData oCapaData;
		#ifndef NDEBUG
		const bool bHasIt =
		#endif //NDEBUG
		refDevice->getCapability(oCapaData.m_refCapability);
		assert(bHasIt);
		//oCapaData.m_refCapability = refDevice->getCapability(stmi::PlaybackCapability::getClass());
		assert(oCapaData.m_refCapability);
		oCapaData.m_nDeviceId = nDeviceId;
		m_oWanderData.m_aCapabilityDatas.push_back(std::move(oCapaData));
	} else {
		assert(eType == stmi::DeviceMgmtEvent::DEVICE_MGMT_REMOVED);
		const int32_t nRemoveIdx = getCapabilityDataIndexFromDeviceId(nDeviceId);
		assert(nRemoveIdx >= 0);
		auto& aCapabilityDatas = m_oWanderData.m_aCapabilityDatas;
		auto& refCapability = aCapabilityDatas[nRemoveIdx].m_refCapability;
		// remove from sources
		for (WanderData::SourceData& oSD : m_oWanderData.m_aSourceDatas) {
			if (oSD.m_refPlayback == refCapability) {
				oSD.m_refPlayback.reset();
				oSD.m_nPlaybackDeviceId = -1;
				oSD.m_nSoundId = -1;
				oSD.m_bPaused = false;
			}
		}
		// remove from capabilities
		const int32_t nLastIdx = static_cast<int32_t>(aCapabilityDatas.size()) - 1;
		if (nRemoveIdx < nLastIdx) {
			aCapabilityDatas[nRemoveIdx] = std::move(aCapabilityDatas[nLastIdx]);
		} else if (nLastIdx < 0) {
			m_oWanderData.m_nCurrentShowDeviceIdx = -1;
		} else {
			m_oWanderData.m_nCurrentShowDeviceIdx = nLastIdx;
		}
		aCapabilityDatas.pop_back();
	}

	setCapabilityWherePossible();

	recreateDevicesLists();
	refreshCurrentShow();
	refreshCurrentSource();
	m_p0DrawingAreaShow->resetButtons();
}
void WanderWindow::handleSndFinishedEvents(const shared_ptr<stmi::Event>& refEvent) noexcept
{
//std::cout << "WanderWindow::handleSndFinishedEvents " << '\n';
	assert(std::dynamic_pointer_cast<stmi::SndFinishedEvent>(refEvent));
	auto refSndFinishedEvent = std::static_pointer_cast<stmi::SndFinishedEvent>(refEvent);

	printToEvents(*refSndFinishedEvent);
	const int32_t nSoundId = refSndFinishedEvent->getSoundId();

	// remove sound from source playing it
	int32_t nSourceId = 0;
	for (WanderData::SourceData& oSD : m_oWanderData.m_aSourceDatas) {
		if (oSD.m_nSoundId == nSoundId) {
			oSD.m_nSoundId = -1;
			oSD.m_bPaused = false;
			if (nSourceId == m_oWanderData.m_nCurrentSource) {
				refreshCurrentSource();
			}
		}
		++nSourceId;
	}
}

void WanderWindow::recreateDevicesLists() noexcept
{
//std::cout << "WanderWindow::recreateDevicesLists " << '\n';
	assert(!m_bRecreateDevicesListInProgress);
	m_bRecreateDevicesListInProgress = true;

	m_refTreeModelDevices->clear();
	int32_t nRowIdx = 0;
	for (WanderData::CapabilityData& oCD : m_oWanderData.m_aCapabilityDatas) {
		const int32_t nDeviceId = oCD.m_nDeviceId;
		auto refDevice = m_refDM->getDevice(nDeviceId);
		assert(refDevice);
		const std::string& sDeviceName = refDevice->getName();

		//
		Gtk::TreeModel::Row oRow;
		oRow = *(m_refTreeModelDevices->append());
		oRow[m_oDeviceColumns.m_oColDeviceId] = nDeviceId;
		oRow[m_oDeviceColumns.m_oColDeviceName] = sDeviceName;
		++nRowIdx;
	}
	if (nRowIdx == 0) {
		assert(m_oWanderData.m_nCurrentShowDeviceIdx < 0);
	}

	m_bRecreateDevicesListInProgress = false;

	selectDeviceInDeviceLists();
}
void WanderWindow::selectDeviceInDeviceLists() noexcept
{
	assert(!m_bRecreateDevicesListInProgress);
	m_bRecreateDevicesListInProgress = true;

	WanderData::CapabilityData& oShowCD = ((m_oWanderData.m_nCurrentShowDeviceIdx >= 0) ? m_oWanderData.m_aCapabilityDatas[m_oWanderData.m_nCurrentShowDeviceIdx] : m_oDummyCapabilityData);
	WanderData::SourceData& oSD = m_oWanderData.m_aSourceDatas[m_oWanderData.m_nCurrentSource];

	int32_t nSelectedShowDevicesRow = -1;
	int32_t nSelectedDevicesRow = -1;
	int32_t nRowIdx = 0;
	auto aRows = m_refTreeModelDevices->children();
	for (const Gtk::TreeModel::Row& oRow : aRows) {
		const int32_t nDeviceId = oRow[m_oDeviceColumns.m_oColDeviceId];
		if ((nDeviceId == oShowCD.m_nDeviceId) && (nSelectedShowDevicesRow < 0)) {
			nSelectedShowDevicesRow = nRowIdx;
		}
		if ((nDeviceId == oSD.m_nPlaybackDeviceId) && (nSelectedDevicesRow < 0)) {
			nSelectedDevicesRow = nRowIdx;
		}
		++nRowIdx;
	}

	m_p0TreeViewShowDevicesList->expand_all();
	m_p0TreeViewDevicesList->expand_all();

	Glib::RefPtr<Gtk::TreeSelection> refTreeSelection = m_p0TreeViewShowDevicesList->get_selection();
	if (nSelectedShowDevicesRow >= 0) {
		Gtk::TreeModel::Path oPath;
		oPath.push_back(nSelectedShowDevicesRow);
		refTreeSelection->select(oPath);
	} else {
		refTreeSelection->unselect_all();
	}

	refTreeSelection = m_p0TreeViewDevicesList->get_selection();
	if (nSelectedDevicesRow >= 0) {
		Gtk::TreeModel::Path oPath;
		oPath.push_back(nSelectedDevicesRow);
		refTreeSelection->select(oPath);
	} else {
		refTreeSelection->unselect_all();
	}

	m_bRecreateDevicesListInProgress = false;
}
void WanderWindow::recreateSourcesList() noexcept
{
	assert(!m_bRecreateSourcesListInProgress);
	m_bRecreateSourcesListInProgress = true;
	m_refTreeModelSources->clear();

	int32_t nRowIdx = 0;
	for (WanderData::SourceData& oSD : m_oWanderData.m_aSourceDatas) {
		Gtk::TreeModel::Row oRow;
		oRow = *(m_refTreeModelSources->append());
		oRow[m_oSourceColumns.m_oColSourceId] = "Source " + std::to_string(nRowIdx);
		oRow[m_oSourceColumns.m_oColSoundId] = oSD.m_nSoundId;
		oRow[m_oSourceColumns.m_oColSourceIdHidden] = nRowIdx;
		++nRowIdx;
	}
	m_p0TreeViewSourcesList->expand_all();
	Glib::RefPtr<Gtk::TreeSelection> refTreeSelection = m_p0TreeViewSourcesList->get_selection();
	Gtk::TreeModel::Path oPath;
	oPath.push_back(m_oWanderData.m_nCurrentSource);
	refTreeSelection->select(oPath);

	m_bRecreateSourcesListInProgress = false;
}
void WanderWindow::refreshCurrentShow() noexcept
{
	selectDeviceInDeviceLists();

	WanderData::CapabilityData& oShowCD = ((m_oWanderData.m_nCurrentShowDeviceIdx >= 0) ? m_oWanderData.m_aCapabilityDatas[m_oWanderData.m_nCurrentShowDeviceIdx] : m_oDummyCapabilityData);

	m_p0SpinListenerPosX->set_value(oShowCD.m_oListenerPos.m_fX);
	m_p0SpinListenerPosY->set_value(oShowCD.m_oListenerPos.m_fY);
	m_p0SpinListenerPosZ->set_value(oShowCD.m_oListenerPos.m_fZ);

	//~ m_p0SpinListenerDirX->set_value(oShowCD.m_oListenerDir.m_fX);
	//~ m_p0SpinListenerDirY->set_value(oShowCD.m_oListenerDir.m_fY);
	//~ m_p0SpinListenerDirZ->set_value(oShowCD.m_oListenerDir.m_fZ);

	m_p0SpinListenerVol->set_value(oShowCD.m_fListenerVol);

	const bool bDevicePresent = (m_oWanderData.m_nCurrentShowDeviceIdx >= 0);

	m_p0SpinListenerPosX->set_sensitive(bDevicePresent);
	m_p0SpinListenerPosY->set_sensitive(bDevicePresent);
	m_p0SpinListenerPosZ->set_sensitive(bDevicePresent);

	//~ m_p0SpinListenerDirX->set_sensitive(bDevicePresent);
	//~ m_p0SpinListenerDirY->set_sensitive(bDevicePresent);
	//~ m_p0SpinListenerDirZ->set_sensitive(bDevicePresent);

	m_p0ButtonPauseDevice->set_sensitive(bDevicePresent && ! oShowCD.m_bDevicePaused);
	m_p0ButtonResumeDevice->set_sensitive(bDevicePresent && oShowCD.m_bDevicePaused);

	const bool bIsActive = get_realized() && get_visible() && is_active();
	if (bIsActive) {
		redrawContents();
	}
}
void WanderWindow::refreshCurrentSource() noexcept
{
	WanderData::SourceData& oSD = m_oWanderData.m_aSourceDatas[m_oWanderData.m_nCurrentSource];

	m_p0LabelSourceName->set_text("Source: " + std::to_string(m_oWanderData.m_nCurrentSource));
	const int32_t nDeviceId = oSD.m_nPlaybackDeviceId;
	m_p0LabelDeviceId->set_text("Device: " + ((nDeviceId >= 0) ? std::to_string(nDeviceId) : "-"));
	m_p0SpinFilenameIdx->set_value(oSD.m_nSoundPathIdx);
	m_p0LabelFilePath->set_text((oSD.m_nSoundPathIdx >= 0) ? m_oWanderData.m_aSoundPaths[oSD.m_nSoundPathIdx] : "-");

	const bool bIsPlaying = oSD.isPlaying();

	m_p0CheckButtonLooping->set_active(oSD.m_bLooping);
	m_p0LabelSoundId->set_text(bIsPlaying ? std::to_string(oSD.m_nSoundId) : "-");
	m_p0SpinSourcePosX->set_value(oSD.m_oPos.m_fX);
	m_p0SpinSourcePosY->set_value(oSD.m_oPos.m_fY);
	m_p0SpinSourcePosZ->set_value(oSD.m_oPos.m_fZ);
	m_p0CheckButtonListenerRelative->set_active(oSD.m_bListenerRelative);

	m_p0SpinSourceVol->set_value(oSD.m_fVol);
	//int32_t m_nFileId = -1; // The file id of the file path for the capability or -1 if not playing

	m_p0CheckButtonLooping->set_sensitive(! bIsPlaying);
	m_p0ButtonPlaySound->set_sensitive((! bIsPlaying) || oSD.m_bPaused);
	m_p0ButtonPauseSound->set_sensitive(bIsPlaying && ! oSD.m_bPaused);
	m_p0ButtonStopSound->set_sensitive(bIsPlaying);

	m_p0TreeViewDevicesList->set_sensitive(! bIsPlaying);

	selectDeviceInDeviceLists();
}
void WanderWindow::redrawContents() noexcept
{
	//m_p0DrawingAreaShow->resetButtons();
	m_p0DrawingAreaShow->redrawContents();
}

void WanderWindow::onSpinAreaXChanged() noexcept
{
	m_oWanderData.m_oAreaPos.m_fX = m_p0SpinAreaX->get_value();

	redrawContents();
}
void WanderWindow::onSpinAreaYChanged() noexcept
{
	m_oWanderData.m_oAreaPos.m_fY = m_p0SpinAreaY->get_value();

	redrawContents();
}
void WanderWindow::onSpinAreaWChanged() noexcept
{
	m_oWanderData.m_oAreaSize.m_fW = m_p0SpinAreaW->get_value();

	redrawContents();
}
void WanderWindow::onSpinAreaHChanged() noexcept
{
	m_oWanderData.m_oAreaSize.m_fH = m_p0SpinAreaH->get_value();

	redrawContents();
}

void WanderWindow::onSpinListenerPosXChanged() noexcept
{
	WanderData::CapabilityData& oShowCD = ((m_oWanderData.m_nCurrentShowDeviceIdx >= 0) ? m_oWanderData.m_aCapabilityDatas[m_oWanderData.m_nCurrentShowDeviceIdx] : m_oDummyCapabilityData);
	oShowCD.m_oListenerPos.m_fX = m_p0SpinListenerPosX->get_value();
	if (oShowCD.m_refCapability) {
		oShowCD.m_refCapability->setListenerPos(oShowCD.m_oListenerPos.m_fX, oShowCD.m_oListenerPos.m_fY, oShowCD.m_oListenerPos.m_fZ);
		redrawContents();
	}
}
void WanderWindow::onSpinListenerPosYChanged() noexcept
{
	WanderData::CapabilityData& oShowCD = ((m_oWanderData.m_nCurrentShowDeviceIdx >= 0) ? m_oWanderData.m_aCapabilityDatas[m_oWanderData.m_nCurrentShowDeviceIdx] : m_oDummyCapabilityData);
	oShowCD.m_oListenerPos.m_fY = m_p0SpinListenerPosY->get_value();
	if (oShowCD.m_refCapability) {
		oShowCD.m_refCapability->setListenerPos(oShowCD.m_oListenerPos.m_fX, oShowCD.m_oListenerPos.m_fY, oShowCD.m_oListenerPos.m_fZ);
		redrawContents();
	}
}
void WanderWindow::onSpinListenerPosZChanged() noexcept
{
	WanderData::CapabilityData& oShowCD = ((m_oWanderData.m_nCurrentShowDeviceIdx >= 0) ? m_oWanderData.m_aCapabilityDatas[m_oWanderData.m_nCurrentShowDeviceIdx] : m_oDummyCapabilityData);
	oShowCD.m_oListenerPos.m_fZ = m_p0SpinListenerPosZ->get_value();
	if (oShowCD.m_refCapability) {
		oShowCD.m_refCapability->setListenerPos(oShowCD.m_oListenerPos.m_fX, oShowCD.m_oListenerPos.m_fY, oShowCD.m_oListenerPos.m_fZ);
		redrawContents();
	}
}

//~ void WanderWindow::onSpinListenerDirXChanged() noexcept
//~ {
	//~ WanderData::CapabilityData& oShowCD = ((m_oWanderData.m_nCurrentShowDeviceIdx >= 0) ? m_oWanderData.m_aCapabilityDatas[m_oWanderData.m_nCurrentShowDeviceIdx] : m_oDummyCapabilityData);
	//~ oShowCD.m_oListenerDir.m_fX = m_p0SpinListenerDirX->get_value();
	//~ if (oShowCD.m_refCapability) {
		//~ oShowCD.m_refCapability->setListenerDir(oShowCD.m_oListenerDir.m_fX, oShowCD.m_oListenerDir.m_fY, oShowCD.m_oListenerDir.m_fZ);
		//~ redrawContents();
	//~ }
//~ }
//~ void WanderWindow::onSpinListenerDirYChanged() noexcept
//~ {
	//~ WanderData::CapabilityData& oShowCD = ((m_oWanderData.m_nCurrentShowDeviceIdx >= 0) ? m_oWanderData.m_aCapabilityDatas[m_oWanderData.m_nCurrentShowDeviceIdx] : m_oDummyCapabilityData);
	//~ oShowCD.m_oListenerDir.m_fY = m_p0SpinListenerDirY->get_value();
	//~ if (oShowCD.m_refCapability) {
		//~ oShowCD.m_refCapability->setListenerDir(oShowCD.m_oListenerDir.m_fX, oShowCD.m_oListenerDir.m_fY, oShowCD.m_oListenerDir.m_fZ);
		//~ redrawContents();
	//~ }
//~ }
//~ void WanderWindow::onSpinListenerDirZChanged() noexcept
//~ {
	//~ WanderData::CapabilityData& oShowCD = ((m_oWanderData.m_nCurrentShowDeviceIdx >= 0) ? m_oWanderData.m_aCapabilityDatas[m_oWanderData.m_nCurrentShowDeviceIdx] : m_oDummyCapabilityData);
	//~ oShowCD.m_oListenerDir.m_fZ = m_p0SpinListenerDirZ->get_value();
	//~ if (oShowCD.m_refCapability) {
		//~ oShowCD.m_refCapability->setListenerDir(oShowCD.m_oListenerDir.m_fX, oShowCD.m_oListenerDir.m_fY, oShowCD.m_oListenerDir.m_fZ);
		//~ redrawContents();
	//~ }
//~ }
void WanderWindow::onSpinListenerVolChanged() noexcept
{
	WanderData::CapabilityData& oShowCD = ((m_oWanderData.m_nCurrentShowDeviceIdx >= 0) ? m_oWanderData.m_aCapabilityDatas[m_oWanderData.m_nCurrentShowDeviceIdx] : m_oDummyCapabilityData);
	oShowCD.m_fListenerVol = m_p0SpinListenerVol->get_value();
	if (oShowCD.m_refCapability) {
		oShowCD.m_refCapability->setListenerVol(oShowCD.m_fListenerVol);
		//redrawContents();
	}
}
void WanderWindow::onSourceSelectionChanged() noexcept
{
	if (m_bRecreateSourcesListInProgress) {
		return;
	}

	Glib::RefPtr<Gtk::TreeSelection> refTreeSelection = m_p0TreeViewSourcesList->get_selection();

	Gtk::TreeModel::iterator it = refTreeSelection->get_selected();
	if (it)	{
		Gtk::TreeModel::Row oRow = *it;
		const int32_t nCurrentSource = oRow[m_oSourceColumns.m_oColSourceIdHidden];
		if (nCurrentSource != m_oWanderData.m_nCurrentSource) {
			m_oWanderData.m_nCurrentSource = nCurrentSource;
			refreshCurrentSource();
			m_p0DrawingAreaShow->resetButtons();
			redrawContents();
		}
	}
}
void WanderWindow::onSpinFilenameChanged() noexcept
{
	if (m_bRecreateSourcesListInProgress) {
		return;
	}

//std::cout << "WanderWindow::onSpinFilenameChanged " << Glib::filename_to_utf8() << '\n';
	WanderData::SourceData& oSD = m_oWanderData.m_aSourceDatas[m_oWanderData.m_nCurrentSource];
	oSD.m_nSoundPathIdx = m_p0SpinFilenameIdx->get_value();
	std::string sDisplayName;
	if (oSD.m_nSoundPathIdx >= 0) {
		sDisplayName = Glib::filename_to_utf8(m_oWanderData.m_aSoundPaths[oSD.m_nSoundPathIdx]);
		if (sDisplayName.size() > 20) {
			sDisplayName = "..." + sDisplayName.substr(sDisplayName.size() - 20);
		}
	} else {
		sDisplayName = "-";
	}
	m_p0LabelFilePath->set_text(sDisplayName);
}
void WanderWindow::onSoundLoopingToggled() noexcept
{
	if (m_bRecreateSourcesListInProgress) {
		return;
	}

	WanderData::SourceData& oSD = m_oWanderData.m_aSourceDatas[m_oWanderData.m_nCurrentSource];
	oSD.m_bLooping = m_p0CheckButtonLooping->get_active();
}
void WanderWindow::onSpinSourcePosXChanged() noexcept
{
//std::cout << "WanderWindow::onSpinSourcePosXChanged " << m_p0SpinSourcePosX->get_value() << '\n';
	WanderData::SourceData& oSD = m_oWanderData.m_aSourceDatas[m_oWanderData.m_nCurrentSource];
	oSD.m_oPos.m_fX = m_p0SpinSourcePosX->get_value();

	if (oSD.isPlaying()) {
		oSD.m_refPlayback->setSoundPos(oSD.m_nSoundId, oSD.m_bListenerRelative, oSD.m_oPos.m_fX, oSD.m_oPos.m_fY, oSD.m_oPos.m_fZ);
		redrawContents();
	}
}
void WanderWindow::onSpinSourcePosYChanged() noexcept
{
	if (m_bRecreateSourcesListInProgress) {
		return;
	}

	WanderData::SourceData& oSD = m_oWanderData.m_aSourceDatas[m_oWanderData.m_nCurrentSource];
	oSD.m_oPos.m_fY = m_p0SpinSourcePosY->get_value();

	if (oSD.isPlaying()) {
		oSD.m_refPlayback->setSoundPos(oSD.m_nSoundId, oSD.m_bListenerRelative, oSD.m_oPos.m_fX, oSD.m_oPos.m_fY, oSD.m_oPos.m_fZ);
		redrawContents();
	}
}
void WanderWindow::onSpinSourcePosZChanged() noexcept
{
	if (m_bRecreateSourcesListInProgress) {
		return;
	}

	WanderData::SourceData& oSD = m_oWanderData.m_aSourceDatas[m_oWanderData.m_nCurrentSource];
	oSD.m_oPos.m_fZ = m_p0SpinSourcePosZ->get_value();

	if (oSD.isPlaying()) {
		oSD.m_refPlayback->setSoundPos(oSD.m_nSoundId, oSD.m_bListenerRelative, oSD.m_oPos.m_fX, oSD.m_oPos.m_fY, oSD.m_oPos.m_fZ);
		redrawContents();
	}
}
void WanderWindow::onSpinSourceVolChanged() noexcept
{
	if (m_bRecreateSourcesListInProgress) {
		return;
	}

	WanderData::SourceData& oSD = m_oWanderData.m_aSourceDatas[m_oWanderData.m_nCurrentSource];
	oSD.m_fVol = m_p0SpinSourceVol->get_value();

	if (oSD.isPlaying()) {
		oSD.m_refPlayback->setSoundVol(oSD.m_nSoundId, oSD.m_fVol);
		//redrawContents();
	}
}

void WanderWindow::onSoundListenerRelativeToggled() noexcept
{
//std::cout << "WanderWindow::onSoundListenerRelativeToggled A " << '\n';
	if (m_bRecreateSourcesListInProgress) {
		return;
	}

	WanderData::SourceData& oSD = m_oWanderData.m_aSourceDatas[m_oWanderData.m_nCurrentSource];
	oSD.m_bListenerRelative = m_p0CheckButtonListenerRelative->get_active();

	if (oSD.isPlaying()) {
		oSD.m_refPlayback->setSoundPos(oSD.m_nSoundId, oSD.m_bListenerRelative, oSD.m_oPos.m_fX, oSD.m_oPos.m_fY, oSD.m_oPos.m_fZ);
		redrawContents();
	}
}

void WanderWindow::onButtonSoundPlay() noexcept
{
	WanderData::SourceData& oSD = m_oWanderData.m_aSourceDatas[m_oWanderData.m_nCurrentSource];
	assert((oSD.m_nSoundId < 0) || oSD.m_bPaused);
	if (oSD.m_bPaused) {
		assert(oSD.m_nSoundId >= 0);
		oSD.m_refPlayback->resumeSound(oSD.m_nSoundId);
		oSD.m_bPaused = false;
	} else {
		if (oSD.m_nSoundPathIdx < 0) {
			printStringToEvents("Please chhose a sound first!");
			return; //-----------------------------------------------------
		}
		const std::string& sSoundPath = m_oWanderData.m_aSoundPaths[oSD.m_nSoundPathIdx];
		const int32_t nCapaIdx = getCapabilityDataIndexFromDeviceId(oSD.m_nPlaybackDeviceId);
		WanderData::CapabilityData& oCD = m_oWanderData.m_aCapabilityDatas[nCapaIdx];
		auto& aFilePathToId = oCD.m_aFilePathToId;
		const auto itFind = std::find_if(aFilePathToId.begin(), aFilePathToId.end(), [&](const std::pair<std::string, int32_t>& oPair)
		{
			return (oPair.first == sSoundPath);
		});
		if (itFind == aFilePathToId.end()) {
			stmi::PlaybackCapability::SoundData oData = oSD.m_refPlayback->playSound(sSoundPath, oSD.m_fVol, oSD.m_bLooping
																					, oSD.m_bListenerRelative, oSD.m_oPos.m_fX, oSD.m_oPos.m_fY, oSD.m_oPos.m_fZ);
			oSD.m_nSoundId = oData.m_nSoundId;
			aFilePathToId.push_back(std::make_pair(sSoundPath, oData.m_nFileId));
		} else {
			const int32_t nFileId = itFind->second;
			oSD.m_nSoundId = oSD.m_refPlayback->playSound(nFileId, oSD.m_fVol, oSD.m_bLooping
														, oSD.m_bListenerRelative, oSD.m_oPos.m_fX, oSD.m_oPos.m_fY, oSD.m_oPos.m_fZ);
		}
	}
	refreshCurrentSource();
	redrawContents();
}
void WanderWindow::onButtonSoundPause() noexcept
{
	WanderData::SourceData& oSD = m_oWanderData.m_aSourceDatas[m_oWanderData.m_nCurrentSource];
	assert((oSD.m_nSoundId >= 0) && ! oSD.m_bPaused);
	oSD.m_refPlayback->pauseSound(oSD.m_nSoundId);
	oSD.m_bPaused = true;

	refreshCurrentSource();
}
void WanderWindow::onButtonSoundStop() noexcept
{
	WanderData::SourceData& oSD = m_oWanderData.m_aSourceDatas[m_oWanderData.m_nCurrentSource];
	assert(oSD.m_nSoundId >= 0);
	oSD.m_refPlayback->stopSound(oSD.m_nSoundId);
	oSD.m_nSoundId = -1;
	oSD.m_bPaused = false;

	refreshCurrentSource();
	redrawContents();
}

void WanderWindow::onButtonDevicePause() noexcept
{
	if (m_oWanderData.m_nCurrentShowDeviceIdx < 0) {
		return;
	}
	WanderData::CapabilityData& oShowCD = m_oWanderData.m_aCapabilityDatas[m_oWanderData.m_nCurrentShowDeviceIdx];
	oShowCD.m_refCapability->pauseDevice();
	oShowCD.m_bDevicePaused = true;

	refreshCurrentShow();
}
void WanderWindow::onButtonDeviceResume() noexcept
{
	if (m_oWanderData.m_nCurrentShowDeviceIdx < 0) {
		return;
	}
	WanderData::CapabilityData& oShowCD = m_oWanderData.m_aCapabilityDatas[m_oWanderData.m_nCurrentShowDeviceIdx];
	oShowCD.m_refCapability->resumeDevice();
	oShowCD.m_bDevicePaused = false;

	refreshCurrentShow();
}
void WanderWindow::onButtonSoundsStopAll() noexcept
{
	for (WanderData::CapabilityData& oCD : m_oWanderData.m_aCapabilityDatas) {
		oCD.m_refCapability->stopAllSounds();;
	}

	int32_t nSourceId = 0;
	for (WanderData::SourceData& oSD : m_oWanderData.m_aSourceDatas) {
		oSD.m_nSoundId = -1;
		oSD.m_bPaused = false;
		if (nSourceId == m_oWanderData.m_nCurrentSource) {
			refreshCurrentSource();
		}
		++nSourceId;
	}

	refreshCurrentSource();
	redrawContents();
}

void WanderWindow::onDeviceSelectionChanged() noexcept
{
	if (m_bRecreateSourcesListInProgress) {
		return;
	}
	if (m_bRecreateDevicesListInProgress) {
		return;
	}

	WanderData::SourceData& oSD = m_oWanderData.m_aSourceDatas[m_oWanderData.m_nCurrentSource];

	if (oSD.isPlaying()) {
		// If current source is playing slection shouldn't be changed
		return; //----------------------------------------------------------
	}

	Glib::RefPtr<Gtk::TreeSelection> refTreeSelection = m_p0TreeViewDevicesList->get_selection();

	Gtk::TreeModel::iterator it = refTreeSelection->get_selected();
	if (it)	{
		Gtk::TreeModel::Row oRow = *it;
		const int32_t nDeviceId = oRow[m_oDeviceColumns.m_oColDeviceId];
		if (nDeviceId != oSD.m_nPlaybackDeviceId) {
			oSD.m_nPlaybackDeviceId = nDeviceId;
			const int32_t nIdx = getCapabilityDataIndexFromDeviceId(nDeviceId);
			oSD.m_refPlayback = m_oWanderData.m_aCapabilityDatas[nIdx].m_refCapability;
			//
			m_p0DrawingAreaShow->resetButtons();
			refreshCurrentSource();
		}
	}
}
void WanderWindow::onShowDeviceSelectionChanged() noexcept
{
	if (m_bRecreateDevicesListInProgress) {
		return;
	}

	Glib::RefPtr<Gtk::TreeSelection> refTreeSelection = m_p0TreeViewShowDevicesList->get_selection();
	Gtk::TreeModel::iterator it = refTreeSelection->get_selected();
	if (it)	{
		Gtk::TreeModel::Row oRow = *it;
		const int32_t nDeviceId = oRow[m_oDeviceColumns.m_oColDeviceId];
		m_oWanderData.m_nCurrentShowDeviceIdx = getCapabilityDataIndexFromDeviceId(nDeviceId);
	} else {
		m_oWanderData.m_nCurrentShowDeviceIdx = -1;
	}
	refreshCurrentShow();
}

void WanderWindow::printStringToEvents(const std::string& sStr) noexcept
{
//std::cout << "WanderWindow::printStringToEvents " << sStr << '\n';
	if (m_nTextBufferEventsTotLines >= s_nTextBufferEventsMaxLines) {
		auto itLine1 = m_refTextBufferEvents->get_iter_at_line(1);
		m_refTextBufferEvents->erase(m_refTextBufferEvents->begin(), itLine1);
		--m_nTextBufferEventsTotLines;
	}
	m_refTextBufferEvents->insert(m_refTextBufferEvents->end(), sStr + "\n");
	++m_nTextBufferEventsTotLines;

	m_refTextBufferEvents->place_cursor(m_refTextBufferEvents->end());
	m_refTextBufferEvents->move_mark(m_refTextBufferEventsBottomMark, m_refTextBufferEvents->end());
	m_p0TextViewEvents->scroll_to(m_refTextBufferEventsBottomMark, 0.1);
}
void WanderWindow::printToEvents(stmi::DeviceMgmtEvent& oEvent) noexcept
{
	const stmi::DeviceMgmtEvent::DEVICE_MGMT_TYPE eType = oEvent.getDeviceMgmtType();
	std::string sType;
	if (eType == stmi::DeviceMgmtEvent::DEVICE_MGMT_ADDED) {
		sType = "Added  ";
	} else if (eType == stmi::DeviceMgmtEvent::DEVICE_MGMT_CHANGED) {
		sType = "Changed";
	} else if (eType == stmi::DeviceMgmtEvent::DEVICE_MGMT_REMOVED) {
		sType = "Removed";
	} else {
		sType = "Error  ";
	}
	auto refDevice = oEvent.getDevice();
	const auto sDevice = (refDevice ? refDevice->getName() : "(no device)");
	const int32_t nDevice = (refDevice ? refDevice->getId() : -1);
	const auto sShow = Glib::ustring::compose("DeviceMgmtEvent: device %3\n - %2 %1"
											, sDevice, nDevice, sType);
	printStringToEvents(sShow);
}
void WanderWindow::printToEvents(stmi::SndFinishedEvent& oEvent) noexcept
{
	const stmi::SndFinishedEvent::FINISHED_TYPE eFT = oEvent.getFinishedType();
	std::string sType;
	if (eFT == stmi::SndFinishedEvent::FINISHED_TYPE_COMPLETED ) {
		sType = "Completed     ";
	} else if (eFT == stmi::SndFinishedEvent::FINISHED_TYPE_ABORTED) {
		sType = "Aborted       ";
	} else if (eFT == stmi::SndFinishedEvent::FINISHED_TYPE_LISTENER_REMOVED) {
		sType = "Listener gone ";
	} else if (eFT == stmi::SndFinishedEvent::FINISHED_TYPE_FILE_NOT_FOUND) {
		sType = "File Not Found";
	} else {
		sType = "Error         ";
	}
	const int32_t nSoundId = oEvent.getSoundId();
	auto refCapability = oEvent.getCapability();
	auto refDevice = (refCapability ? refCapability->getDevice() : shared_ptr<stmi::Device>{});
	const auto sDevice = (refDevice ? refDevice->getName() : "(no device)");
	const int32_t nDevice = (refDevice ? refDevice->getId() : -1);
	const auto sShow = Glib::ustring::compose("SndFinishedEvent: device\n - %2 %1\n - sound: %3 %4"
											, sDevice, nDevice, sType, nSoundId);
	printStringToEvents(sShow);
}


} // namespace wander

} // namespace example
