
solution "vesper"
	configurations { "Debug", "Release" }
	location "_build_"

	flags { "ExtraWarnings", "FatalWarnings", "NoExceptions", "NoRTTI", "OptimizeSize" }

	configuration { "linux", "gmake" }
		buildoptions { "-nostdlib", "-nostartfiles", "-nodefaultlibs", "-fno-builtin" }
		buildoptions { "-fno-stack-protector", "-fno-leading-underscore" }
		-- the following must be set by flags but it doesn't work
		buildoptions { "-Wextra", "-Werror", "-fno-rtti", "-fno-exceptions" }

-- 	configuration "Debug"
-- 		defines { "DEBUG" }
-- 	configuration "Release"
-- 		defines { "RELEASE" }

newoption {
	trigger     = "arch",
	value       = "arch",
	description = "Machine architecture to build for",
	allowed = {
		{ "x86",    "x86" },
		{ "x86_64", "x86-64" },
		{ "arm11",  "ARM11" },
		{ "cortex", "Cortex-A8 ARM core with NEON" }
	}
}
if not _OPTIONS["arch"] then
	_OPTIONS["arch"] = "x86"
end

include_dirs = {  ".", "lib", "boot", "memory", "pd", "schedule", "boot", "lib/oskit", "lib/bstrlib", "lib/klibc", "arch/" .. _OPTIONS['arch'], "lib/oskit/oskit/" .. _OPTIONS['arch']  }

-- Move these to config file?
defines {
	"CONFIG_CPU_IA32_P4",
	'BSTRLIB_CANNOT_USE_STL',
	'BSTRLIB_CANNOT_USE_IOSTREAM',
	'BSTRLIB_DOESNT_THROW_EXCEPTIONS',
	'BSTRLIB_DONT_USE_VIRTUAL_DESTRUCTOR',
	'BSTRLIB_DONT_ASSUME_NAMESPACE'
}

--waf: /bin/g++ -Wall -Wextra -Werror -Os -fno-stack-protector -fno-leading-underscore -fno-rtti -fno-exceptions -nostdlib -nostartfiles -nodefaultlibs -fno-builtin  -Wno-strict-overflow  -Ix86-release/lib/bstrlib -I../lib/bstrlib -Ix86-release/lib -I../lib -Ix86-release/lib/klibc -I../lib/klibc -Ix86-release -I.. -Ix86-release/memory -I../memory -Ix86-release/pd -I../pd -Ix86-release/schedule -I../schedule -Ix86-release/boot -I../boot -Ix86-release/lib/oskit -I../lib/oskit -Ix86-release/arch/x86 -I../arch/x86 -Ix86-release/lib/oskit/oskit/x86 -I../lib/oskit/oskit/x86 -DCONFIG_CPU_IA32_P4 -DBSTRLIB_CANNOT_USE_STL -DBSTRLIB_CANNOT_USE_IOSTREAM -DBSTRLIB_DOESNT_THROW_EXCEPTIONS -DBSTRLIB_DONT_USE_VIRTUAL_DESTRUCTOR -DBSTRLIB_DONT_ASSUME_NAMESPACE -DBOCHS_IO_HACKS ../lib/bstrlib/bstrwrap.cpp -c -o x86-release/lib/bstrlib/bstrwrap_1.o

--pm4: g++  -MMD  -I. -Ilib -Iboot -Imemory -Ipd -Ischedule -Iboot -Ilib/oskit -Ilib/bstrlib -Ilib/klibc -Iarch/x86 -Ilib/oskit/oskit/x86  -Wall -Os -nostdlib -nostartfiles -nodefaultlibs -fno-builtin -fno-stack-protector -fno-leading-underscore --no-exceptions --no-rtti -o obj/Debug/bstrwrap.o -c lib/bstrlib/bstrwrap.cpp

--missing:

project "bstrlib"
	kind "StaticLib"
	language "C++"
	includedirs (include_dirs)
	files { "lib/bstrlib/*.cpp", "lib/bstrlib/*.c", "lib/bstrlib/*.h" }
	excludes { "lib/bstrlib/test.cpp", "lib/bstrlib/testaux.c", "lib/bstrlib/bstest.c", "lib/bstrlib/cpptest.cpp" }
	configuration { "linux", "gmake" }
		buildoptions { "-Wno-strict-overflow" }

-- project "klibc"
-- 	kind "StaticLib"
-- 	language "C++"
-- 	files { "lib/klibc/*.c", "lib/klibc/*.h" }
--
-- project "kernel"
-- 	kind "ConsoleApp"
-- 	language "C++"
-- 	files {}
-- 	links { "bstrlib", "klibc" }

-- solution "lib_tests"
-- 	configurations { "Debug" }
-- 	configuration "Debug"
-- 		defines { "DEBUG" }
--
-- 	project "test_suite"
-- 		language "C++"
-- 		files {}
