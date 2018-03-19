#include "ManagedSymbolsBuilder.h"

#include "llvm/DebugInfo/CodeView/CVRecord.h"
#include "llvm/DebugInfo/CodeView/SymbolRecord.h"
#include "llvm/DebugInfo/CodeView/LazyRandomTypeCollection.h"
#include <cstdint>

using namespace llvm;
using namespace codeview;
using namespace pdb;

Error ManagedSymbolsBuilder::visitUnknownSymbol(CVSymbol &Record)
{
    const auto kind = Record.kind();

    switch (kind)
    {
    case S_GMANPROC:
    case S_LMANPROC:
    {
        auto &recordData = Record.RecordData;

        uint32_t end;
        {
            char *p = reinterpret_cast<char *>(&end);
            p[0] = recordData[8];
            p[1] = recordData[9];
            p[2] = recordData[10];
            p[3] = recordData[11];
        }

        uint32_t len;
        {
            char *p = reinterpret_cast<char *>(&len);
            p[0] = recordData[16];
            p[1] = recordData[17];
            p[2] = recordData[18];
            p[3] = recordData[19];
        }

        uint32_t token;
        {
            char *p = reinterpret_cast<char *>(&token);

            p[0] = recordData[28];
            p[1] = recordData[29];
            p[2] = recordData[30];
            p[3] = recordData[31];
        }

        uint32_t offset;
        {
            char *p = reinterpret_cast<char *>(&offset);
            p[0] = recordData[32];
            p[1] = recordData[33];
            p[2] = recordData[34];
            p[3] = recordData[35];
        }

        uint16_t segment;
        {
            char *p = reinterpret_cast<char *>(&segment);
            p[0] = recordData[36];
            p[1] = recordData[37];
        }

        this->managedMethods.push_back(ManagedMethodSymbol(end, len, token, offset, segment));

        break;
    }

    default:
        break;
    }

    return Error::success();
}

Error ManagedSymbolsBuilder::visitSymbolBegin(CVSymbol &Record)
{
    return visitSymbolBegin(Record, 0);
}

Error ManagedSymbolsBuilder::visitSymbolBegin(CVSymbol &Record,
                                              uint32_t Offset)
{
    return Error::success();
}

Error ManagedSymbolsBuilder::visitSymbolEnd(CVSymbol &Record)
{
    return Error::success();
}

Error ManagedSymbolsBuilder::visitKnownRecord(CVSymbol &CVR, BlockSym &Block)
{
    return Error::success();
}

Error ManagedSymbolsBuilder::visitKnownRecord(CVSymbol &CVR, Thunk32Sym &Thunk)
{
    return Error::success();
}

Error ManagedSymbolsBuilder::visitKnownRecord(CVSymbol &CVR,
                                              TrampolineSym &Tramp)
{
    return Error::success();
}

Error ManagedSymbolsBuilder::visitKnownRecord(CVSymbol &CVR,
                                              SectionSym &Section)
{
    return Error::success();
}

Error ManagedSymbolsBuilder::visitKnownRecord(CVSymbol &CVR, CoffGroupSym &CG)
{
    return Error::success();
}

Error ManagedSymbolsBuilder::visitKnownRecord(CVSymbol &CVR,
                                              BPRelativeSym &BPRel)
{
    return Error::success();
}

Error ManagedSymbolsBuilder::visitKnownRecord(CVSymbol &CVR,
                                              BuildInfoSym &BuildInfo)
{
    return Error::success();
}

Error ManagedSymbolsBuilder::visitKnownRecord(CVSymbol &CVR,
                                              CallSiteInfoSym &CSI)
{
    return Error::success();
}

Error ManagedSymbolsBuilder::visitKnownRecord(CVSymbol &CVR,
                                              EnvBlockSym &EnvBlock)
{
    return Error::success();
}

Error ManagedSymbolsBuilder::visitKnownRecord(CVSymbol &CVR, FileStaticSym &FS)
{
    return Error::success();
}

Error ManagedSymbolsBuilder::visitKnownRecord(CVSymbol &CVR, ExportSym &Export)
{
    return Error::success();
}

Error ManagedSymbolsBuilder::visitKnownRecord(CVSymbol &CVR,
                                              Compile2Sym &Compile2)
{
    return Error::success();
}

