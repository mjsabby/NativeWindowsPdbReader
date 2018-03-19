#pragma once

#include <cstdint>

struct VirtualAddressInfo
{
    uint32_t VirtualAddress;
    uint32_t VirtualSize;

    VirtualAddressInfo() : VirtualAddress(0), VirtualSize(0)
    {
    }

    VirtualAddressInfo(const uint32_t va, const uint32_t size) : VirtualAddress(va), VirtualSize(size)
    {
    }
};