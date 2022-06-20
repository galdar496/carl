//
//  ReflectionDataManager.h
//  carl
//
//  Created by Cody White on 6/19/22.
//  Copyright (c) 2022 Cody White. All rights reserved.
//

#pragma once

///
/// Hold all defitions of reflected types for later retrieval.
///

#include <unordered_map>
#include <string>
#include <vector>

namespace carl {

// Forward declarations.
class ReflectionData;

class ReflectionDataManager
{
public:
    ///
    /// This class is static and can only be accessed through this instance.
    ///
    static ReflectionDataManager &instance();

    ///
    /// Add a reflected type to the manager.
    ///
    /// @param data Data for the reflected type.
    ///
    void addReflectedData(const ReflectionData *data);

    ///
    /// Get a reflected type based on the type name.
    ///
    /// @param name Name of the reflected type to lookup. Type must be declared with QI_DECLARE_REFLECTION_CLASS or QI_DECLARE_REFLECTION_POD.
    /// @return The reflection data, or nullptr if this type was not found.
    ///
    const ReflectionData *reflectionData(const std::string &name);
    
    ///
    /// Same as 'GetReflectionData' except the hashed name id can be used directly.
    ///
    /// @param hashedName The result of hashing the name of a type after calling Qi::StringHash().
    /// @return The reflection data, or nullptr if this type was not found.
    ///
    const ReflectionData *reflectionData(size_t hashedName);

    ///
    /// Get all type names stored in the reflection system.
    ///
    /// @param typenames List of typenames to populate for the calling function.
    ///
    using Typenames = std::vector<std::string>;
    void allTypenames(Typenames &typenames) const;

private:
    // This class is a singleton and cannot be copied.
    ReflectionDataManager() = default;
    ~ReflectionDataManager() = default;
    ReflectionDataManager(const ReflectionDataManager &other) = delete;
    ReflectionDataManager &operator=(const ReflectionDataManager &other) = delete;

    using ReflectionTable = std::unordered_map<uint32_t, const ReflectionData *>;
    ReflectionTable m_reflectedData; ///< All reflected objects stored by a string key. Use Qi::StringHash() to generate a key for to classname to lookup.
};

} // namespace carl