Error ManagedSymbolsBuilder::visitKnownRecord(CVSymbol &CVR,
                                              Compile3Sym &Compile3)
{
    return Error::success();
}

Error ManagedSymbolsBuilder::visitKnownRecord(CVSymbol &CVR,
                                              ConstantSym &Constant)
{
    return Error::success();
}

Error ManagedSymbolsBuilder::visitKnownRecord(CVSymbol &CVR, DataSym &Data)
{
    return Error::success();
}

Error ManagedSymbolsBuilder::visitKnownRecord(
    CVSymbol &CVR, DefRangeFramePointerRelFullScopeSym &Def)
{
    return Error::success();
}

Error ManagedSymbolsBuilder::visitKnownRecord(CVSymbol &CVR,
                                              DefRangeFramePointerRelSym &Def)
{
    return Error::success();
}

Error ManagedSymbolsBuilder::visitKnownRecord(CVSymbol &CVR,
                                              DefRangeRegisterRelSym &Def)
{
    return Error::success();
}

Error ManagedSymbolsBuilder::visitKnownRecord(
    CVSymbol &CVR, DefRangeRegisterSym &DefRangeRegister)
{
    return Error::success();
}

Error ManagedSymbolsBuilder::visitKnownRecord(CVSymbol &CVR,
                                              DefRangeSubfieldRegisterSym &Def)
{
    return Error::success();
}

Error ManagedSymbolsBuilder::visitKnownRecord(CVSymbol &CVR,
                                              DefRangeSubfieldSym &Def)
{
    return Error::success();
}

Error ManagedSymbolsBuilder::visitKnownRecord(CVSymbol &CVR, DefRangeSym &Def)
{
    return Error::success();
}

Error ManagedSymbolsBuilder::visitKnownRecord(CVSymbol &CVR, FrameCookieSym &FC)
{
    return Error::success();
}

Error ManagedSymbolsBuilder::visitKnownRecord(CVSymbol &CVR, FrameProcSym &FP)
{
    return Error::success();
}

Error ManagedSymbolsBuilder::visitKnownRecord(CVSymbol &CVR,
                                              HeapAllocationSiteSym &HAS)
{
    return Error::success();
}

Error ManagedSymbolsBuilder::visitKnownRecord(CVSymbol &CVR, InlineSiteSym &IS)
{
    return Error::success();
}

Error ManagedSymbolsBuilder::visitKnownRecord(CVSymbol &CVR,
                                              RegisterSym &Register)
{
    return Error::success();
}

Error ManagedSymbolsBuilder::visitKnownRecord(CVSymbol &CVR,
                                              PublicSym32 &Public)
{
    return Error::success();
}

Error ManagedSymbolsBuilder::visitKnownRecord(CVSymbol &CVR, ProcRefSym &PR)
{
    return Error::success();
}

Error ManagedSymbolsBuilder::visitKnownRecord(CVSymbol &CVR, LabelSym &Label)
{
    return Error::success();
}

Error ManagedSymbolsBuilder::visitKnownRecord(CVSymbol &CVR, LocalSym &Local)
{
    return Error::success();
}

Error ManagedSymbolsBuilder::visitKnownRecord(CVSymbol &CVR,
                                              ObjNameSym &ObjName)
{
    return Error::success();
}

Error ManagedSymbolsBuilder::visitKnownRecord(CVSymbol &CVR, ProcSym &Proc)
{
    return Error::success();
}

Error ManagedSymbolsBuilder::visitKnownRecord(CVSymbol &CVR,
                                              ScopeEndSym &ScopeEnd)
{
    return Error::success();
}

Error ManagedSymbolsBuilder::visitKnownRecord(CVSymbol &CVR, CallerSym &Caller)
{
    return Error::success();
}

Error ManagedSymbolsBuilder::visitKnownRecord(CVSymbol &CVR,
                                              RegRelativeSym &RegRel)
{
    return Error::success();
}

Error ManagedSymbolsBuilder::visitKnownRecord(CVSymbol &CVR,
                                              ThreadLocalDataSym &Data)
{
    return Error::success();
}

Error ManagedSymbolsBuilder::visitKnownRecord(CVSymbol &CVR, UDTSym &UDT)
{
    return Error::success();
}