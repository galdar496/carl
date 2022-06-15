//
//  ReflectionData.inl
//  carl
//
//  Created by Cody White on 6/14/2022
//  Copyright (c) 2022 Cody White. All rights reserved.
//

#pragma once

#include "../carl.h"

#define CARL_GET_REFLECTED_TYPE(type) &(carl::ReflectionDataCreator<typename carl::QualifierRemover<type>::type>::instance())

namespace carl {

template <typename T>
ReflectedVariable::ReflectedVariable(const T &value) :
    m_instanceData((void *)(&value))
{
    m_reflectionData = &(carl::ReflectionDataCreator<typename carl::QualifierRemover<T>::type>::instance());
}

template <typename T>
const T &ReflectedVariable::value() const
{		
	return *reinterpret_cast<const T *>(m_instanceData);
}

template <typename T>
T &ReflectedVariable::value()
{
	return *reinterpret_cast<T *>(m_instanceData);
}

} // namespace carll