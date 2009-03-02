--
-- Copyright 2007 - 2009, Stanislav Karchebnyy <berkus+metta@madfire.net>
--
-- Distributed under the Boost Software License, Version 1.0.
-- (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
--
--
-- Solution for building vesper kernel.
--
-- NB: do not use this script, premake lacks some essential toolchain support (nasm)
-- NB: use premake4-svn (fixes NoExceptions and NoRTTI flags)
--
solution "vesper"
    configurations { "Debug", "Release" }
    location "_build_"

    flags { "ExtraWarnings", "FatalWarnings", "NoExceptions", "NoRTTI", "OptimizeSize" }

    configuration { "linux", "gmake" }
        buildoptions { "-fno-stack-protector", "-fno-leading-underscore" }
    configuration { "linux", "gmake", "C++" }
        buildoptions { "-std=gnu++0x" }

    configuration {}

-- 	configuration "Debug"
-- 		defines { "DEBUG" }
-- 	configuration "Release"
-- 		defines { "RELEASE" }

newoption {
    trigger     = "arch",
    value       = "arch",
    description = "Machine architecture to build for",
    allowed = {
        { "x86",    "x86 (default)" },
        { "x86_64", "x86-64" },
        { "arm11",  "ARM11" },
        { "cortex", "Cortex-A8 ARM core with NEON" }
    }
}
if not _OPTIONS["arch"] then
	_OPTIONS["arch"] = "x86"
end

include_dirs = {  ".", "lib", "runtime", "boot", "memory", "pd", "schedule", "boot", "lib/oskit", "arch/" .. _OPTIONS['arch'], "lib/oskit/oskit/" .. _OPTIONS['arch']  }

for k, value in pairs(include_dirs) do
    include_dirs[k] = 'vesper/'..value
end

-- Move these to config file?
defines {
    "CONFIG_CPU_IA32_P4"
}

-- project "bstrlib"
--     kind "StaticLib"
--     location "_build_/bstrlib"
--     language "C++"
--     includedirs (include_dirs)
--     files { "lib/bstrlib/*.cpp", "lib/bstrlib/*.c", "lib/bstrlib/*.h" }
--     excludes { "lib/bstrlib/test.cpp", "lib/bstrlib/testaux.c", "lib/bstrlib/bstest.c", "lib/bstrlib/cpptest.cpp" }
--     configuration { "linux", "gmake" }
--         buildoptions { "-Wno-strict-overflow" }

-- project "klibc"
--     kind "StaticLib"
--     location "_build_/klibc"
--     language "C++"
--     includedirs (include_dirs)
--     files { "lib/klibc/*.c", "lib/klibc/*.h" }

project "kernel"
    kind "ConsoleApp"
    targetname "vesper"
    location "_build_/vesper"
    language "C++"
    files { "vesper/**.cpp", "vesper/**.h", "vesper/**.s" }
    excludes { "vesper/lib/klibc/**", "vesper/lib/bstrlib/**", "vesper/tests/**", "vesper/initfs/**" }
    includedirs (include_dirs)
    postbuildcommands { "sh update_image.sh" }
    configuration { "linux", "gmake" }
        linkoptions { "-nostdlib", "-nostartfiles", "-nodefaultlibs", "-fno-builtin" }
        linkoptions { "-Wl,-Map,vesper.map", " -T linker.ld" }
--         asmoptions { "-f elf" }

project "lib_tests"
    kind "ConsoleApp"
    location "_build_/tests"
    language "C++"
    files { "vesper/tests/*" }

-- kate: indent-width 4; replace-tabs on;
-- vim: set et sw=4 ts=4 sts=4 cino=(4 :
