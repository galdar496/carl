//
//  ReflectionDataManager.cpp
//  carl
//
//  Created by Cody White on 6/19/22.
//  Copyright (c) 2022 Cody White. All rights reserved.
//

#include "ReflectionDataManager.h"
#include "ReflectionPrimitiveTypes.h"

#include <assert.h>
#include <functional>

namespace carl {

ReflectionDataManager &ReflectionDataManager::instance()
{
    static ReflectionDataManager manager;
    return manager;
}

size_t hashString(const std::string &s)
{
    static std::hash<std::string> hasher;
    return hasher(s);
}

void ReflectionDataManager::addReflectedData(const ReflectionData *data)
{
    assert(data != nullptr);

    size_t hash = hashString(data->name());    
    assert(m_reflectedData.find(hash) == m_reflectedData.end());
    
    m_reflectedData[hash] = data;
}

const ReflectionData *ReflectionDataManager::reflectionData(const std::string &name)
{
    size_t hash = hashString(name);
    return reflectionData(hash);
}

const ReflectionData *ReflectionDataManager::reflectionData(size_t hashedName)
{
    ReflectionTable::const_iterator iter = m_reflectedData.find(hashedName);
    if (iter != m_reflectedData.end()) {
        return iter->second;
    }
    
    return nullptr;
}

void ReflectionDataManager::allTypenames(Typenames &typenames) const
{
	assert(!m_reflectedData.empty());

	typenames.resize(m_reflectedData.size());

	// Add all typenames to the list by simply iterating over them.
	int index = 0;
    for (auto &reflectedData : m_reflectedData) {
        typenames[index++] = reflectedData.second->name();
    }
}

} // namespace carl