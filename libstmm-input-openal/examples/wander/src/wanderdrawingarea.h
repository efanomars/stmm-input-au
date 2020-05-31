/*
 * File:   wanderdrawingarea.h
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

#ifndef STMI_WANDER_DRAWING_AREA_H
#define STMI_WANDER_DRAWING_AREA_H

#include "wanderdata.h"

#include <gtkmm/drawingarea.h>
#include <cairomm/refptr.h>
#include <cairomm/region.h>

#include <memory>

#include <stdint.h>

namespace Cairo { class Context; }

namespace example
{

namespace wander
{

class WanderWindow;

using std::shared_ptr;

class WanderDrawingArea  : public Gtk::DrawingArea
{
public:
	WanderDrawingArea(WanderData& oWanderData, WanderWindow* p0WanderWindow) noexcept;

	void redrawContents() noexcept;

	void resetButtons() noexcept;
protected:
	bool on_button_press_event(GdkEventButton* p0GdkEvent) override;
	bool on_button_release_event(GdkEventButton* p0GdkEvent) override;
	bool on_motion_notify_event(GdkEventMotion* p0GdkEvent) override;

private:
	bool onCustomDraw(const Cairo::RefPtr<Cairo::Context>& refCc) noexcept;
	bool onCheckForUpdates() noexcept;

	//bool isRealized() noexcept;

	void setCanvasSize(int32_t nPixW, int32_t nPixH) noexcept;
	void drawShowData(const Cairo::RefPtr<Cairo::Context>& refCc) noexcept;

	void moveIt(bool bListener, double fAbsX, double fAbsY) noexcept;

private:
	//bool m_bIsRealized;

	//Cairo::RefPtr<Cairo::Region> m_refCurrentDrawingRegion;

	//Glib::Timer& m_oTimer;
	WanderData& m_oWanderData;
	WanderWindow* m_p0WanderWindow;

	sigc::connection m_oCheckForUpdatesConn;
	bool m_bUpdate = false;

	int32_t m_nCanvasPixW;
	int32_t m_nCanvasPixH;

	bool m_bButtonListenerPressed = false;
	bool m_bButtonSourcePressed = false;
};

} // namespace wander

} // namespace example

#endif	/* STMI_WANDER_DRAWING_AREA_H */

