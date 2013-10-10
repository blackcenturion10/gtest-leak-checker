// Copyright 2013 blackcenturion10. All Rights Reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Author: blackcenturion10@gmail.com (Junwon Lee)

// A memory leak checker/locator for Google Test in MSVC.

#include "gtest/gtest.h"

namespace leak_checker {

#ifdef _WIN32

inline int __cdecl ReportHook(int /* report_type */, char* message,
                              int* /* returnValue */) {
  printf("%s", message);
  return true;
}

// This event listener monitors how many bytes are allocated and freed
// by each test, and reports a if test leaks some bytes. It does this
// by comparing memory state at the beginning of a test and at the end
// of a test.
class LeakChecker : public testing::EmptyTestEventListener {
#ifdef _DEBUG
 private:
  // Called before a test starts.
  virtual void OnTestStart(const testing::TestInfo& /* test_info */) {
    ::_CrtMemCheckpoint(&initially_allocated_);
  }

  // Called after a test ends.
  virtual void OnTestEnd(const testing::TestInfo& test_info) {
    if (test_info.result()->Passed()) CheckLeak(test_info);
  }

  // Generates failure if there is difference between memory allocated before 
  // a test starts and after the test ends.
  void CheckLeak(const testing::TestInfo& test_info) {
    ::_CrtMemState difference, allocated;
    ::_CrtMemCheckpoint(&allocated);
    if (::_CrtMemDifference(&difference, &initially_allocated_, &allocated)) {
      leaked_ = true;   
      ADD_FAILURE() << test_info.test_case_name() << "." << test_info.name() 
        << " leaked " << difference.lSizes[1] << " byte(s).";
    }
  }

  // Called when this program starts.
  virtual void OnTestProgramStart(const testing::UnitTest& /* unit_test */) {
    ::_CrtSetReportHook2(_CRT_RPTHOOK_INSTALL, &ReportHook);
    ::_CrtMemCheckpoint(&program_start_);
    leaked_ = false;
  }

  // Called when this program ends.
  virtual void OnTestProgramEnd(const testing::UnitTest& /* unit_test */) {
    if (leaked_) ::_CrtMemDumpAllObjectsSince(&program_start_);
    ::_CrtSetReportHook2(_CRT_RPTHOOK_REMOVE, &ReportHook);
  }

  ::_CrtMemState initially_allocated_;
  ::_CrtMemState program_start_;
  bool leaked_;
#endif // _DEBUG
};

#endif // _WIN32

} // namespace leak_checker
