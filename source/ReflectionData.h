//
//  ReflectionData.h
//  carl
//
//  Created by Cody White on 6/19/22.
//  Copyright (c) 2022 Cody White. All rights reserved.
//

#pragma once

#include "ReflectionDataManager.h"

#include <ostream>
#include <string>
#include <list>
#include <functional>

///
/// Classes which contain reflected data members and
/// classes.
///

namespace carl {

// Forward declarations.
class ReflectedMember;
class ReflectedVariable;
class PointerTable;

class ReflectionData
{
public:        
    ReflectionData();
    ~ReflectionData();

    ///
    /// Function pointer typedefs.
    ///
    typedef std::function<void *()> AllocateInstanceFunction;
    typedef std::function<void(const ReflectedVariable *variable, std::ostream &stream)> SerializeFunction;
    typedef std::function<void(ReflectedVariable *variable, std::istream &stream)> DeserializeFunction;

    ///
    /// Info struct to use for initializing this object.
    ///
    struct ReflectionDataCInfo
    {
        std::string name; ///< Name of this type.
        size_t      size; ///< Size of this type (in bytes).

        AllocateInstanceFunction allocateFunction; ///< Function to use for allocating an instance of this object.
    };
    
    ///
    /// Initialize this object for use.
    ///
    /// @param info Initialization object to use for setting up the reflection data.
    ///
    void init(ReflectionDataCInfo &info);
    
    ///
    /// Get the name of this type.
    ///
    /// @return Name of the type.
    ///
    inline const std::string &name() const { return m_name; }
    
    ///
    /// Get the size of this type (in bytes).
    ///
    /// @return Size of this type (in bytes).
    ///
    inline size_t size() const { return m_size; }

    ///
    /// Declare the parent type to this type (for inheritance).
    ///
    /// @param parent Parent object to this type.
    ///
    inline void declareParent(const ReflectionData *parent) { m_parent = parent; }

    ///
    /// Does this type have a parent (inheritance) or not?
    ///
    /// @return If true, this type has a parent.
    ///
    inline bool hasParent() const { return (m_parent != nullptr); }
    
    ///
    /// Add a member to this type.
    ///
    /// @param member New member info to add to this type.
    ///
    inline void addMember(const ReflectedMember *member) {
        m_members.push_back(const_cast<ReflectedMember *>(member));
    }
    
    ///
    /// Determine if this type has members (a class/struct) or
    /// doesn't (a POD type).
    ///
    /// @return If true, this type has members.
    ///
    inline bool hasDataMembers() const { return !(m_members.empty()); }

    ///
    /// Find a specific member by name for this object. 
    ///
    /// @param name Name of the member.
    /// @return A pointer to the found member, nullptr if not found.
    /// 
    const ReflectedMember *member(const std::string &name) const;
    
    ///
    /// Storage list for reflected members of this type.
    ///
    using Members = std::list<ReflectedMember *>;

    ///
    /// Get access to the members of this type. If members exist
    /// the first one is returned.
    ///
    /// @return List of members. If the list is empty then there are no members.
    ///
    inline const Members &members() const { return m_members; }

    ///
    /// Allocate an instance the object that this reflection data represents.
    ///
    /// @return A created instance for this type.
    ///
    inline void *allocateInstance() const { return m_allocateInstanceFunction(); }
    
    ///
    /// Serialize the reflected variable to the stream.
    ///
    /// @param variable Reflected variable to serialize.
    /// @param stream Output stream to serialize to.
    /// @param pointerTable Table to write to when coming across pointer types.
    /// @param padding Padding to apply to the output (in terms of tabs).
    ///
    void serialize(const ReflectedVariable *variable, std::ostream &stream, PointerTable &pointerTable, size_t padding = 0, bool isArray = false) const;

    ///
    /// Deserialize the reflected variable from the stream.
    ///
    /// @param variable Reflected variable to deserialize.
    /// @param stream Input stream to serialize from.
    /// @param pointerTable Table to read from when coming across pointer types.
    /// @param isArray If true, we're currently deserializing elements of an array (don't try to read the pointer index as there isn't one per array element).
    ///
    void deserialize(ReflectedVariable *variable, std::istream &stream, PointerTable &pointerTable, bool isArray = false) const;
    
    ///
    /// Set the serialization function. Some types (such as the primitive types defined in ReflectionPrimitiveTypes.h) know
    /// how to serialize themselves. This provides a function callback to use for serialization of known types.
    ///
    /// @param function Function to use for serialization of this type.
    ///
    inline void setSerializeFunction(SerializeFunction function = nullptr) { m_serializeFunction = function; }

    ///
    /// Set the deserialization function. Some types (such as the primitive types defined in ReflectionPrimitiveTypes.h) know
    /// how to deserialize themselves. This provides a function callback to use for deserialization of known types.
    ///
    /// @param function Function to use for deserialization of this type.
    ///
    inline void setDeserializeFunction(DeserializeFunction function = nullptr) { m_deserializeFunction = function; }
        
    private:
        
    Members                m_members;    ///< Members contained in this type.
    std::string            m_name;       ///< Name of this type.
    size_t                 m_size = 0;       ///< Size of this type in bytes.
    const ReflectionData  *m_parent = nullptr;     ///< Parent object to this type (only populated if this is an inherited type).
    
