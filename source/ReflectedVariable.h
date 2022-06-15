//
//  ReflectedVariable.h
//  carl
//
//  Created by Cody White on 6/14/22
//  Copyright (c) 2022 Cody White. All rights reserved.
//

#pragma once

///
/// Holds a specific instance of a reflected type along with the actual data of
/// a specific instance of the type.
///

#include <ostream>

namespace carl {

// Forward declarations.
class ReflectionData;

class ReflectedVariable
{
    public:
    
        // Constructors/destructor.
        ReflectedVariable();
        ReflectedVariable(const ReflectedVariable &other);
		ReflectedVariable &operator=(const ReflectedVariable &other);
        ~ReflectedVariable();
    
        ///
        /// Construct this object from any reflected type.
        ///
        template<typename T>
        ReflectedVariable(const T &value);
    
        ///
        /// Construct this object from reflected data and the data of an actual instance of the object.
        ///
        /// @param reflectedData Pointer to the reflected data for the type of instance data passed in.
        /// @param instanceData Data for an actual instance of one of the reflected types.
        ///
        ReflectedVariable(const ReflectionData *reflectionData, void *instanceData);
    
        ///
        /// Get the reflected data contained within this reflected variable.
        ///
        /// @return The reflected data.
        ///
        const ReflectionData *reflectionData() const;
    
        ///
        /// Get the instance data within this reflected variable.
        ///
        /// @return The instance data.
        ///
        const void *instanceData() const;

		///
		/// Set the instance data for this variable.
		///
		/// @param data Instance data to use.
		///
		void setInstanceData(const void *data);
    
        ///
        /// Get the instance data cast to a specific type.
        ///
        template<typename T>
        const T &value() const;
		template<typename T>
		T &value();
    
        ///
        /// Serialize this variable to the specified stream.
        ///
        /// @param stream Output stream to write the serialized data to.
        ///
        void serialize(std::ostream &stream) const;

		///
		/// Deserialize this variable to the specified stream.
		///
		/// @param stream Input stream to read the serialized data from.
		///
		void deserialize(std::istream &stream);
    
    private:
    
        const ReflectionData *m_reflectionData = nullptr; ///< Reflection data for this type.
        void				 *m_instanceData = nullptr;   ///< Data for actual instance of the reflected type.
};


} // namespace carl

#include "ReflectedVariable.inl"
