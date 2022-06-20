//
//  ReflectionUtilities.h
//  carl
//
//  Created by Cody White on 6/19/22.
//  Copyright (c) 2022 Cody White. All rights reserved.
//

#pragma once

///
/// Utility functions to be used by carl.
///

namespace carl {

///
/// Simple pointer manipulation.
///
inline void *pointerOffset(const void *ptr, size_t offset)
{
    return (void *)((char *)ptr + offset);
}

} // namespace carl.