#include "RVASymbolBuilder.h"
#include "InputFile.h"

#include "llvm/Object/COFF.h"
#include "llvm/DebugInfo/CodeView/SymbolVisitorCallbackPipeline.h"
#include "llvm/DebugInfo/CodeView/SymbolDeserializer.h"
#include "llvm/DebugInfo/PDB/Native/NativeSession.h"
#include "llvm/DebugInfo/PDB/Native/PDBFile.h"
#include "llvm/DebugInfo/PDB/Native/InfoStream.h"
#include "llvm/DebugInfo/PDB/Native/GlobalsStream.h"
#include "llvm/DebugInfo/PDB/Native/DbiStream.h"
#include "llvm/DebugInfo/PDB/Native/PublicsStream.h"
#include "llvm/DebugInfo/PDB/Native/SymbolStream.h"
#include "llvm/DebugInfo/CodeView/CVSymbolVisitor.h"
#include "llvm/DebugInfo/CodeView/DebugLinesSubsection.h"
#include "ManagedSymbolsBuilder.h"

using namespace llvm;
using namespace codeview;
using namespace pdb;

typedef void *(*mallocPtr)(size_t);

bool GetLineInformation(InputFile &input, const uint32_t methodToken, const uint32_t ilOffset, const mallocPtr allocator, char **out, uint32_t *out_size, uint32_t *out_lineStart, uint32_t *out_lineEnd, uint16_t *out_columnStart, uint16_t *out_columnEnd)
{
    uint32_t lastModi = UINT32_MAX;
    uint32_t lastNameIndex = UINT32_MAX;

    uint32_t moduleIndex = 0;

    for (const auto &sg : input.symbol_groups())
    {
        auto &moduleStream = sg.getPdbModuleStream();
        const auto symbolSubStream = moduleStream.getSymbolsSubstream();

        SymbolVisitorCallbackPipeline pipeline;
        SymbolDeserializer deserializer(nullptr, CodeViewContainer::Pdb);
        ManagedSymbolsBuilder builder;

        pipeline.addCallbackToPipeline(deserializer);
        pipeline.addCallbackToPipeline(builder);
        CVSymbolVisitor visitor(pipeline);

        if (auto error = visitor.visitSymbolStream(moduleStream.getSymbolArray(), symbolSubStream.Offset))
        {
            continue;
        }

        uint16_t searchSegment = 0;
        uint32_t searchBegin = 0;
        uint32_t searchEnd = 0;

        bool found = false;

        const auto symbolCount = builder.GetSymbolCount();
        for (unsigned int i = 0; i < symbolCount; ++i)
        {
            auto &s = builder.GetSymbolAt(i);
            if (s.Token == methodToken)
            {
                searchBegin = s.Offset;
                searchEnd = s.Len + s.Offset;
                searchSegment = s.Segment;
                found = true;
                break;
            }
        }

        if (!found)
        {
            continue;
        }

        for (const auto &ss : sg.getDebugSubsections())
        {
            if (ss.kind() != DebugSubsectionKind::Lines)
            {
                continue;
            }

            DebugLinesSubsectionRef lines;
            const BinaryStreamReader reader(ss.getRecordData());
            if (auto EC = lines.initialize(reader))
            {
                continue;
            }

            const uint16_t segment = lines.header()->RelocSegment;
            const uint32_t begin = lines.header()->RelocOffset;
            const uint32_t end = begin + lines.header()->CodeSize;

            if (searchBegin == begin && searchEnd == end && searchSegment == segment)
            {
                for (const auto &block : lines)
                {
                    if (lastModi != moduleIndex || lastNameIndex != block.NameIndex)
                    {
                        lastModi = moduleIndex;
                        lastNameIndex = block.NameIndex;
                    }

                    LineNumberEntry searchILOffset;
                    searchILOffset.Offset = ilOffset;

                    const auto iter = upper_bound(block.LineNumbers.begin(), block.LineNumbers.end(), searchILOffset, [](const LineNumberEntry &lhs, const LineNumberEntry &rhs) -> bool { return lhs.Offset < rhs.Offset; });
                    if (iter == block.LineNumbers.end())
                    {
                        if (iter->Offset != ilOffset)
                        {
                            continue; // not found
                        }
                    }

                    const uint32_t index = std::distance(block.LineNumbers.begin(), iter) - 1;
                    if (index == -1)
                    {
                        continue; // not found
                    }

                    LineInfo line(block.LineNumbers[index].Flags);

                    uint16_t startColumn = 0;
                    uint16_t endColumn = 0;
                    if (index < block.Columns.size())
                    {
                        startColumn = block.Columns[index].StartColumn;
                        endColumn = block.Columns[index].EndColumn;
                    }

                    *out_lineStart = line.getStartLine();
                    *out_lineEnd = line.getEndLine();
                    *out_columnStart = startColumn;
                    *out_columnEnd = endColumn;

                    auto stringRef = sg.getFileNameChecksumsOffset(block.NameIndex);

                    const auto size = stringRef.size();
                    *out = static_cast<char *>(allocator(size));
                    *out_size = size;

                    memcpy(*out, stringRef.data(), size);

                    return true;
                }
            }
        }

        ++moduleIndex;
    }

    return false;
}

