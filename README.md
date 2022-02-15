[![CI](https://github.com/xamarin/xamarin-android-binutils/actions/workflows/ci.yml/badge.svg)](https://github.com/xamarin/xamarin-android-binutils/actions/workflows/ci.yml)

# Binary utilities for Xamarin.Android 

This repository hosts builds of select
[LLVM](https://llvm.org/) and custom utilities for Xamarin.Android.

Xamarin.Android uses a handful of utilities from LLVM (`llvm-mc`
assembler, `llvm-strip/objcopy` and the `lld` linker), a custom
wrapper for `llvm-mc` which implements the GNU AS interface required
by Mono AOT and Xamarin.Android alike. These utilities are used in
Xamarin.Android application build process, parts of which entail
generation of native assembly for `ARM`, `ARM64`, `x86` and `x86_64`
devices.  They are also required when using the AOT and Profiled AOT
capabilities of Mono and dotnet runtimes.

# Component versions

The build requires sources from various locations and at different
versions.  This section explains how to update and where to find them.

Each change to any of the components below requires that the
Xamarin.Utilities version (see below) is updated as well.

## LLVM

LLVM is a submodule of this repository and its version is determined
automatically by the build scripts.  In order to update LLVM, simply
update the submodule to whichever branch/tag is desired.

## Xamarin.Android Utilities

This version can be found in `common.sh` in the `XA_UTILS_VERSION`
variable.  Every time LLVM is updated, this version has to be changed
as well.

# How to build and publish new version of utilities

  1. Update versions as desired (see `Component_versions`above)
  2. Commit and push the changes
  3. After the build finishes, visit the Actions tab, find the run
     corresponding to commit from `2.` above, download the
     `Xamarin.Android` artifact
  4. On your local machine, execute the release preparation script
     (`TAG_NAME` should be Xamarin.Android utilities version in
     general, but it may contain other bits, as needed):
```shell
  ./prepare-release.sh PATH_TO_ARTIFACT_FROM_4 TAG_NAME
```
  5. The script will produce a `7-zip` package on your local disk and
     display instructions how to proceed with the release.
 
