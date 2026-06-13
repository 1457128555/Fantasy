#pragma once

#define FANTASY_NON_COPYABLE(Type)         \
    Type(const Type&)            = delete; \
    Type& operator=(const Type&) = delete

#define FANTASY_MOVABLE_ONLY(Type)             \
    Type(const Type&)            = delete;     \
    Type& operator=(const Type&) = delete;     \
    Type(Type&&)                 = default;    \
    Type& operator=(Type&&)      = default