struct OMap
{
    uint32_t OptimizedImageRVA;
    uint32_t SourceImageRVA;

    OMap() : OptimizedImageRVA(0), SourceImageRVA(0)
    {
    }

    OMap(const uint32_t imageRVA, const uint32_t sourceRVA)
    {
        this->OptimizedImageRVA = imageRVA;
        this->SourceImageRVA = sourceRVA;
    }

    bool operator<(const OMap &that) const
    {
        return this->OptimizedImageRVA < that.OptimizedImageRVA;
    }
};

bool CopyOMapData(std::vector<OMap> &copyToBuffer, const PDBFile &pdbFile, const uint32_t streamIdx)
{
    const uint32_t pdbBlockSize = pdbFile.getBlockSize();
    const auto blocks = pdbFile.getStreamBlockList(streamIdx);
    const auto numBlocks = blocks.size();

    const auto omapSize = numBlocks * pdbBlockSize / sizeof(OMap);
    copyToBuffer.reserve(omapSize);

    for (uint32_t i = 0; i < numBlocks; ++i)
    {
        auto data = pdbFile.getBlockData(blocks[i], pdbBlockSize);
        if (data)
        {
            auto &d = data.get();
            for (uint32_t j = 0; j < d.size(); j += 8)
            {
                uint32_t imageRVA;
                char *p = reinterpret_cast<char *>(&imageRVA);

                p[0] = d[j];
                p[1] = d[j + 1];
                p[2] = d[j + 2];
                p[3] = d[j + 3];

                uint32_t sourceRVA;
                char *q = reinterpret_cast<char *>(&sourceRVA);

                q[0] = d[j + 4];
                q[1] = d[j + 5];
                q[2] = d[j + 6];
                q[3] = d[j + 7];

                copyToBuffer.push_back(OMap(imageRVA, sourceRVA));
            }
        }
        else
        {
            return false;
        }
    }

    copyToBuffer.resize(pdbFile.getStreamByteSize(streamIdx) / sizeof(OMap));

    return true;
}

bool CopyStreamToBuffer(std::vector<uint8_t> &copyToBuffer, const PDBFile &pdbFile, const uint32_t streamIdx)
{
    const uint32_t pdbBlockSize = pdbFile.getBlockSize();
    const auto &blocks = pdbFile.getStreamBlockList(streamIdx);
    const auto numBlocks = blocks.size();

    copyToBuffer.reserve(numBlocks * pdbBlockSize);

    for (uint32_t i = 0; i < numBlocks; ++i)
    {
        auto data = pdbFile.getBlockData(blocks[i], pdbBlockSize);
        if (data)
        {
            auto &d = data.get();
            for (uint32_t j = 0; j < d.size(); ++j)
            {
                copyToBuffer.push_back(d[j]);
            }
        }
        else
        {
            return false;
        }
    }

    return true;
}

