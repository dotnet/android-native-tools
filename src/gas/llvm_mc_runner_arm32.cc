// SPDX-License-Identifier: MIT
#include "command_line.hh"
#include "exceptions.hh"
#include "llvm_mc_runner.hh"

using namespace xamarin::android::gas;

// Mapping isn't complete, because it's not clear to me what some of the entries need to map to, but since most of them
// are unlikely to be used, I'm providing translation only for the most obvious ones. If there's need for the
// non-obvious ones, they can be researched better at that time.
//
// The llvm-mc arguments are passed in the the `--mattr=` argument, as a comma-separated list with each
// attribute/feature preceded either by `-` to turn it off, or `+` to turn it on. The full list of supported attributes
// can be obtained by running
//
//   llvm-mc --arch=arm --mattr=help < /dev/null
//
std::unordered_map<platform::string, std::vector<platform::string>> LlvmMcRunnerARM32::fpu_type_map {
	{ PSTR("arm1020e"),               {}},
	{ PSTR("arm1020t"),               {}},
	{ PSTR("arm1136jf-s"),            {}},
	{ PSTR("arm7500fe"),              {}},
	{ PSTR("crypto-neon-fp-armv8"),   {PSTR("+crypto"), PSTR("+neon"), PSTR("+armv8-a")}},
	{ PSTR("crypto-neon-fp-armv8.1"), {PSTR("+crypto"), PSTR("+neon"), PSTR("+armv8.1-a")}},
	{ PSTR("fp-armv8"),               {PSTR("+armv8-a")}},
	{ PSTR("fpa"),                    {}},
	{ PSTR("fpa10"),                  {}},
	{ PSTR("fpa11"),                  {}},
	{ PSTR("fpe"),                    {}},
	{ PSTR("fpe2"),                   {}},
	{ PSTR("fpe3"),                   {}},
	{ PSTR("fpv4-sp-d16"),            {}},
	{ PSTR("fpv5-d16"),               {}},
	{ PSTR("fpv5-sp-d16"),            {}},
	{ PSTR("maverick"),               {}},
	{ PSTR("neon"),                   {PSTR("+neon")}},
	{ PSTR("neon-fp-armv8"),          {PSTR("+neon"), PSTR("+armv8-a"), PSTR("+fp-armv8")}},
	{ PSTR("neon-fp-armv8.1"),        {PSTR("+neon"), PSTR("+armv8.1-a"), PSTR("+fp-armv8")}},
	{ PSTR("neon-fp16"),              {PSTR("+neon"), PSTR("+fp16")}},
	{ PSTR("neon-vfpv3"),             {PSTR("+neon"), PSTR("+vfp3")}},
	{ PSTR("neon-vfpv4"),             {PSTR("+neon"), PSTR("+vfp4")}},
	{ PSTR("softfpa"),                {PSTR("+soft-float")}},
	{ PSTR("softvfp"),                {}}, // no llvm-mc equivalent?
	{ PSTR("softvfp+vfp"),            {}}, // no llvm-mc equivalent?
	{ PSTR("vfp"),                    {}}, // no llvm-mc equivalent?
	{ PSTR("vfp10"),                  {}}, // no llvm-mc equivalent?
	{ PSTR("vfp10-r0"),               {}}, // no llvm-mc equivalent?
	{ PSTR("vfp9"),                   {}}, // no llvm-mc equivalent?
	{ PSTR("vfpv2"),                  {PSTR("+vfp2")}},
	{ PSTR("vfpv3"),                  {PSTR("+vfp3")}},
	{ PSTR("vfp3"),                   {PSTR("+vfp3")}}, // undocumented GAS option, alias for vfpv3 above
	{ PSTR("vfpv3-d16"),              {PSTR("+vfp3d16")}},
	{ PSTR("vfpv3-d16-fp16"),         {PSTR("vfp3d16,+fp16")}},
	{ PSTR("vfpv3-fp16"),             {PSTR("+vfp3,+fp16")}},
	{ PSTR("vfpv3xd"),                {}}, // no llvm-mc equivalent?
	{ PSTR("vfpv3xd-d16"),            {}}, // no llvm-mc equivalent?
	{ PSTR("vfpv4"),                  {PSTR("+vfp4")}},
	{ PSTR("vfpv4-d16"),              {PSTR("+vfp4d16")}},
	{ PSTR("vfpxd"),                  {}}, // no llvm-mc equivalent?
};

void LlvmMcRunnerARM32::map_option (platform::string const& gas_name, platform::string const& value)
{
	if (gas_name != PSTR("mfpu")) {
		return;
	}

	if (value.empty ()) {
		throw invalid_argument_error { "The `-mfpu` option requires a value, argument `value` must not be empty" };
	}

	auto mc_fpu = fpu_type_map.find (value);
	if (mc_fpu == fpu_type_map.end ()) {
		platform::string message { PSTR("Unknown GAS FPU type: ") };
		message.append (value);
		throw invalid_argument_error { message };
	}

	if (mc_fpu->second.empty ()) {
		platform::string message { PSTR("Unable to map known GAS FPU type '") };
		message.append (value);
		message.append (PSTR("' to llvm-mc value"));
		throw invalid_operation_error { message };
	}

	append_attributes (mc_fpu->second);
}
