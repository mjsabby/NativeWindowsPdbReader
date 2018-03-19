#pragma once

#include "AddressRange.h"
#include "VirtualAddressInfo.h"
#include "llvm/DebugInfo/CodeView/SymbolVisitorCallbacks.h"

namespace llvm
{
namespace pdb
{
class RVASymbolBuilder : public codeview::SymbolVisitorCallbacks
{

  public:
    RVASymbolBuilder(VirtualAddressInfo *sectionVirtualAddresses, AddressRange *addresses, StringRef *addressNames) : currentIndex(0), sectionVirtualAddresses(sectionVirtualAddresses), addresses(addresses), addressNames(addressNames)
    {
    }

    Error visitUnknownSymbol(codeview::CVSymbol &Record) override;
    Error visitSymbolBegin(codeview::CVSymbol &Record) override;
    Error visitSymbolBegin(codeview::CVSymbol &Record, uint32_t Offset) override;
    Error visitSymbolEnd(codeview::CVSymbol &Record) override;

#define SYMBOL_RECORD(EnumName, EnumVal, Name)              \
    virtual Error visitKnownRecord(codeview::CVSymbol &CVR, \
                                   codeview::Name &Record) override;
#define SYMBOL_RECORD_ALIAS(EnumName, EnumVal, Name, AliasName)
#include "llvm/DebugInfo/CodeView/CodeViewSymbols.def"

  private:
    uint32_t currentIndex;
    VirtualAddressInfo *sectionVirtualAddresses;
    AddressRange *addresses;
    StringRef *addressNames;
};
}
}