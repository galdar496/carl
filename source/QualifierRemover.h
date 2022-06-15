//
//  QualifierRemover.h
//  carl
//
//  Created by Cody White on 6/14/2022
//  Copyright (c) 2022 Cody White. All rights reserved.
//

#pragma once

///
/// Remove any qualifiers from a type. For example, take
/// a const int & and return an int. Similar with const float ->
/// float.
///

namespace carl {
    // Strip all qualifiers and get down to just the base type.

    template<class T>
    struct QualifierRemover
    {
		typedef typename std::decay<T>::type type;
		static const bool IsPointer = false;
    };

	template<class T>
    struct QualifierRemover<T *>
    {
        typedef typename QualifierRemover<T>::type type;
		static const bool IsPointer = true;
    };

	template<class T>
	struct QualifierRemover<const T *>
	{
		typedef typename QualifierRemover<T>::type type;
		static const bool IsPointer = true;
	};

} // namespace carl