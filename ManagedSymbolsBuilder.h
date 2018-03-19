#pragma once

#include "llvm/DebugInfo/CodeView/SymbolVisitorCallbacks.h"
#include <cstdint>
#include <vector>

struct ManagedMethodSymbol
{
    uint32_t End;
    uint32_t Len;
    uint32_t Token;
    uint32_t Offset;
    uint16_t Segment;

    ManagedMethodSymbol(const uint32_t end, const uint32_t len, const uint32_t token, const uint32_t offset, const uint16_t segment) : End(end), Len(len), Token(token), Offset(offset), Segment(segment)
    {
    }
};

namespace llvm
{
namespace pdb
{
class ManagedSymbolsBuilder : public codeview::SymbolVisitorCallbacks
{

  public:
    Error visitUnknownSymbol(codeview::CVSymbol &Record) override;
    Error visitSymbolBegin(codeview::CVSymbol &Record) override;
    Error visitSymbolBegin(codeview::CVSymbol &Record, uint32_t Offset) override;
    Error visitSymbolEnd(codeview::CVSymbol &Record) override;

#define SYMBOL_RECORD(EnumName, EnumVal, Name)              \
    virtual Error visitKnownRecord(codeview::CVSymbol &CVR, \
                                   codeview::Name &Record) override;
#define SYMBOL_RECORD_ALIAS(EnumName, EnumVal, Name, AliasName)
#include "llvm/DebugInfo/CodeView/CodeViewSymbols.def"

    size_t GetSymbolCount() const
    {
        return this->managedMethods.size();
    }

    ManagedMethodSymbol &GetSymbolAt(const int index)
    {
        return this->managedMethods[index];
    }

  private:
    std::vector<ManagedMethodSymbol> managedMethods;
};
}
}