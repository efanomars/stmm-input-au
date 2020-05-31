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
 * File:   private_recycler.h
 */

#ifndef STMI_RECYCLER_H
#define STMI_RECYCLER_H

#include <cassert>
#include <iostream>
#include <vector>
#include <memory>


namespace stmi
{

namespace Private
{

////////////////////////////////////////////////////////////////////////////////
/** Recycling factory for shared_ptr wrapped classes.
 */
template <class T, class B = T>
class Recycler final
{
public:
	Recycler() noexcept = default;

	/** Construct or recycle the shared_ptr wrapped instance of T.
	 * T must be same or subclass of B.
	 * T must have a constructor T(const P& ... oParam)
	 * and public member function reInit(const P& ... oParam) with the same params.
	 */
	template <typename ...P>
	void create(std::shared_ptr<B>& refOutB, const P& ... oParam)
	{
		static_assert(std::is_base_of<B,T>::value, "Wrong type.");
		for (auto& refB : m_oAll) {
			if (refB.use_count() == 1) {
//#ifndef NDEBUG
//static int32_t nCount = 0;
//std::cout << "recycled " << nCount << '\n';
//++nCount;
//#endif //NDEBUG
				T* p0T = static_cast<T*>(refB.get());
				p0T->reInit(oParam...);
				refOutB = refB;
				return; //------------------------------------------------------
			}
		}
		// not found: create new instance
		if (std::is_same<B,T>::value) {
			m_oAll.emplace_back(std::make_shared<T>(oParam...));
		} else {
			m_oAll.emplace_back(std::shared_ptr<B>(new T(oParam...)));
		}
		refOutB = m_oAll.back();
	}
private:
	std::vector< std::shared_ptr<B> > m_oAll;
private:
	Recycler(const Recycler& oSource) = delete;
	Recycler& operator=(const Recycler& oSource) = delete;
};

} // namespace Private

} // namespace stmi

#endif /* STMI_RECYCLER_H */

