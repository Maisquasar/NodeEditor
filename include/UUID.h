﻿#pragma once
#include <iostream>
#define UUID_NULL -1ull

using TemplateID = uint64_t;

class UUID
{
public:
    UUID();
    UUID(uint64_t uuid);
    UUID& operator=(const UUID& other) = default;
    UUID(const UUID&) = default;
    UUID(UUID&&) noexcept = default;
    virtual ~UUID();

    operator uint64_t() const { return m_uuid; }

private:
    uint64_t m_uuid;
};

namespace std
{
    template <>
    struct hash<UUID>
    {
        size_t operator()(const UUID& uuid) const noexcept
        {
            return hash<uint64_t>()(uuid);
        }
    };
}
