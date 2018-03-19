CXX_FLAGS="$CXX_FLAGS -fno-rtti -fno-exceptions -fPIC -DBIT64 -std=c++11"
INCLUDES="-I $LLVM_SRC_DIR/include -I $LLVM_BUILD_DIR/include"

clang++ -O2 -shared -o $Output $CXX_FLAGS $INCLUDES InputFile.cpp ManagedSymbolsBuilder.cpp RVASymbolBuilder.cpp Microsoft.BPerf.WindowsPdbReader.cpp -L$LLVM_LIB_DIR -lLLVMBinaryFormat -lLLVMDebugInfoCodeView -lLLVMDebugInfoMSF -lLLVMDebugInfoPDB -lLLVMObject -lLLVMObjectYAML -lLLVMSupport -lLLVMBitReader -lLLVMCore -lLLVMBinaryFormat -lLLVMMCParser -lLLVMMC -lLLVMDebugInfoCodeView -lLLVMDebugInfoMSF -lLLVMSupport -lLLVMDemangle -lcurses

strip -s -R .comment -R .gnu.version --strip-unneeded --keep-symbol=CreatePdbSymbolReader --keep-symbol=DeletePdbSymbolReader --keep-symbol=FindLineNumberForIL --keep-symbol=FindNameForRVA --keep-symbol=GetPdbAndSignatureAndAge --keep-symbol=GetSourceLinkData --keep-symbol=GetSrcSrvData --keep-symbol=IsReaderValid $Output