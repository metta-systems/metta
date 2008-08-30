#pragma once

extern "C"  Address readInstructionPointer();
extern "C"  Address readStackPointer();
extern "C"  Address readBasePointer();

extern "C"  Address readPageDirectory();
extern "C"  void writePageDirectory(Address pageDirPhysical);
extern "C"  void flushPageDirectory(void);

extern "C"  void enablePaging(void);
extern "C"  void enableInterrupts(void);
extern "C"  void disableInterrupts(void);

// defined in schedule/CriticalSection.cpp
extern "C" void criticalSection();
extern "C" void endCriticalSection();
