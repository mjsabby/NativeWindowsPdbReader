#pragma once

#include <cstdint>

struct AddressRange
{
    uint32_t RVA;
    uint32_t IndexIntoStringTable;

    AddressRange() : RVA(0), IndexIntoStringTable(0)
    {
    }

    AddressRange(const uint64_t rva, const uint32_t index) : RVA(rva), IndexIntoStringTable(index)
    {
    }

    bool operator<(const AddressRange &that) const
    {
        return this->RVA < that.RVA;
    }
};