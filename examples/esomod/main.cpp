// Copyright (C) 2010-2014 Joshua Boyce.
// See the file COPYING for copying permission.

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <iterator>
#include <memory>
#include <string>
#include <vector>

#include <windows.h>

#include <hadesmem/detail/warning_disable_prefix.hpp>
#include <tclap/cmdline.h>
#include <hadesmem/detail/warning_disable_suffix.hpp>

#include <hadesmem/config.hpp>
#include <hadesmem/debug_privilege.hpp>
#include <hadesmem/detail/to_upper_ordinal.hpp>
#include <hadesmem/error.hpp>
#include <hadesmem/find_pattern.hpp>
#include <hadesmem/process.hpp>
#include <hadesmem/process_helpers.hpp>
#include <hadesmem/read.hpp>
#include <hadesmem/write.hpp>

#include "3d.hpp"
#include "camera_distance.hpp"
#include "fader.hpp"
#include "fog.hpp"
#include "fov.hpp"
#include "time.hpp"
#include "tone_mapping.hpp"
#include "view_distances.hpp"

int main(int argc, char* argv[])
{
  try
  {
    std::cout << "HadesMem ESO Mod [" << HADESMEM_VERSION_STRING << "]\n";

    TCLAP::CmdLine cmd{"ESO Mod", ' ', HADESMEM_VERSION_STRING};
    TCLAP::ValueArg<DWORD> pid_arg{"",
                                   "pid",
                                   "Process ID (for multiple ESO instances)",
                                   false,
                                   0,
                                   "DWORD",
                                   cmd};
    // This should probably have some value constraints set, but it's more
    // entertaining to leave it open so people can set it to silly values.
    TCLAP::ValueArg<float> fov_arg{
      "",
      "fov",
      "Set vertical field of view (in degrees) (default "
      "50.0) for both perspectives",
      false,
      50.0f,
      "float",
      cmd};
    TCLAP::ValueArg<float> fov_3p_arg{
      "",
      "fov-3p",
      "Set vertical field of view (in degrees) (default 50.0) for 3rd person",
      false,
      50.0f,
      "float",
      cmd};
    TCLAP::ValueArg<float> fov_1p_arg{
      "",
      "fov-1p",
      "Set vertical field of view (in degrees) (default 50.0) for 1st person",
      false,
      50.0f,
      "float",
      cmd};
    TCLAP::ValueArg<float> max_camera_distance_arg{
      "",
      "max-camera-dist",
      "Set max camera zoom distance (default 10.0)",
      false,
      10.0f,
      "float",
      cmd};
    TCLAP::ValueArg<float> time_arg{"",
                                    "time",
                                    "Set time (to change day/night/etc) (range "
                                    "of [0, 24), e.g. 13.5 for 1330)",
                                    false,
                                    12.0f,
                                    "float",
                                    cmd};
    // Values above 5 seem to be invalid, but leave it open in case that changes
    // in the future (that way if the patterns don't need updating I don't need
    // to do anything).
    TCLAP::ValueArg<std::uint32_t> tone_mapping_arg{
      "",
      "tone-mapping",
      "Set tone mapping type (a.k.a shader filters) (range of [0, 5])",
      false,
      0,
      "uint32_t",
      cmd};
    TCLAP::ValueArg<float> min_view_distance_arg{
      "",
      "min-view-dist",
      "Set minimum view distance. (default 0.4)",
      false,
      0.40000001f,
      "float",
      cmd};
    TCLAP::ValueArg<float> max_view_distance_arg{
      "",
      "max-view-dist",
      "Set maximum view distance (default 2.0)",
      false,
      2.0f,
      "float",
      cmd};
    TCLAP::ValueArg<float> cur_view_distance_arg{
      "",
      "view-dist",
      "Set current view distance (range of [min-view-dist, max-view-dist])",
      false,
      2.0f,
      "float",
      cmd};
    TCLAP::SwitchArg fog_arg{"", "fog", "Toggle fog (default on)", cmd};
    TCLAP::SwitchArg anaglyph_arg{
      "", "3d", "Toggle anaglyph 3D (default off)", cmd};
    TCLAP::SwitchArg fader_arg{
      "",
      "fader",
      "Toggle fader (turn off to force high quality models) (default on)",
      cmd};
    cmd.parse(argc, argv);

    try
    {
      hadesmem::GetSeDebugPrivilege();

      std::wcout << "\nAcquired SeDebugPrivilege.\n";
    }
    catch (std::exception const& /*e*/)
    {
      std::wcout << "\nFailed to acquire SeDebugPrivilege.\n";
    }

    std::unique_ptr<hadesmem::Process> process;

    if (pid_arg.isSet())
    {
      DWORD const pid = pid_arg.getValue();
      process = std::make_unique<hadesmem::Process>(pid);
    }
    else
    {
      std::wstring const kProcName = L"eso.exe";
      process = std::make_unique<hadesmem::Process>(
        hadesmem::GetProcessByName(kProcName, false));
    }

    bool const set_fov_both = fov_arg.isSet();
    bool const set_fov_3p = fov_3p_arg.isSet();
    bool const set_fov_1p = fov_1p_arg.isSet();
    if (set_fov_both)
    {
      if (set_fov_3p || set_fov_1p)
      {
        HADESMEM_DETAIL_THROW_EXCEPTION(
          hadesmem::Error{} << hadesmem::ErrorString{
            "Please set the FoV using either the perspective-specific flags or "
            "the general flag, but not both."});
      }

      float* fov_both = &fov_arg.getValue();
      SetFov(*process, fov_both, fov_both);
    }
    else if (set_fov_3p || set_fov_1p)
    {
      float* fov_3p = set_fov_3p ? &fov_3p_arg.getValue() : nullptr;
      float* fov_1p = set_fov_1p ? &fov_1p_arg.getValue() : nullptr;
      SetFov(*process, fov_3p, fov_1p);
    }

    if (max_camera_distance_arg.isSet())
    {
      SetMaxCameraDistance(*process, max_camera_distance_arg.getValue());
    }

    if (time_arg.isSet())
    {
      SetTime(*process, time_arg.getValue());
    }

    if (tone_mapping_arg.isSet())
    {
      SetToneMappingType(*process, tone_mapping_arg.getValue());
    }

    bool const set_min_view_distance = min_view_distance_arg.isSet();
    bool const set_max_view_distance = max_view_distance_arg.isSet();
    bool const set_cur_view_distance = cur_view_distance_arg.isSet();
    if (set_min_view_distance || set_max_view_distance || set_cur_view_distance)
    {
      auto const min_view_distance =
        set_min_view_distance ? &min_view_distance_arg.getValue() : nullptr;
      auto const max_view_distance =
        set_max_view_distance ? &max_view_distance_arg.getValue() : nullptr;
      auto const cur_view_distance =
        set_cur_view_distance ? &cur_view_distance_arg.getValue() : nullptr;
      SetViewDistances(
        *process, min_view_distance, max_view_distance, cur_view_distance);
    }

    if (fog_arg.isSet())
    {
      ToggleFog(*process);
    }

    if (anaglyph_arg.isSet())
    {
      Toggle3D(*process);
    }

    if (fader_arg.isSet())
    {
      ToggleFader(*process);
    }

    std::cout << "\nFinished.\n";

    return 0;
  }
  catch (...)
  {
    std::cerr << "\nError!\n";
    std::cerr << boost::current_exception_diagnostic_information() << '\n';

    return 1;
  }
}
