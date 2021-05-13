[![CI](https://github.com/xamarin/xamarin-android-binutils/actions/workflows/ci.yml/badge.svg)](https://github.com/xamarin/xamarin-android-binutils/actions/workflows/ci.yml)

# GNU Binutils for Xamarin.Android 

This repository hosts builds of [GNU Binutils](https://sourceware.org/binutils) for Xamarin.Android.

Xamarin.Android uses a handful of utilities from Binutils (`as`, `ld`
and `strip`) in its application build process, parts of which entail
generation of native assembly for `ARM`, `ARM64`, `x86` and `x86_64`
devices.  Until NDK release 22 these utilities were part of its
distribution, but with release 23 almost all of them (except for `as`)
are going to be removed.  Thus the need to build our own copies.

The Binutils sources are built in this repository without any changes.
After the build is done, we create a tarball with the binaries for
Linux, macOS and Windows.  The tarball is then used by Xamarin.Android
to package the utilities for Xamarin.Android distribution.

# How to build and publish new version of GNU Binutils

  1. Edit `.github/workflows/ci.yml`
  2. Set the `BINUTILS_VERSION` variable to the new version number of
     the upstream package.
  3. Commit and push the changes
  4. After the build finishes, visit the Actions tab, find the run
     corresponding to commit from `3.` above, download the
     `Xamarin.Android` artifact
  5. On your local machine, execute the release preparation script:
     ```shell
  ./prepare-release.sh PATH_TO_ARTIFACT_FROM_4 TAG_NAME
     ```
     `TAG_NAME` should be created by concatenating the Binutils version
     with the `-XA.NUM` string, where `NUM` is an integer increasing
     monotonously each time a new release of the same Binutils version
     is made.
  6. The script will produce a `7-zip` package on your local disk and
     display instructions how to proceed with the release.
	 
