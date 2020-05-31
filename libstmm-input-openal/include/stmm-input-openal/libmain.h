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
 * File:   libmain.h
 */

#ifndef STMI_OPENAL_DEVICE_MANAGER_LIB_MAIN_H
#define STMI_OPENAL_DEVICE_MANAGER_LIB_MAIN_H

#include <stmm-input/event.h>

#include <memory>
#include <string>
#include <vector>

namespace stmi { class ChildDeviceManager; }


#ifdef __cplusplus
extern "C" {
#endif

using std::shared_ptr;

//#pragma clang diagnostic push
//#pragma clang diagnostic ignored "-Wreturn-type-c-linkage"

shared_ptr<stmi::ChildDeviceManager> createPlugin(const std::string& sAppName
												, bool bEnableEventClasses, const std::vector<stmi::Event::Class>& aEnDisableEventClasses);

//#pragma clang diagnostic pop

#ifdef __cplusplus
}
#endif

#endif /* STMI_OPENAL_DEVICE_MANAGER_LIB_MAIN_H */

