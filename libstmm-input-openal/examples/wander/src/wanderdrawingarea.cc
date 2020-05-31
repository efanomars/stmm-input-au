/*
 * File:   wanderdrawingarea.cc
 *
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

#include "wanderdrawingarea.h"

#include "wanderwindow.h"

#include <gtkmm.h>
#include <cairomm/context.h>

//#include <iostream>
//#include <cassert>
#include <algorithm>
#include <utility>


namespace example
{

namespace wander
{

static constexpr const int32_t s_nCheckForUpdatesEachMillisec = 200;

WanderDrawingArea::WanderDrawingArea(WanderData& oWanderData, WanderWindow* p0WanderWindow) noexcept
: m_oWanderData(oWanderData)
, m_p0WanderWindow(p0WanderWindow)
{
	signal_draw().connect(sigc::mem_fun(*this, &WanderDrawingArea::onCustomDraw));

	m_oCheckForUpdatesConn = Glib::signal_timeout().connect(
			sigc::mem_fun(*this, &WanderDrawingArea::onCheckForUpdates), s_nCheckForUpdatesEachMillisec);

	const Gdk::EventMask oCurMask = get_events();
	const Gdk::EventMask oNewMask = oCurMask | Gdk::BUTTON_PRESS_MASK | Gdk::POINTER_MOTION_MASK | Gdk::BUTTON_RELEASE_MASK;
	if (oNewMask != oCurMask) {
		set_events(oNewMask);
	}
}

bool WanderDrawingArea::onCheckForUpdates() noexcept
{
//std::cout << "WanderDrawingArea::onCheckForUpdates() 1" << '\n';
	if (! m_bUpdate) {
		return true;
	}
//std::cout << "WanderDrawingArea::onCheckForUpdates() 2" << '\n';
	Glib::RefPtr<Gdk::Window> refGdkWindow = get_window();
	if (!refGdkWindow) {
		return true;
	}
//std::cout << "WanderDrawingArea::onCheckForUpdates() 3" << '\n';
	m_bUpdate = false;

	Gtk::Allocation oAllocation = get_allocation();
	const int nAlloPixW = oAllocation.get_width();
	const int nAlloPixH = oAllocation.get_height();
	Cairo::RectangleInt oRect;
	oRect.x = 0;
	oRect.y = 0;
	oRect.width = nAlloPixW;
	oRect.height = nAlloPixH;
	auto refCurrentDrawingRegion = Cairo::Region::create(oRect);

	auto refDrawingCtx = refGdkWindow->begin_draw_frame(refCurrentDrawingRegion);
	auto refCc = refDrawingCtx->get_cairo_context();

	setCanvasSize(nAlloPixW, nAlloPixH);

	drawShowData(refCc);

	refGdkWindow->end_draw_frame(refDrawingCtx);
	return true;
}
void WanderDrawingArea::setCanvasSize(int32_t nPixW, int32_t nPixH) noexcept
{
	m_nCanvasPixW = ((nPixW < 35) ? 35 : nPixW);
	m_nCanvasPixH = ((nPixH < 35) ? 35 : nPixH);
}
void WanderDrawingArea::redrawContents() noexcept
{
	//gdk_window_invalidate_rect(get_window()->gobj(), nullptr, true);
	m_bUpdate = true;
}

bool WanderDrawingArea::onCustomDraw(const Cairo::RefPtr<Cairo::Context>& refCc) noexcept
{
//std::cout << "WanderDrawingArea::onCustomDraw() time: " << m_oTimer.elapsed() << '\n';

	// This is where we draw on the window
	Glib::RefPtr<Gdk::Window> refWindow = get_window();
	if (!refWindow) {
		return true; //---------------------------------------------------------
	}

	Gtk::Allocation oAllocation = get_allocation();

	//const int32_t nAlloPixX = oAllocation.get_x();
	//const int32_t nAlloPixY = oAllocation.get_y();
	const int32_t nAlloPixW = oAllocation.get_width();
	const int32_t nAlloPixH = oAllocation.get_height();

//~ std::cout << "WanderDrawingArea::onCustomDraw() "
//~ << "  w=" << nAlloPixW
//~ << "  h=" << nAlloPixH
//~ << '\n';

	setCanvasSize(nAlloPixW, nAlloPixH);

	drawShowData(refCc);

	return true;
}

void WanderDrawingArea::drawShowData(const Cairo::RefPtr<Cairo::Context>& refCc) noexcept
{
	const int32_t nPixW = m_nCanvasPixW;
	const int32_t nPixH = m_nCanvasPixH;

	const double fScaleX = 1.0 * nPixW / m_oWanderData.m_oAreaSize.m_fW;
	const double fScaleY = 1.0 * nPixH / m_oWanderData.m_oAreaSize.m_fH;

	const double fScale = ((fScaleX < fScaleY) ? fScaleX : fScaleY);
	const int32_t nShowPixW = fScale * m_oWanderData.m_oAreaSize.m_fW;
	const int32_t nShowPixH = fScale * m_oWanderData.m_oAreaSize.m_fH;

	const int32_t nShowPixX = (nPixW - nShowPixW) / 2;
	const int32_t nShowPixY = (nPixH - nShowPixH) / 2;
//~ std::cout << "WanderDrawingArea::onCustomDraw() "
//~ << "  fScaleX=" << fScaleX
//~ << "  fScaleY=" << fScaleY
//~ << "  fScale=" << fScale
//~ << '\n';
//~ std::cout << "WanderDrawingArea::onCustomDraw() "
//~ << "  nShowPixX=" << nShowPixX
//~ << "  nShowPixY=" << nShowPixY
//~ << "  nShowPixW=" << nShowPixW
//~ << "  nShowPixH=" << nShowPixH
//~ << '\n';

	refCc->save();
	refCc->set_operator(Cairo::OPERATOR_CLEAR);
	refCc->rectangle(0, 0, nPixW, nPixH);
	refCc->fill();
	refCc->restore();


	refCc->save();
	refCc->set_operator(Cairo::OPERATOR_SOURCE);
	refCc->rectangle(0, 0, nPixW, nPixH);
	refCc->set_source_rgb(128,128,128);
	refCc->fill();
	refCc->restore();

	const double fFlipY = -1.0; // ref sys is rotated 180 deg around x axis

	if (m_oWanderData.m_nCurrentShowDeviceIdx < 0) {
		return; //----------------------------------------------------------
	}
	const auto& oCD = m_oWanderData.m_aCapabilityDatas[m_oWanderData.m_nCurrentShowDeviceIdx];
	const double fX = oCD.m_oListenerPos.m_fX;
	const double fY = oCD.m_oListenerPos.m_fY;
	const double fPixX = (fX - m_oWanderData.m_oAreaPos.m_fX) * fScale;
	const double fPixY = (fFlipY * fY - m_oWanderData.m_oAreaPos.m_fY) * fScale;

	refCc->save();
	refCc->set_operator(Cairo::OPERATOR_SOURCE);
	refCc->rectangle(nShowPixX + fPixX - 5, nShowPixY + fPixY - 5, 10, 10);
	refCc->set_source_rgb(128,0,0);
	refCc->fill();
	refCc->restore();

	for (const WanderData::SourceData& oSD : m_oWanderData.m_aSourceDatas) {
//~ std::cout << "WanderDrawingArea::drawShowData() "
//~ << oSD.m_nSoundId
//~ << '\n';
		if (oSD.m_refPlayback != oCD.m_refCapability) {
			continue;
		}
		if (! oSD.isPlaying()) {
			continue;
		}
		double fX = oSD.m_oPos.m_fX;
		double fY = oSD.m_oPos.m_fY;
		if (oSD.m_bListenerRelative) {
			fX += oCD.m_oListenerPos.m_fX;
			fY += oCD.m_oListenerPos.m_fY;
		}
		const double fPixX = (fX - m_oWanderData.m_oAreaPos.m_fX) * fScale;
		const double fPixY = (fFlipY * fY - m_oWanderData.m_oAreaPos.m_fY) * fScale;
		
		refCc->save();
		refCc->set_operator(Cairo::OPERATOR_SOURCE);
		refCc->rectangle(nShowPixX + fPixX - 5, nShowPixY + fPixY - 5, 10, 10);
		refCc->set_source_rgb(0,0,128);
		refCc->fill();
		refCc->restore();
	}
}

bool WanderDrawingArea::on_button_press_event(GdkEventButton* p0GdkEvent)
{
//std::cout << "WanderDrawingArea::on_button_press_event" << '\n';
//std::cout << "  x=" << p0GdkEvent->x << '\n';
//std::cout << "  y=" << p0GdkEvent->y << '\n';
//std::cout << "  button=" << p0GdkEvent->button << '\n';
	if (m_oWanderData.m_nCurrentShowDeviceIdx < 0) {
		m_bButtonListenerPressed = false;
		m_bButtonSourcePressed = false;
		return true;
	}

	const bool bListener = (p0GdkEvent->button == 3);
	if (bListener) {
		if (m_bButtonListenerPressed) {
			// already pressed
			return true; //-------------------------------------------------
		}
		m_bButtonListenerPressed = true;
	} else {
		if (m_bButtonSourcePressed) {
			// already pressed
			return true; //-------------------------------------------------
		}
		m_bButtonSourcePressed = true;
	}
	moveIt(bListener, p0GdkEvent->x, p0GdkEvent->y);

	return true;
}
bool WanderDrawingArea::on_button_release_event(GdkEventButton* p0GdkEvent)
{
	if (m_oWanderData.m_nCurrentShowDeviceIdx < 0) {
		return true;
	}

	const bool bListener = (p0GdkEvent->button == 3);
	if (bListener) {
		m_bButtonListenerPressed = false;
	} else {
		m_bButtonSourcePressed = false;
	}

	moveIt(bListener, p0GdkEvent->x, p0GdkEvent->y);
	return true;
}
bool WanderDrawingArea::on_motion_notify_event(GdkEventMotion* p0GdkEvent)
{
	if (m_oWanderData.m_nCurrentShowDeviceIdx < 0) {
		return true;
	}
//std::cout << "WanderDrawingArea::on_motion_notify_event" << '\n';
//std::cout << "  x=" << p0GdkEvent->x << '\n';
//std::cout << "  y=" << p0GdkEvent->y << '\n';
//std::cout << "  button=" << p0GdkEvent->button << '\n';
	if (m_bButtonListenerPressed) {
		moveIt(true, p0GdkEvent->x, p0GdkEvent->y);
	}
	if (m_bButtonSourcePressed) {
		moveIt(false, p0GdkEvent->x, p0GdkEvent->y);
	}
	return true;
}
void WanderDrawingArea::moveIt(bool bListener, double fAbsX, double fAbsY) noexcept
{
	const double fFlipY = -1.0; // ref sys is rotated 180 deg around x axis

	const int32_t nPixW = m_nCanvasPixW;
	const int32_t nPixH = m_nCanvasPixH;

	const double fScaleX = 1.0 * nPixW / m_oWanderData.m_oAreaSize.m_fW;
	const double fScaleY = 1.0 * nPixH / m_oWanderData.m_oAreaSize.m_fH;

	const double fScale = ((fScaleX < fScaleY) ? fScaleX : fScaleY);
	const int32_t nShowPixW = fScale * m_oWanderData.m_oAreaSize.m_fW;
	const int32_t nShowPixH = fScale * m_oWanderData.m_oAreaSize.m_fH;

	const int32_t nShowPixX = (nPixW - nShowPixW) / 2;
	const int32_t nShowPixY = (nPixH - nShowPixH) / 2;

	bool bRefreshSource = false;
	auto& oCD = m_oWanderData.m_aCapabilityDatas[m_oWanderData.m_nCurrentShowDeviceIdx];
	if (bListener) {
		oCD.m_oListenerPos.m_fX = m_oWanderData.m_oAreaPos.m_fX + 1.0 * (fAbsX - nShowPixX) / fScale;
		oCD.m_oListenerPos.m_fY = fFlipY * (m_oWanderData.m_oAreaPos.m_fY + 1.0 * (fAbsY - nShowPixY) / fScale);
	} else {
		WanderData::SourceData& oSD = m_oWanderData.m_aSourceDatas[m_oWanderData.m_nCurrentSource];
		if (oSD.m_refPlayback == oCD.m_refCapability) {
			oSD.m_oPos.m_fX = m_oWanderData.m_oAreaPos.m_fX + 1.0 * (fAbsX - nShowPixX) / fScale;
			oSD.m_oPos.m_fY = fFlipY * (m_oWanderData.m_oAreaPos.m_fY + 1.0 * (fAbsY - nShowPixY) / fScale);
			if (oSD.m_bListenerRelative) {
				oSD.m_oPos.m_fX -= oCD.m_oListenerPos.m_fX;
				oSD.m_oPos.m_fY -= oCD.m_oListenerPos.m_fY;
			}
			bRefreshSource = true;
		}
	}

	m_p0WanderWindow->refreshCurrentShow();
	if (bRefreshSource) {
		m_p0WanderWindow->refreshCurrentSource();
	}
}
void WanderDrawingArea::resetButtons() noexcept
{
//std::cout << "WanderDrawingArea::resetButtons()" << '\n';
	m_bButtonListenerPressed = false;
	m_bButtonSourcePressed = false;
}

} // namespace wander

} // namespace example
