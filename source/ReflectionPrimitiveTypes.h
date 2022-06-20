//
//  ReflectionPrimitiveTypes.h
//  carl
//
//  Created by Cody White on 6/19/22.
//  Copyright (c) 2022 Cody White. All rights reserved.
//

#pragma once

///
/// Definitions for all primitive types supported by the reflection system.
///

#include "../carl.h"
#include "ReflectedVariable.h"

#include <assert.h>
#include <ostream>
#include <istream>

///
/// Macro to declare the reflection data for primitive (POD) types. All reflected primitive types are declared in this file.
///
#define CARL_DECLARE_REFLECTION_PRIMITIVE_TYPE(T) \
    carl::ReflectionDataCreator<carl::QualifierRemover<T>::type> CARL_UNIQUE_NAME( )(#T, sizeof(T)); \
    template<> void carl::ReflectionDataCreator<carl::QualifierRemover<T>::type>::registerReflectionData() \
    { \
        carl::ReflectionDataCreator<carl::QualifierRemover<T>::type>::instance().setSerializeFunction(serializePrimitiveValue<carl::QualifierRemover<T>::type>); \
		carl::ReflectionDataCreator<carl::QualifierRemover<T>::type>::instance().setDeserializeFunction(deserializePrimitiveValue<carl::QualifierRemover<T>::type>); \
    }

namespace carl {

template<class T>
void serializePrimitiveValue(const ReflectedVariable *variable, std::ostream &stream)
{
	stream << variable->value<T>() << std::endl;
}

template<class T>
void deserializePrimitiveValue(ReflectedVariable *variable, std::istream &stream)
{
	stream >> variable->value<T>();
}

/////// Specializations for serializing string types (have to be able to handle multiple words).
template<>
void serializePrimitiveValue<std::string>(const ReflectedVariable *variable, std::ostream &stream)
{
	std::string string = variable->value<std::string>();

	stream << string.length() << " " << string << std::endl;
}

template<>
void deserializePrimitiveValue<std::string>(ReflectedVariable *variable, std::istream &stream)
{
	size_t stringLength = 0;
	stream >> stringLength;

	assert(stringLength > 0);

	// seek ahead one character to avoid reading the space inserted by the serialization function.
	stream.seekg(1, std::ios_base::cur);

	char tmpArray[1024];
	stream.read(tmpArray, stringLength);
	std::string tmp(tmpArray, stringLength);
	variable->value<std::string>() = tmp;
}
////////

// Declare all supported POD reflected types.
CARL_DECLARE_REFLECTION_PRIMITIVE_TYPE(int);
CARL_DECLARE_REFLECTION_PRIMITIVE_TYPE(float);
CARL_DECLARE_REFLECTION_PRIMITIVE_TYPE(double);
CARL_DECLARE_REFLECTION_PRIMITIVE_TYPE(char);
CARL_DECLARE_REFLECTION_PRIMITIVE_TYPE(bool);
CARL_DECLARE_REFLECTION_PRIMITIVE_TYPE(uint16_t);
CARL_DECLARE_REFLECTION_PRIMITIVE_TYPE(uint32_t);
CARL_DECLARE_REFLECTION_PRIMITIVE_TYPE(uint64_t);
//CARL_DECLARE_REFLECTION_PRIMITIVE_TYPE(size_t);
CARL_DECLARE_REFLECTION_PRIMITIVE_TYPE(long);
CARL_DECLARE_REFLECTION_PRIMITIVE_TYPE(long long);
CARL_DECLARE_REFLECTION_PRIMITIVE_TYPE(std::string);

} // namespace carl.