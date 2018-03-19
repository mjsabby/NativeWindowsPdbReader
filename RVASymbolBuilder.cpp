#include "RVASymbolBuilder.h"

#include "llvm/DebugInfo/CodeView/CVRecord.h"
#include "llvm/DebugInfo/CodeView/SymbolRecord.h"
#include "llvm/DebugInfo/CodeView/LazyRandomTypeCollection.h"

using namespace llvm;
using namespace codeview;
using namespace pdb;

Error RVASymbolBuilder::visitUnknownSymbol(codeview::CVSymbol &Record)
{
    return visitUnknownSymbol(Record);
}

Error RVASymbolBuilder::visitSymbolBegin(codeview::CVSymbol &Record)
{
    return visitSymbolBegin(Record, 0);
}

Error RVASymbolBuilder::visitSymbolBegin(codeview::CVSymbol &Record,
                                         uint32_t Offset)
{
    return Error::success();
}

Error RVASymbolBuilder::visitSymbolEnd(CVSymbol &Record)
{
    return Error::success();
}

Error RVASymbolBuilder::visitKnownRecord(CVSymbol &CVR, BlockSym &Block)
{
    return Error::success();
}

Error RVASymbolBuilder::visitKnownRecord(CVSymbol &CVR, Thunk32Sym &Thunk)
{
    return Error::success();
}

Error RVASymbolBuilder::visitKnownRecord(CVSymbol &CVR,
                                         TrampolineSym &Tramp)
{
    return Error::success();
}

Error RVASymbolBuilder::visitKnownRecord(CVSymbol &CVR,
                                         SectionSym &Section)
{
    return Error::success();
}

Error RVASymbolBuilder::visitKnownRecord(CVSymbol &CVR, CoffGroupSym &CG)
{
    return Error::success();
}

Error RVASymbolBuilder::visitKnownRecord(CVSymbol &CVR,
                                         BPRelativeSym &BPRel)
{
    return Error::success();
}

Error RVASymbolBuilder::visitKnownRecord(CVSymbol &CVR,
                                         BuildInfoSym &BuildInfo)
{
    return Error::success();
}

Error RVASymbolBuilder::visitKnownRecord(CVSymbol &CVR,
                                         CallSiteInfoSym &CSI)
{
    return Error::success();
}

Error RVASymbolBuilder::visitKnownRecord(CVSymbol &CVR,
                                         EnvBlockSym &EnvBlock)
{
    return Error::success();
}

Error RVASymbolBuilder::visitKnownRecord(CVSymbol &CVR, FileStaticSym &FS)
{
    return Error::success();
}

Error RVASymbolBuilder::visitKnownRecord(CVSymbol &CVR, ExportSym &Export)
{
    return Error::success();
}

Error RVASymbolBuilder::visitKnownRecord(CVSymbol &CVR,
                                         Compile2Sym &Compile2)
{
    return Error::success();
}

Error RVASymbolBuilder::visitKnownRecord(CVSymbol &CVR,
                                         Compile3Sym &Compile3)
{
    return Error::success();
}

Error RVASymbolBuilder::visitKnownRecord(CVSymbol &CVR,
                                         ConstantSym &Constant)
{
    return Error::success();
}

Error RVASymbolBuilder::visitKnownRecord(CVSymbol &CVR, DataSym &Data)
{
    return Error::success();
}

Error RVASymbolBuilder::visitKnownRecord(
    CVSymbol &CVR, DefRangeFramePointerRelFullScopeSym &Def)
{
    return Error::success();
}

Error RVASymbolBuilder::visitKnownRecord(CVSymbol &CVR,
                                         DefRangeFramePointerRelSym &Def)
{
    return Error::success();
}

Error RVASymbolBuilder::visitKnownRecord(CVSymbol &CVR,
                                         DefRangeRegisterRelSym &Def)
{
    return Error::success();
}

Error RVASymbolBuilder::visitKnownRecord(
    CVSymbol &CVR, DefRangeRegisterSym &DefRangeRegister)
{
    return Error::success();
}

Error RVASymbolBuilder::visitKnownRecord(CVSymbol &CVR,
                                         DefRangeSubfieldRegisterSym &Def)
{
    return Error::success();
}

Error RVASymbolBuilder::visitKnownRecord(CVSymbol &CVR,
                                         DefRangeSubfieldSym &Def)
{
    return Error::success();
}

Error RVASymbolBuilder::visitKnownRecord(CVSymbol &CVR, DefRangeSym &Def)
{
    return Error::success();
}

Error RVASymbolBuilder::visitKnownRecord(CVSymbol &CVR, FrameCookieSym &FC)
{
    return Error::success();
}

Error RVASymbolBuilder::visitKnownRecord(CVSymbol &CVR, FrameProcSym &FP)
{
    return Error::success();
}

Error RVASymbolBuilder::visitKnownRecord(CVSymbol &CVR,
                                         HeapAllocationSiteSym &HAS)
{
    return Error::success();
}

Error RVASymbolBuilder::visitKnownRecord(CVSymbol &CVR, InlineSiteSym &IS)
{
    return Error::success();
}

Error RVASymbolBuilder::visitKnownRecord(CVSymbol &CVR,
                                         RegisterSym &Register)
{
    return Error::success();
}

Error RVASymbolBuilder::visitKnownRecord(CVSymbol &CVR,
                                         PublicSym32 &Public)
{

    const auto rva = this->sectionVirtualAddresses[Public.Segment].VirtualAddress + Public.Offset;
    const auto curr = this->currentIndex;

    this->addresses[curr] = AddressRange(rva, curr);
    this->addressNames[curr] = Public.Name;

    this->currentIndex++;
    return Error::success();
}

Error RVASymbolBuilder::visitKnownRecord(CVSymbol &CVR, ProcRefSym &PR)
{
    return Error::success();
}

Error RVASymbolBuilder::visitKnownRecord(CVSymbol &CVR, LabelSym &Label)
{
    return Error::success();
}

Error RVASymbolBuilder::visitKnownRecord(CVSymbol &CVR, LocalSym &Local)
{
    return Error::success();
}

Error RVASymbolBuilder::visitKnownRecord(CVSymbol &CVR,
                                         ObjNameSym &ObjName)
{
    return Error::success();
}

Error RVASymbolBuilder::visitKnownRecord(CVSymbol &CVR, ProcSym &Proc)
{
    return Error::success();
}

Error RVASymbolBuilder::visitKnownRecord(CVSymbol &CVR,
                                         ScopeEndSym &ScopeEnd)
{
    return Error::success();
}

Error RVASymbolBuilder::visitKnownRecord(CVSymbol &CVR, CallerSym &Caller)
{
    return Error::success();
}

Error RVASymbolBuilder::visitKnownRecord(CVSymbol &CVR,
                                         RegRelativeSym &RegRel)
{
    return Error::success();
}

Error RVASymbolBuilder::visitKnownRecord(CVSymbol &CVR,
                                         ThreadLocalDataSym &Data)
{
    return Error::success();
}

Error RVASymbolBuilder::visitKnownRecord(CVSymbol &CVR, UDTSym &UDT)
{
    return Error::success();
}