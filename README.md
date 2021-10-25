[![CI](https://github.com/xamarin/xamarin-android-binutils/actions/workflows/ci.yml/badge.svg)](https://github.com/xamarin/xamarin-android-binutils/actions/workflows/ci.yml)

# LLVM binary utilities for Xamarin.Android 

This repository hosts builds of select
[LLVM](https://sourceware.org/binutils) utilities for Xamarin.Android.

Xamarin.Android uses a handful of utilities from LLVM (`llvm-mc`, `lld`
and `strip`) in its application build process, parts of which entail
generation of native assembly for `ARM`, `ARM64`, `x86` and `x86_64`
devices.

# How to build and publish new version of LLVM utilities

  1. Edit `.github/workflows/ci.yml`
  2. Set the `BINUTILS_VERSION` variable to the new version number of
     the upstream package.
  3. Commit and push the changes
  4. After the build finishes, visit the Actions tab, find the run
     corresponding to commit from `3.` above, download the
     `Xamarin.Android` artifact
  5. On your local machine, execute the release preparation script (`TAG_NAME` should be created by concatenating the Binutils version
     with the `-XA.NUM` string, where `NUM` is an integer increasing monotonously each time a new release of the same Binutils version
     is made):
```shell
  ./prepare-release.sh PATH_TO_ARTIFACT_FROM_4 TAG_NAME
```
  6. The script will produce a `7-zip` package on your local disk and
     display instructions how to proceed with the release.
 
