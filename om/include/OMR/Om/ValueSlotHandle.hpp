/*******************************************************************************
 *  Copyright (c) 2018, 2018 IBM and others
 *
 *  This program and the accompanying materials are made available under
 *  the terms of the Eclipse Public License 2.0 which accompanies this
 *  distribution and is available at https://www.eclipse.org/legal/epl-2.0/
 *  or the Apache License, Version 2.0 which accompanies this distribution and
 *  is available at https://www.apache.org/licenses/LICENSE-2.0.
 *
 *  This Source Code may also be made available under the following
 *  Secondary Licenses when the conditions for such availability set
 *  forth in the Eclipse Public License, v. 2.0 are satisfied: GNU
 *  General Public License, version 2 with the GNU Classpath
 *  Exception [1] and GNU General Public License, version 2 with the
 *  OpenJDK Assembly Exception [2].
 *
 *  [1] https://www.gnu.org/software/classpath/license.html
 *  [2] http://openjdk.java.net/legal/assembly-exception.html
 *
 *  SPDX-License-Identifier: EPL-2.0 OR Apache-2.0 OR GPL-2.0 WITH Classpath-exception-2.0 OR LicenseRef-GPL-2.0 WITH Assembly-exception
 *******************************************************************************/

#if !defined(OMR_OM_VALUESLOTHANDLE_HPP_)
#define OMR_OM_VALUESLOTHANDLE_HPP_

#include <OMR/Om/Value.hpp>

namespace OMR {
namespace Om {

class Any;

/// A handle to an object's slot. Slot constains a Value.
class ValueSlotHandle {
public:
	explicit constexpr ValueSlotHandle(Value* slot) : slot_(slot) {}

	explicit constexpr ValueSlotHandle(void* slot) : slot_((Value*)slot) {}

	void writeReference(void* ref) const { slot_->setRef(ref); }

	void writeReference(Value ref) const { *slot_ = ref; }

	void atomicWriteReference(void* ref) const { assert(0); }

	template<typename T = Any>
	Any* readReference() const {
		return slot_->getRef<T>();
	}

	bool isReference() const { return slot_->isRef(); }

	Value* slot() const { return slot_; }

private:
	Value* slot_;
};

/// A Handle to an object's slot. Slot contains an immutable Value.
class ConstValueSlotHandle {
public:
	ConstValueSlotHandle(ValueSlotHandle& other) : slot_(other.slot()) {}

	explicit constexpr ConstValueSlotHandle(const Value* slot) : slot_(slot) {}

	explicit constexpr ConstValueSlotHandle(const void* slot) : slot_((Value*)slot) {}

	template<typename T = Cell>
	T* readReference() const {
		return slot_->getRef<T>();
	}

	bool isReference() const { return slot_->isRef(); }

	const Value* slot() const { return slot_; }

private:
	const Value* slot_;
};

} // namespace Om
} // namespace OMR

#endif // OMR_OM_VALUESLOTHANDLE_HPP_