class IPdbSymbolReader
{
  public:
    IPdbSymbolReader(const char *pdbFileName) : srcSrvSize(0), sourceLinkSize(0), isOmapValid(false), inputFile(InputFile::open(pdbFileName)), signature({0}), age(0), valid(true), rvaMapBuilt(false)
    {
        if (this->inputFile && this->inputFile->isPdb())
        {
            auto &pdbFile = this->inputFile->pdb();

            if (pdbFile.hasPDBInfoStream() && pdbFile.hasPDBDbiStream() && pdbFile.hasPDBPublicsStream() && pdbFile.hasPDBSymbolStream())
            {
                auto &pdbInfoStream = cantFail(pdbFile.getPDBInfoStream());

                this->signature = pdbInfoStream.getGuid();
                this->age = pdbInfoStream.getAge();

                // setup named streams
                for (auto &nse : pdbInfoStream.named_streams())
                {
                    if (nse.second != kInvalidStreamIndex)
                    {
                        this->namedStreams[nse.second] = nse.first();
                    }
                }

                // populate srcsrv & sourcelink
                {
                    const auto streamCount = pdbFile.getNumStreams();

                    for (uint32_t streamIdx = 0; streamIdx < streamCount; ++streamIdx)
                    {
                        const auto streamName = this->namedStreams[streamIdx];

                        if (streamName.compare("srcsrv") == 0)
                        {
                            if (CopyStreamToBuffer(this->srcsrv, pdbFile, streamIdx))
                            {
                                this->srcSrvSize = pdbFile.getStreamByteSize(streamIdx);
                            }
                        }

                        if (streamName.compare("sourcelink") == 0)
                        {
                            if (CopyStreamToBuffer(this->sourcelink, pdbFile, streamIdx))
                            {
                                this->sourceLinkSize = pdbFile.getStreamByteSize(streamIdx);
                            }
                        }
                    }
                }

                auto dbi = pdbFile.getPDBDbiStream();

                const auto omapToSrc = dbi->getDebugStreamIndex(DbgHeaderType::OmapToSrc);
                if (omapToSrc != kInvalidStreamIndex)
                {
                    this->isOmapValid = CopyOMapData(this->omap, pdbFile, omapToSrc);
                }

                const auto si = dbi->getDebugStreamIndex(this->isOmapValid ? DbgHeaderType::SectionHdrOrig : DbgHeaderType::SectionHdr);
                if (si != kInvalidStreamIndex)
                {
                    auto stream = pdbFile.createIndexedStream(si);
                    if (stream)
                    {
                        ArrayRef<object::coff_section> headers;
                        if (!(stream->getLength() % sizeof(object::coff_section) != 0))
                        {
                            const uint32_t numHeaders = stream->getLength() / sizeof(object::coff_section);
                            BinaryStreamReader reader(*stream);
                            reader.readArray(headers, numHeaders);

                            this->sectionVirtualAddresses.push_back(VirtualAddressInfo(0, 0));

                            for (const auto &header : headers)
                            {
                                this->sectionVirtualAddresses.push_back(VirtualAddressInfo(header.VirtualAddress, header.VirtualSize));
                            }
                        }
                        else
                        {
                            this->valid = false;
                        }
                    }
                    else
                    {
                        this->valid = false;
                    }
                }
                else
                {
                    this->sectionVirtualAddresses.push_back(VirtualAddressInfo(0, 0));

                    uint32_t previousSectionVA = 0x0;
                    uint32_t previousSectionSize = 0x0;
                    const uint32_t alignment = 0x1000;

                    auto secMapArr = dbi->getSectionMap();
                    for (const auto &secMapEntry : secMapArr)
                    {
                        const auto currentSectionVA = (previousSectionVA + previousSectionSize) - ((previousSectionVA + previousSectionSize) % alignment) + alignment;
                        this->sectionVirtualAddresses.push_back(VirtualAddressInfo(currentSectionVA, secMapEntry.SecByteLength));
                        previousSectionVA = currentSectionVA;
                        previousSectionSize = secMapEntry.SecByteLength;
                    }
                }
            }
            else
            {
                this->valid = false;
            }
        }
        else
        {
            this->valid = false;
        }
    }

    bool IsValid() const
    {
        return this->valid;
    }

    LLVM_ATTRIBUTE_NOINLINE bool FindNameForRVA(const uint32_t rva, const mallocPtr allocator, char **out, uint32_t *out_size)
    {
        *out = nullptr;
        *out_size = 0;

        if (!this->rvaMapBuilt)
        {
            this->BuildRvaMap();
        }

        // OMAPT is available so we MUST use that.
        if (this->isOmapValid)
        {
            return this->omapBasedLookup(rva, allocator, out, out_size);
        }

        // the 99% case for all other PDBs
        return this->rvaLookup(rva, allocator, out, out_size);
    }

