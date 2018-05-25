#if !defined(OMR_OM_TRANSITIONSET_INL_HPP_)
#define OMR_OM_TRANSITIONSET_INL_HPP_

#include <OMR/Om/Array.hpp>
#include <OMR/Om/TransitionSet.hpp>
#include <OMR/Om/MemArrayOperations.hpp>
#include <OMR/Om/Shape.hpp>
#include <OMR/Om/Context.hpp>

namespace OMR
{
namespace Om
{

/// TODO: Write barrier?
inline bool
initializeTransitionSet(Context &cx, MemHandle<TransitionSet> self)
{
	
	return initializeMemArray(cx,
							  MemHandle<MemArray<TransitionSetEntry>>(self, &TransitionSet::table),
							  32);
}

/// Store a transition entry into the transition set. Can fail if the transition set is full.
/// TODO: Make updates to table atomic
/// TODO: Do we want a write barrier on the transition stores
/// TODO: Make transition table resizable
inline bool
tryStoreTransition(TransitionSet &set, TransitionSetEntry entry, std::size_t hash)
{
	std::size_t sz = set.size();

	for (std::size_t i = 0; i < sz; i++)
	{
		std::size_t idx = (hash + i) % sz;
		if (set.table[idx].shape == nullptr)
		{
			set.table[idx] = entry;
			return true;
		}
	}
	return false;
}

/// Lookup a transition in the set, using a precalculated hash. Returns nullptr on failure.
inline Shape *
lookUpTransition(TransitionSet &set, Infra::Span<const SlotAttr> attributes, std::size_t hash)
{
	std::size_t size = set.size();

	for (std::size_t i = 0; i < size; i++)
	{
		std::size_t index = (hash + i) % size;
		Shape *shape = set.table.at(index).shape;

		if (shape == nullptr)
		{
			return nullptr;
		}
		if (attributes == shape->instanceSlotAttrs())
		{
			return shape;
		}
	}
	return nullptr;
}

/// Lookup a transition. Returns nullptr on failure.
inline Shape *
lookUpTransition(TransitionSet &set, Infra::Span<const SlotAttr> attributes)
{
	return lookUpTransition(set, attributes, hash(attributes));
}

} // namespace Om
} // namespace OMR

#endif // OMR_OM_TRANSITIONSET_INL_HPP_
