// SPDX-License-Identifier: MIT
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
std::unordered_map<std::string, std::vector<std::string>> LlvmMcRunnerARM32::fpu_type_map {
	{ "arm1020e",               {}},
	{ "arm1020t",               {}},
	{ "arm1136jf-s",            {}},
	{ "arm7500fe",              {}},
	{ "crypto-neon-fp-armv8",   {"+crypto", "+neon", "+armv8-a"}},
	{ "crypto-neon-fp-armv8.1", {"+crypto", "+neon", "+armv8.1-a"}},
	{ "fp-armv8",               {"+armv8-a"}},
	{ "fpa",                    {}},
	{ "fpa10",                  {}},
	{ "fpa11",                  {}},
	{ "fpe",                    {}},
	{ "fpe2",                   {}},
	{ "fpe3",                   {}},
	{ "fpv4-sp-d16",            {}},
	{ "fpv5-d16",               {}},
	{ "fpv5-sp-d16",            {}},
	{ "maverick",               {}},
	{ "neon",                   {"+neon"}},
	{ "neon-fp-armv8",          {"+neon", "+armv8-a", "+fp-armv8"}},
	{ "neon-fp-armv8.1",        {"+neon", "+armv8.1-a", "+fp-armv8"}},
	{ "neon-fp16",              {"+neon", "+fp16"}},
	{ "neon-vfpv3",             {"+neon", "+vfp3"}},
	{ "neon-vfpv4",             {"+neon", "+vfp4"}},
	{ "softfpa",                {"+soft-float"}},
	{ "softvfp",                {}}, // no llvm-mc equivalent?
	{ "softvfp+vfp",            {}}, // no llvm-mc equivalent?
	{ "vfp",                    {}}, // no llvm-mc equivalent?
	{ "vfp10",                  {}}, // no llvm-mc equivalent?
	{ "vfp10-r0",               {}}, // no llvm-mc equivalent?
	{ "vfp9",                   {}}, // no llvm-mc equivalent?
	{ "vfpv2",                  {"+vfp2"}},
	{ "vfpv3",                  {"+vfp3", "thumb2"}},
	{ "vfp3",                   {"+vfp3", "thumb2"}}, // undocumented GAS option, alias for vfpv3 above
	{ "vfpv3-d16",              {"+vfp3d16"}},
	{ "vfpv3-d16-fp16",         {"vfp3d16,+fp16"}},
	{ "vfpv3-fp16",             {"+vfp3,+fp16"}},
	{ "vfpv3xd",                {}}, // no llvm-mc equivalent?
	{ "vfpv3xd-d16",            {}}, // no llvm-mc equivalent?
	{ "vfpv4",                  {"+vfp4"}},
	{ "vfpv4-d16",              {"+vfp4d16"}},
	{ "vfpxd",                  {}}, // no llvm-mc equivalent?
};

void LlvmMcRunnerARM32::map_option (std::string const& gas_name, std::string const& value)
{
	if (gas_name != "mfpu") {
		return;
	}

	if (value.empty ()) {
		throw invalid_argument_error { "The `-mfpu` option requires a value, argument `value` must not be empty" };
	}

	auto mc_fpu = fpu_type_map.find (value);
	if (mc_fpu == fpu_type_map.end ()) {
		throw invalid_argument_error { "Unknown GAS FPU type: " + value };
	}

	if (mc_fpu->second.empty ()) {
		throw invalid_operation_error {"Unable to map known GAS FPU type '" + value + "' to llvm-mc value" };
	}

	append_attributes (mc_fpu->second);
}