    LLVM_ATTRIBUTE_NOINLINE bool FindLineNumberForIL(const uint32_t methodToken, const uint32_t iloffset, const mallocPtr allocator, char **out, uint32_t *out_size, uint32_t *out_lineStart, uint32_t *out_lineEnd, uint16_t *out_columnStart, uint16_t *out_columnEnd)
    {
        *out = nullptr;
        *out_size = 0;
        *out_lineStart = 0;
        *out_lineEnd = 0;
        *out_columnStart = 0;
        *out_columnStart = 0;

        return GetLineInformation(this->inputFile.get(), methodToken, iloffset, allocator, out, out_size, out_lineStart, out_lineEnd, out_columnStart, out_columnEnd);
    }

    LLVM_ATTRIBUTE_NOINLINE bool GetSrcSrvData(uint8_t **out, uint32_t *out_size)
    {
        *out = nullptr;
        *out_size = 0;

        if (this->srcSrvSize > 0)
        {
            *out = this->srcsrv.data();
            *out_size = this->srcSrvSize;

            return true;
        }

        return false;
    }

    LLVM_ATTRIBUTE_NOINLINE bool GetSourceLinkData(uint8_t **out, uint32_t *out_size)
    {
        *out = nullptr;
        *out_size = 0;

        if (this->sourceLinkSize > 0)
        {
            *out = this->sourcelink.data();
            *out_size = this->sourceLinkSize;

            return true;
        }

        return false;
    }

    LLVM_ATTRIBUTE_NOINLINE void GetSignature(GUID *incomingGuid, uint32_t *incomingAge) const
    {
        *incomingGuid = this->signature;
        *incomingAge = this->age;
    }

  private:
    DenseMap<uint16_t, std::string> namedStreams;

    uint32_t srcSrvSize;
    std::vector<uint8_t> srcsrv;

    uint32_t sourceLinkSize;
    std::vector<uint8_t> sourcelink;

    bool isOmapValid;
    std::vector<OMap> omap;

    std::vector<VirtualAddressInfo> sectionVirtualAddresses;

    std::vector<AddressRange> addresses;
    std::vector<StringRef> addressNames;

    Expected<InputFile> inputFile;

    GUID signature;
    uint32_t age;

    bool valid;
    bool rvaMapBuilt;

    LLVM_ATTRIBUTE_NOINLINE bool omapBasedLookup(const uint32_t incomingImageRVA, const mallocPtr allocator, char **out, uint32_t *out_size)
    {
        const auto iter = upper_bound(this->omap.begin(), this->omap.end(), OMap(incomingImageRVA, 0));
        if (iter == this->omap.end())
        {
            if (iter->OptimizedImageRVA != incomingImageRVA)
            {
                return false;
            }
        }

        const auto index = distance(this->omap.begin(), iter) - 1;
        if (index == -1)
        {
            return false;
        }

        return this->rvaLookup(this->omap[index].SourceImageRVA, allocator, out, out_size);
    }

    LLVM_ATTRIBUTE_NOINLINE bool rvaLookup(const uint32_t incomingRVA, const mallocPtr allocator, char **out, uint32_t *out_size)
    {
        const auto iter = upper_bound(this->addresses.begin(), this->addresses.end(), AddressRange(incomingRVA, 0));
        if (iter == this->addresses.end())
        {
            if (iter->RVA != incomingRVA)
            {
                return false;
            }
        }

        const auto index = distance(this->addresses.begin(), iter) - 1;
        if (index == -1)
        {
            return false;
        }

        auto &stringRef = this->addressNames[this->addresses[index].IndexIntoStringTable];

        const auto size = stringRef.size();
        *out = static_cast<char *>(allocator(size));
        *out_size = size;

        memcpy(*out, stringRef.data(), size);

        return true;
    }