    SerializeFunction   m_serializeFunction = nullptr;   ///< Serialization function to use if this type is a primitive type defined in ReflectionPrimitiveTypes.h
    DeserializeFunction m_deserializeFunction = nullptr; ///< Deserialization function to use if this type is a primitive type defined in ReflectionPrimitiveTypes.h

    AllocateInstanceFunction m_allocateInstanceFunction = nullptr; ///< Function to use to allocate an instance of this type (returns a void *).
};

class ReflectedMember
{
public:

    ///
    /// Default constructor.
    ///
    /// @param name Name of the variable.
    /// @param offset Offset (in bytes) from the beginning of the class data.
    /// @param size Size of this member (in bytes). In the case of pointers, this size is the size of the base type of the pointer, not the pointer itself.
    /// @param isPointer If true, this variable is a pointer.
    /// @param reflectionData Reflected data for this member variable. Could be a POD type
    ///                       or another class/struct.
    ///
    explicit ReflectedMember(const std::string &name, size_t offset, size_t size, bool isPointer, const ReflectionData *reflectionData);

    ~ReflectedMember();
    
    ///
    /// Get the name of the variable.
    ///
    /// @return Name of the member variable.
    ///
    inline const std::string& name() const { return m_name; }

    ///
    /// Get the offset of the variable relative to the beginning
    /// of the class data (in bytes).
    ///
    /// @return Offset of the variable.
    ///
    inline size_t offset() const { return m_offset; }
    
    ///
    /// Get the reflection data for this member variable.
    ///
    /// @return Reflection data for this variable.
    ///
    inline const ReflectionData *reflectionData() const { return m_data; }

    ///
    /// Get the size of this member variable (in bytes). If this member
    /// variable is an array, the size will be the size of the entire
    /// array.
    ///
    /// @return Size of this member variable (in bytes).
    ///
    inline size_t size() const { return m_size; }

    ///
    /// Check to see if this member variable represents an array. If so, each element of the array will be
    /// serialized independently.
    ///
    /// @return If true, this member variable is an array type.
    ///
    bool isArray() const;

    ///
    /// Check to see if this member variable represents pointer. If so, then serialization of this
    /// variable will happen to a separate table.
    ///
    /// @return If true, this member variable is a pointer type.
    ///
    inline bool isPointer() const { return m_isPointer; }

private:

    const std::string     m_name;       ///< Name of this variable.
    size_t                m_offset = 0;     ///< Offset (in bytes) from the start of the class for this variable.
    size_t                m_size = 0;       ///< Size of this variable (in bytes).
    bool                  m_isPointer = false;  ///< If true, this member variable is a pointer to an instance of some other type.
    const ReflectionData *m_data = nullptr;       ///< Reflected data for this variable.
};

template<class T>
class ReflectionDataCreator
{
public:

    ///
    /// Create a new instance of reflection data.
    ///
    /// @param name Name of the type.
    /// @param size Size of the type (in bytes).
    ///
    ReflectionDataCreator(const std::string &name, size_t size)
    {
        init(name, size);
    }

    ///
    /// This class is static per-type.
    ///
    static ReflectionData &instance()
    {
        static ReflectionData instance;
        return instance;
    }
    
    ///
    /// Initialize this type.
    ///
    /// @param name Name of the type.
    /// @param size Size of the type in bytes.
    ///
    static void init(const std::string &name, size_t size)
    {
        ReflectionData &data = instance();

        ReflectionData::ReflectionDataCInfo info;
        info.name = name;
        info.size = size;
        info.allocateFunction = std::bind(allocateInstance);

        // Initialize this reflection data.
        data.init(info);
        
        registerReflectionData();
        ReflectionDataManager::instance().addReflectedData(&data);
    }

    ///
    /// Add a member to this reflected type.
    ///
    /// @param name Name of the member.
    /// @param offset Offset of the member from the beginning of the class (in bytes).
    /// @param size Size of this member (in bytes). In the case of pointers, this size is the size of the base type of the pointer, not the pointer itself.
    /// @param isPointer If true, this member is a pointer to a different instance of some type.
    /// @param data Reflected data for this member.
    ///
    static void addMember(const std::string &name, size_t offset, size_t size, bool isPointer, const ReflectionData *data)
    {
        instance().addMember(new ReflectedMember(name, offset, size, isPointer, data));
    }

    ///
    /// Declare the parent type to this type (for inheritance).
    ///
    /// @param parent Parent object to this type.
    ///
    static void declareParent(const ReflectionData *parent)
    {
        instance().declareParent(parent);
    }
    
    ///
    /// Perform a null cast which is specific to this type.
    ///
    static T *nullCast()
    {
        return reinterpret_cast<T *>(nullptr);
    }

    ///
    /// Automatically implemented via macros defined in Reflection.h per type.
    /// This function is used to populate the reflected member variables per-class.
    ///
    static void registerReflectionData();

    ///
    /// Allocate an instance of this type.
    ///
    /// @return A new instance of this type to use.
    ///
    static void *allocateInstance()
    {
        T *instance = new T;
        return static_cast<void *>(instance);
    }
};

} // namespace carl