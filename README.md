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
