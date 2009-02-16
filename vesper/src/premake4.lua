-- Solution for building vesper kernel.
solution "vesper"
	configurations { "Debug", "Release" }
	location "_build_"

	flags { "ExtraWarnings", "FatalWarnings", "NoExceptions", "NoRTTI", "OptimizeSize" }

	configuration { "linux", "gmake" }
		buildoptions { "-nostdlib", "-nostartfiles", "-nodefaultlibs", "-fno-builtin" }
		buildoptions { "-fno-stack-protector", "-fno-leading-underscore" }
		-- the following must be set by flags but it doesn't work
		buildoptions { "-Wextra", "-Werror" }
	configuration { "linux", "gmake", "C++" }
		buildoptions { "-fno-rtti", "-fno-exceptions" }

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

project "bstrlib"
	kind "StaticLib"
	location "_build_/bstrlib"
	language "C++"
	includedirs (include_dirs)
	files { "lib/bstrlib/*.cpp", "lib/bstrlib/*.c", "lib/bstrlib/*.h" }
	excludes { "lib/bstrlib/test.cpp", "lib/bstrlib/testaux.c", "lib/bstrlib/bstest.c", "lib/bstrlib/cpptest.cpp" }
	configuration { "linux", "gmake" }
		buildoptions { "-Wno-strict-overflow" }

project "klibc"
	kind "StaticLib"
	location "_build_/klibc"
	language "C++"
	includedirs (include_dirs)
	files { "lib/klibc/*.c", "lib/klibc/*.h" }

-- project "kernel"
-- 	kind "ConsoleApp"
-- 	location "_build_"
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