    LLVM_ATTRIBUTE_NOINLINE void BuildRvaMap()
    {
        auto &pdbFile = this->inputFile->pdb();
        auto publics = pdbFile.getPDBPublicsStream();

        const GSIHashTable &publicsTable = publics->getPublicsTable();
        const auto publicSymbolCount = publicsTable.HashHdr->HrSize / sizeof(PSHashRecord) + this->sectionVirtualAddresses.size(); // + VA size is because we want to guard RVAs by bounds of each VA

        this->addresses.resize(publicSymbolCount);
        this->addressNames.resize(publicSymbolCount);

        SymbolVisitorCallbackPipeline pipeline;
        SymbolDeserializer deserializer(nullptr, CodeViewContainer::Pdb);
        RVASymbolBuilder rvaBuilder(this->sectionVirtualAddresses.data(), addresses.data(), addressNames.data());

        pipeline.addCallbackToPipeline(deserializer);
        pipeline.addCallbackToPipeline(rvaBuilder);

        CVSymbolVisitor visitor(pipeline);

        const auto symStream = pdbFile.getPDBSymbolStream()->getSymbolArray().getUnderlyingStream();

        for (auto pubSymOff : publicsTable)
        {
            auto sym = readSymbolFromStream(symStream, pubSymOff);

            if (!sym)
            {
                break;
            }

            if (auto E = visitor.visitSymbolRecord(*sym, pubSymOff))
            {
                break;
            }
        }

        const auto sectionCount = this->sectionVirtualAddresses.size();
        for (uint32_t i = 0; i < sectionCount; ++i)
        {
            const auto index = publicSymbolCount - sectionCount + i;
            const auto virtualAddressInfo = this->sectionVirtualAddresses[i];
            this->addresses[index] = AddressRange(virtualAddressInfo.VirtualAddress + virtualAddressInfo.VirtualSize, index);
            this->addressNames[index] = StringRef("End of VA");
        }

        sort(this->addresses.begin(), this->addresses.end());

        this->rvaMapBuilt = true;
    }
};

extern "C" bool GetPdbSignatureAndAge(const char *pdbFileName, GUID *incomingSignature, uint32_t *incomingAge)
{
    auto file = InputFile::open(pdbFileName);
    if (file && file->isPdb())
    {
        auto &pdbFile = file->pdb();
        if (pdbFile.hasPDBInfoStream())
        {
            auto &pdbInfoStream = cantFail(pdbFile.getPDBInfoStream());

            *incomingSignature = pdbInfoStream.getGuid();
            *incomingAge = pdbInfoStream.getAge();

            return true;
        }
    }

    return false;
}

extern "C" IPdbSymbolReader *CreatePdbSymbolReader(const char *pdbFileName)
{
    return new IPdbSymbolReader(pdbFileName);
}

extern "C" void DeletePdbSymbolReader(IPdbSymbolReader *reader)
{
    if (reader != nullptr)
    {
        delete reader;
    }
}

extern "C" bool IsReaderValid(IPdbSymbolReader *reader)
{
    if (reader != nullptr)
    {
        return reader->IsValid();
    }

    return false;
}

extern "C" bool GetSignature(IPdbSymbolReader *reader, GUID *signature, uint32_t *age)
{
    if (reader != nullptr && reader->IsValid())
    {
        reader->GetSignature(signature, age);
        return true;
    }

    return false;
}

extern "C" bool FindNameForRVA(IPdbSymbolReader *reader, const uint32_t rva, const mallocPtr allocator, char **out, uint32_t *out_size)
{
    if (reader != nullptr && reader->IsValid())
    {
        return reader->FindNameForRVA(rva, allocator, out, out_size);
    }

    return false;
}

extern "C" bool FindLineNumberForManagedMethod(IPdbSymbolReader *reader, const uint32_t methodToken, const uint32_t iloffset, const mallocPtr allocator, char **out, uint32_t *out_size, uint32_t *out_lineStart, uint32_t *out_lineEnd, uint16_t *out_columnStart, uint16_t *out_columnEnd)
{
    if (reader != nullptr && reader->IsValid())
    {
        return reader->FindLineNumberForIL(methodToken, iloffset, allocator, out, out_size, out_lineStart, out_lineEnd, out_columnStart, out_columnEnd);
    }

    return false;
}

extern "C" bool FindLineNumberForNativeMethod(IPdbSymbolReader *reader, const uint32_t rva, const mallocPtr allocator, char **out, uint32_t *out_size, uint32_t *out_lineStart, uint32_t *out_lineEnd, uint16_t *out_columnStart, uint16_t *out_columnEnd)
{
    return false;
}

extern "C" bool GetSrcSrvData(IPdbSymbolReader *reader, uint8_t **out, uint32_t *out_size)
{
    if (reader != nullptr && reader->IsValid())
    {
        return reader->GetSrcSrvData(out, out_size);
    }

    return false;
}

extern "C" bool GetSourceLinkData(IPdbSymbolReader *reader, uint8_t **out, uint32_t *out_size)
{
    if (reader != nullptr && reader->IsValid())
    {
        return reader->GetSourceLinkData(out, out_size);
    }

    return false;
}