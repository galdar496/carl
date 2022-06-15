//
//  carl.h
//  carl
//
//  Created by Cody White on 6/14/2022
//  Copyright (c) 2022 Cody White. All rights reserved.
//

#pragma once

#include "source/ReflectionData.h"
#include "source/QualifierRemover.h"

///
/// Specify the macros and classes necessary to generate reflection
/// information.
///

///
/// Declare functions necessary to reflect class 'classType'. This must be placed
/// in the 'public' section of a class otherwise carl will fail to compile.
///
#define CARL_DECLARE_REFLECTED_CLASS(classType) \
    static void addMember(const std::string &name, size_t offset, size_t size, bool isPointer, const carl::ReflectionData *data); \
    static carl::QualifierRemover<classType>::type *nullCast(); \
    static void registerReflectionData();

///
/// Reflect the of class 'classType'. This macro acts like a function 
/// which should be called like this:
/// CARL_REFLECT_CLASS(SomeClassType)
/// {
///     CARL_REFLECT_MEMBER(memberName1);
///     CARL_REFLECT_MEMBER(memberName2);
///     CARL_REFLECT_MEMBER(memberName3);
///     CARL_REFLECT_MEMBER(memberName4);
/// }
///
/// This can ONLY be called on a class that has been declared with reflection data
/// with CARL_DECLARE_REFLECTED_CLASS().
///
#define CARL_REFLECT_CLASS(classType) \
    const carl::ReflectionDataCreator<carl::QualifierRemover<classType>::type> CARL_UNIQUE_NAME( )(#classType, sizeof(classType)); \
    carl::QualifierRemover<classType>::type *classType::nullCast() { return (carl::QualifierRemover<classType>::type *)(nullptr); } \
    void classType::addMember(const std::string &name, size_t offset, size_t size, bool isPointer, const carl::ReflectionData *data) { return carl::ReflectionDataCreator<carl::QualifierRemover<classType>::type>::addMember(name, offset, size, isPointer, data); } \
    template<> void carl::ReflectionDataCreator<carl::QualifierRemover<classType>::type>::registerReflectionData() { classType::registerReflectionData(); } \
    void classType::registerReflectionData()

///
/// Declare a parent type for the type specified by classType. Must be called within the scope
/// of the macro CARL_REFLECT_CLASS.
///
#define CARL_DECLARE_PARENT(classType, parentType) \
	carl::ReflectionDataCreator<carl::QualifierRemover<classType>::type>::DeclareParent(&carl::ReflectionDataCreator<carl::QualifierRemover<parentType>::type>::instance());

///
/// Reflect a specific member of a class. This must be called within
/// the scope of the macro CARL_REFLECT_CLASS.
///
// sizeof(carl::QualifierRemover<std::remove_all_extents<decltype(nullCast()->memberName)>::type >::type)
#define CARL_REFLECT_MEMBER(memberName) \
    addMember(#memberName, (size_t)(&(nullCast()->memberName)), \
              sizeof(nullCast()->memberName), \
			  carl::QualifierRemover<decltype(nullCast()->memberName)>::IsPointer, \
              &(carl::ReflectionDataCreator<carl::QualifierRemover<std::remove_all_extents<decltype(nullCast()->memberName)>::type >::type>::instance()));

///
/// Generate a unique name. Make use of CARL_UNIQUE_NAME, the other macros
/// should not be directly called. Generates a name such as 
/// __carl_Reflected1 etc.
///
#define CARL_APPEND_TOKENS(counter) __carl_Reflected##counter
#define CARL_UNIQUE_NAME_INTERNAL(counter) CARL_APPEND_TOKENS(counter)
#define CARL_UNIQUE_NAME( ) CARL_UNIQUE_NAME_INTERNAL(__COUNTER__)