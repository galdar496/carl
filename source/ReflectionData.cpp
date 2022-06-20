//
//  ReflectionData.cpp
//  carl
//
//  Created by Cody White on 6/19/22.
//  Copyright (c) 2022 Cody White. All rights reserved.
//

#include "ReflectionData.h"
#include "ReflectedVariable.h"
#include "PointerTable.h"
#include "ReflectionUtilities.h"

#include <assert.h>
#include <iostream>

namespace carl {

// ReflectedMember implementation begin ------------------------------------------------------

ReflectedMember::ReflectedMember(const std::string &name, size_t offset, size_t size, bool isPointer, const ReflectionData *reflectionData)
: m_name(name)
, m_offset(offset)
, m_size(size)
, m_isPointer(isPointer)
, m_data(reflectionData)
{
}

ReflectedMember::~ReflectedMember()
{
}

bool ReflectedMember::isArray() const
{
	// If this is an array, m_size will contain the size of of the entire array
	// whereas the reflection data will contain the size of just one element of
	// the array.
	return (m_size > m_data->size());
}

// ReflectedMember implementation end --------------------------------------------------------

// ReflectionData implementation begin -------------------------------------------------------

ReflectionData::ReflectionData() 
: m_name("")
, m_size(0)
, m_serializeFunction(nullptr)
, m_parent(nullptr)
{
}
    
ReflectionData::~ReflectionData()
{
    for (auto &member : m_members) {
        delete member;
    }
}
    
void ReflectionData::init(ReflectionDataCInfo &info)
{
	assert(info.size > 0);
	assert(info.name.length());
	assert(info.allocateFunction != nullptr);

	m_name = info.name;
	m_size = info.size;
	m_allocateInstanceFunction = info.allocateFunction;
}

const ReflectedMember *ReflectionData::member(const std::string &name) const
{
    for (auto &member : m_members) {
        if (member->name() == name) {
            return member;
        }
    }

	return nullptr;
}
    
void padStream(std::ostream &stream, size_t pad)
{
    for (size_t ii = 0; ii < pad; ++ii) {
        stream << "\t";
    }
}

void ReflectionData::serialize(const ReflectedVariable *variable, std::ostream &stream, PointerTable &pointerTable, size_t padding, bool isArray) const
{
	// If this object has a parent, serialize its data first.
	if (m_parent) {
		m_parent->serialize(variable, stream, pointerTable, padding);
	}

    // If this type has a valid serialization function then it knows how to serialize itself, let it.
    if (m_serializeFunction) {
        m_serializeFunction(variable, stream);
        return;
    }
    
    // For each member of this type, ask it to serialize itself.

	// Write out the name of this type.
	if (!isArray) {
		stream << pointerTable.index(*variable) << " ";
	}

	stream << m_name << std::endl;

	// Make sure the instance data for this object is valid (could be a null pointer).
	if (variable->instanceData() == nullptr) {
		padStream(stream, padding);
		stream << "[" << std::endl;
		++padding;
		padStream(stream, padding);
		stream << "null" << std::endl;
		--padding;
		padStream(stream, padding);
		stream << "]" << std::endl;
		return;
	}

	padStream(stream, padding);
    stream << "[" << std::endl;
    ++padding;
    for (auto &member : m_members) {
		padStream(stream, padding);

		// If this is a pointer type, serialize its index in the pointer table.
		if (member->isPointer()) {
			void *offsetData = pointerOffset(variable->instanceData(), member->offset());
			ReflectedVariable memberVariable(member->reflectionData(), offsetData);

			void *pointerData = &(*(memberVariable.value<char *>()));
			ReflectedVariable resolvedPointer(member->reflectionData(), pointerData);

			stream << member->name() << " " << pointerTable.index(resolvedPointer) << std::endl;
		}
		// If this type is an array, we have to serialize each element of the array before moving on 
		// to the next member variable.
		else if (member->isArray()) {
			stream << member->name() << std::endl;
			++padding;
			const ReflectionData *data = member->reflectionData();
			size_t baseTypeSize = data->size();
			assert(baseTypeSize > 0);
			for (size_t ii = 0; ii < member->size(); ii += baseTypeSize) {
				padStream(stream, padding);

				// Get the next element to serialize.
				void *offsetData = pointerOffset(variable->instanceData(), member->offset() + ii);
				ReflectedVariable arrayElement(data, offsetData);
				data->serialize(&arrayElement, stream, pointerTable, padding, true);
			}
			--padding;
		} else { // non-array/pointer type.
			stream << member->name() << " ";
			void *offsetData = pointerOffset(variable->instanceData(), member->offset());
			ReflectedVariable memberVariable(member->reflectionData(), offsetData);
			member->reflectionData()->serialize(&memberVariable, stream, pointerTable, padding, false);
		}
    }

    --padding;
    padStream(stream, padding);
	stream << "]" << std::endl;
}

void ReflectionData::deserialize(ReflectedVariable *variable, std::istream &stream, PointerTable &pointerTable, bool isArray) const
{
	// If this object has a parent, deserialize its data first.
	if (m_parent) {
		m_parent->deserialize(variable, stream, pointerTable, isArray);
	}

	// If this type has a valid deserialization function then it knows how to deserialize itself, let it.
	if (m_deserializeFunction) {
		m_deserializeFunction(variable, stream);
		return;
	}

	// For each member read from this object, ask it to deserialize itself if we have a definition for it.

	std::string streamInput;

	// Read the pointer table index and typename first if we're not deserializing an array.
	PointerTable::TableIndex tableIndex = 0;
	if (!isArray) {
		stream >> tableIndex;
		assert(tableIndex >= 0);
	}

	stream >> streamInput;
	assert(streamInput == m_name);

	// Read the starting bracket denoting the start of member variables for this type.
	{
		stream >> streamInput;
		assert(streamInput == "[");
	}

	while (streamInput != "]") {
		// Read in the type.
		stream >> streamInput;
        assert(stream);

		// Handle deserializing a NULL pointer. In this case, there will be no other
		// members to deserialize as this instance has no data.
		if (streamInput == "null") {
			variable->setInstanceData(nullptr);
			continue;
		}
	
		const ReflectedMember *member = this->member(streamInput);
		if (member) {
			if (member->isPointer()) {
				// Read in the index for this pointer that corresponds to the pointer table.
				PointerTable::TableIndex pointerIndex = 0;
				stream >> pointerIndex;
				assert(pointerIndex >= 0);

				void *offsetData = pointerOffset(variable->instanceData(), member->offset());
				ReflectedVariable memberVariable(member->reflectionData(), offsetData);

				// Add this pointer to the patch table to deffer resolving it until the pointer table
				// has been entirely deserialized.
				pointerTable.addPatchPointer(pointerIndex, memberVariable);
 			} else if (member->isArray()) { // If this member is an array type, read in each element of the array individually.
				const ReflectionData *data = member->reflectionData();
				size_t baseTypeSize = data->size();
				for (size_t ii = 0; ii < member->size(); ii += baseTypeSize) {
					// Get the next element to serialize.
					void *offsetData = pointerOffset(variable->instanceData(), member->offset() + ii);
					ReflectedVariable arrayElement(data, offsetData);
					data->deserialize(&arrayElement, stream, pointerTable, true);
				}
			} else { // non-array/pointer type type.
				void *offsetData = pointerOffset(variable->instanceData(), member->offset());
				ReflectedVariable memberVariable(member->reflectionData(), offsetData);
				member->reflectionData()->deserialize(&memberVariable, stream, pointerTable, false);
			}
		}
	}

	// Add this variable to the pointer table if it hasn't already been added by a base type.
	if (!m_parent) {
		ReflectedVariable &tableVariable = const_cast<ReflectedVariable&>(pointerTable.pointer(tableIndex));
		tableVariable = *variable;
	}
}
    
// ReflectionData implementation end ---------------------------------------------------------

} // namespace carl