# THE LIST OF PLUGINS AND THE CORRESPONDING SOURCE FILES
# ======================================================
set(LLVM_TUTOR_PLUGINS
    StaticCallCounter
    DynamicCallCounter
    FindFCmpEq
    ConvertFCmpEq
    FuncTimer
    InjectFuncCall
    InjectTrampoline
    MBAAdd
    MBASub
    RIV
    DuplicateBB
    OpcodeCounter
    MergeBB
    )

set(StaticCallCounter_SOURCES
  StaticCallCounter.cpp)
set(DynamicCallCounter_SOURCES
  DynamicCallCounter.cpp)
set(FindFCmpEq_SOURCES
  FindFCmpEq.cpp)
set(ConvertFCmpEq_SOURCES
  ConvertFCmpEq.cpp)
set(FuncTimer_SOURCES
  FuncTimer.cpp)
set(InjectFuncCall_SOURCES
  InjectFuncCall.cpp)
set(InjectTrampoline_SOURCES
  InjectTrampoline.cpp)
set(MBAAdd_SOURCES
  MBAAdd.cpp
  Ratio.cpp)
set(MBASub_SOURCES
  MBASub.cpp
  Ratio.cpp)
set(RIV_SOURCES
  RIV.cpp)
set(DuplicateBB_SOURCES
  DuplicateBB.cpp)
set(OpcodeCounter_SOURCES
  OpcodeCounter.cpp)
set(MergeBB_SOURCES
  MergeBB.cpp)

# CONFIGURE THE PLUGIN LIBRARIES
# ==============================
foreach( plugin ${LLVM_TUTOR_PLUGINS} )
    # Create a library corresponding to 'plugin'
    add_library(
      ${plugin}
      SHARED
      ${${plugin}_SOURCES}
      )

    # Configure include directories for 'plugin'
    target_include_directories(
      ${plugin}
      PRIVATE
      "${CMAKE_CURRENT_SOURCE_DIR}/../include"
    )
endforeach()
