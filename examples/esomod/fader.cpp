// Copyright (C) 2010-2014 Joshua Boyce.
// See the file COPYING for copying permission.

#include "fader.hpp"

#include <iostream>

#include <hadesmem/error.hpp>
#include <hadesmem/find_pattern.hpp>
#include <hadesmem/process.hpp>
#include <hadesmem/read.hpp>
#include <hadesmem/write.hpp>

void ToggleFader(hadesmem::Process const& process)
{
  std::cout << "\nPreparing to toggle fader.\n";

  // eso.live.1.5.7.1091867 (dumped with module base of 0x00F70000)
  // .text:011C3915                 lea     edi, [esi+90h]
  // .text:011C391B                 lea     ebx, [esi+1C8h]
  // .text:011C3921                 fstp    dword ptr [ebx]
  // .text:011C3923                 cmp     ds:byte_21282AA, 0
  auto const fader_flag_ref = static_cast<std::uint8_t*>(
    hadesmem::Find(process,
                   L"",
                   L"8D BE ?? ?? ?? ?? 8D 9E ?? ?? ?? ?? D9 1B 80 3D",
                   hadesmem::PatternFlags::kThrowOnUnmatch,
                   0));
  std::cout << "Got fader flag ref. [" << static_cast<void*>(fader_flag_ref)
            << "].\n";

  auto const kFaderFlagRefOffset = 0x10;
  auto const fader_flag_ptr = hadesmem::Read<std::uint8_t*>(
    process, fader_flag_ref + kFaderFlagRefOffset);
  std::cout << "Got fader flag ptr. [" << static_cast<void*>(fader_flag_ptr)
            << "].\n";

  auto const fader_flag = hadesmem::Read<std::uint8_t>(process, fader_flag_ptr);
  std::cout << "Old fader flag is " << static_cast<std::uint32_t>(fader_flag)
            << ".\n";
  auto const new_fader_flag = static_cast<std::uint8_t>(!fader_flag);
  hadesmem::Write(process, fader_flag_ptr, new_fader_flag);
  std::cout << "New fader flag is "
            << static_cast<std::uint32_t>(new_fader_flag) << ".\n";
}
