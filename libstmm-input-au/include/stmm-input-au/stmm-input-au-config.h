/*
 * Copyright © 2019-2020  Stefano Marsili, <stemars@gmx.ch>
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
 * File:   stmm-input-au-config.h
 */

#ifndef STMI_STMM_INPUT_AU_LIB_CONFIG_H
#define STMI_STMM_INPUT_AU_LIB_CONFIG_H

namespace stmi
{

namespace libconfig
{

namespace au
{

/** The stmm-input-au library version.
 * @return The version string. Cannot be empty.
 */
const char* getVersion() noexcept;

} // namespace au

} // namespace libconfig

} // namespace stmi

#endif /* STMI_STMM_INPUT_AU_LIB_CONFIG_H */

