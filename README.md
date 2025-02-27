[![CI](https://devdiv.visualstudio.com/DevDiv/_apis/build/status%2FXamarin%2FAndroid%2Fxamarin-android-binutils?repoName=dotnet%2Fandroid-native-tools&branchName=main)](https://devdiv.visualstudio.com/DevDiv/_build/latest?definitionId=17684&repoName=dotnet%2Fandroid-native-tools&branchName=main)

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
  3. Find the build pipeline instance you want to create a release for,
     and approve the "Release" stage. This stage will wait for manual
     approval for up to two hours, and can be re-ran to trigger a new
     release workflow after it times out if needed.
  4. After approving the release stage a new GitHub release should be
     created with the relevant build artifacts from the pipeline.
     The tag will generated based on the binutils version and
     LLVM version.
  5. Edit the automatically created GitHub release to fill in any
     relevent release notes or other information.
