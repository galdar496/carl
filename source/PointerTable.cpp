//
//  PointerTable.cpp
//  carl
//
//  Created by Cody White on 6/19/22.
//  Copyright (c) 2022 Cody White. All rights reserved.
//

#include "PointerTable.h"
#include "ReflectionDataManager.h"
#include "ReflectionUtilities.h"

#include <assert.h>
#include <istream>

namespace carl {

void PointerTable::populate(const ReflectedVariable &reflectedVariable, bool needsSerialization)
{
	if (!hasPointer(reflectedVariable)) {
		// Add this object's instance to the table.
		addPointer(reflectedVariable, needsSerialization);

		if (reflectedVariable.instanceData() == nullptr) {
			// No need to keep processing this type, it is null.
			return;
		}

		// Loop over each of this member's variables and add them to the table using a depth-first
		// recursive call.
		const ReflectionData *reflectionData = reflectedVariable.reflectionData();
        for (auto &member : reflectionData->members()) {
			// Only add objects who also have data members.
			if (member->reflectionData()->hasDataMembers() || member->isPointer()) {
				void *offsetData = pointerOffset(reflectedVariable.instanceData(), member->offset());
				ReflectedVariable memberVariable(member->reflectionData(), offsetData);
				if (member->isPointer()) {
					void *pointerData = &(*(memberVariable.value<char *>()));
					ReflectedVariable resolvedPointer(member->reflectionData(), pointerData);
					
					// Tell the serialization code that this variable needs to be manually serialized
					// as we don't have direct access to it under the current object.
					populate(resolvedPointer, true);
				} else {
					populate(memberVariable, false);
				}
			}
		}
	}
}

const ReflectedVariable &PointerTable::pointer(TableIndex index)
{
	assert(index >= 0 && index < m_dataTable.size());
	return m_dataTable[index].variable;
}

PointerTable::TableIndex PointerTable::index(const ReflectedVariable &variable) const
{
	PointerAddress address = reinterpret_cast<PointerAddress>(variable.instanceData());
	LookupTable::const_iterator iter = m_lookupTable.find(address);
	
	assert(iter != m_lookupTable.end());

	// Look though the possible entires at this address to match up the typename to the passed
	// in variable type.
	const std::string &typeName = variable.reflectionData()->name();
    for (auto &object : iter->second) {
        if (object.reflectionData->name() == typeName) {
            return object.tableIndex;
        }
    }

	// Shouldn't ever get here.
	assert(0);
	return static_cast<TableIndex>(-1);
}

void PointerTable::serialize(std::ostream &stream)
{
	// First write out the size of the table.
	stream << m_dataTable.size() << std::endl;

	for (size_t ii = 0; ii < m_dataTable.size(); ++ii) {
		// Only serialize this object if it won't be serialized by some other object (is a child
		// of an object already being serialized).
		if (m_dataTable[ii].needsSerialization) {
			//stream << std::endl; 
			const ReflectionData *reflectionData = m_dataTable[ii].variable.reflectionData();
			if (reflectionData->hasParent()) {
				// Write out the name of this type (so that the deserializer knows what type that any base classes
				// belong to).
				stream << "(" << reflectionData->name() << ") ";
			}

			const ReflectedVariable *tableVariable = &(m_dataTable[ii].variable);

			// This function will auto-serialize all objects contained within the current object unless
			// they are only associated via pointers. In that case, only the index into the table will
			// be written.
			tableVariable->reflectionData()->serialize(tableVariable, stream, *this);
		}
	}

	stream.flush();
}

void PointerTable::deserialize(std::istream &stream)
{
	// The first thing in the stream should be the size of the pointer table.
	size_t tableSize = 0;
	stream >> tableSize;
	assert(tableSize > 0);

	m_dataTable.resize(tableSize);

	std::string streamInput;

	ReflectionDataManager &manager = ReflectionDataManager::instance();

	// Eat the newline character.
	stream.ignore(256, '\n');

	while (stream.peek() > 0) { // Valid characters have an ascii value > 0.
		// See if there is a derived type that we're about to read in.
		bool inheritedObject = false;
		if (stream.peek() == '(') {
			// Read in the name of the derived type.
			stream >> streamInput;

			// The name will have surrounding () symbols, get rid of them.
			streamInput = streamInput.substr(1, streamInput.length() - 2);

			inheritedObject = true;
		}

		// Save off this position in the stream so that the deserialization code and read it again.
		std::streamoff streamPosition = stream.tellg();

		// Read in the table index for this variable.
		TableIndex index;
		stream >> index;
		assert(index >= 0 && index < tableSize);

		// If this isn't a derived type, just read in its name and start working with it.
		if (!inheritedObject) {
			// Read in the type.
			stream >> streamInput;
		}

		const ReflectionData *reflectionData = manager.reflectionData(streamInput);
		assert(reflectionData);

		// Allocate the space for this type.
		void *instanceData = reflectionData->allocateInstance();
		ReflectedVariable variable(reflectionData, static_cast<void *>(instanceData));

		// Reset the stream position before moving on so that the reflection deserialization code can
		// read that info as well.
		stream.seekg(streamPosition);

		// Allow this variable to deserialize itself.
		reflectionData->deserialize(&variable, stream, *this, false);

		// Eat the newline character.
		stream.ignore(256, '\n');
	} 

    for (auto &pointer : m_pointersToPatch) {
        ReflectedVariable *tablePointer = &(m_dataTable[pointer.index].variable);

        // Set the pointer to the proper pointer in the table.
        pointer.variable.value<void *>() = (void *)tablePointer->instanceData();
    }
}

void PointerTable::addPatchPointer(PointerTable::TableIndex index, ReflectedVariable &pointer)
{
    PatchPointer pointerToPatch;
    pointerToPatch.index = index;
    pointerToPatch.variable = pointer;
	m_pointersToPatch.push_back(pointerToPatch);
}

PointerTable::TableIndex PointerTable::addPointer(const ReflectedVariable &pointer, bool needsSerialization)
{
	PointerAddress address = reinterpret_cast<PointerAddress>(pointer.instanceData());
	LookupTable::iterator iter = m_lookupTable.find(address);
	if (iter != m_lookupTable.end()) {
		// An entry for this address already exists. Make sure we have a matching entry for this type.
		const std::string &typeName = pointer.reflectionData()->name();
        for (auto &object : iter->second) {
			if (object.reflectionData->name() == typeName) {
				// This pointer already exists in the table, return its index.
				TableIndex index = object.tableIndex;

				// If this pointer already exists and also wants to be serialized, update its
				// serialization status. There may be the case where the pointer does not need
				// to be serialized anymore as what it points to has already been serialized.
				if (m_dataTable[index].needsSerialization == true) {
					m_dataTable[index].needsSerialization = needsSerialization;
				}

				return index;
			}
		}

		// If we got to this point, then that means that this specific type doesn't exist at this address (for example, the first
		// member variable in a struct). Add it now.
		TableIndex index = m_dataTable.size();
		Instance instance(pointer.reflectionData(), index);
		TableRecord record(pointer, needsSerialization);
		m_dataTable.push_back(record);

		iter->second.push_back(instance);

		return index;
	}

	// We don't yet have an entry for this address. Create one and add it to the table.

	// First create the Objects list to push into the lookup table.
	TableIndex index = m_dataTable.size();
	Instance instance(pointer.reflectionData(), index);
	Objects objects;
	objects.push_back(instance);

	// Push the data onto the table record.
	TableRecord record(pointer, needsSerialization);
	m_dataTable.push_back(record);

	m_lookupTable[address] = objects;
	return index;
}

bool PointerTable::hasPointer(const ReflectedVariable &variable) const
{
	PointerAddress address = reinterpret_cast<PointerAddress>(variable.instanceData());
	LookupTable::const_iterator iter = m_lookupTable.find(address);

	if (iter == m_lookupTable.end()) {
		return false;
	}

	// We have an entry for this address, make sure that we have a type match as well.
	const std::string &typeName = variable.reflectionData()->name();
    for (auto &object : iter->second) {
        if (object.reflectionData->name() == typeName) {
            return true;
        }
    }
	return false;
}

} // namespace carl.