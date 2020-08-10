<!--
Copyright (c) 2020, 2020 IBM Corp. and others

This program and the accompanying materials are made available under
the terms of the Eclipse Public License 2.0 which accompanies this
distribution and is available at http://eclipse.org/legal/epl-2.0
or the Apache License, Version 2.0 which accompanies this distribution
and is available at https://www.apache.org/licenses/LICENSE-2.0.

This Source Code may also be made available under the following Secondary
Licenses when the conditions for such availability set forth in the
Eclipse Public License, v. 2.0 are satisfied: GNU General Public License,
version 2 with the GNU Classpath Exception [1] and GNU General Public
License, version 2 with the OpenJDK Assembly Exception [2].

[1] https://www.gnu.org/software/classpath/license.html
[2] http://openjdk.java.net/legal/assembly-exception.html

SPDX-License-Identifier: EPL-2.0 OR Apache-2.0 OR GPL-2.0 WITH Classpath-exception-2.0 OR LicenseRef-GPL-2.0 WITH Assembly-exception
-->

# OMR specific CMake mechanics
This document serves to detail OMR specific CMake functionality.

## Host System Specifics
This section documents how the build system handles and configures the specifics of different combinations of operating systems, toolchains, and processor architectures.
For informational purposes, the functions and files responsible for each step are spelled out, however for most cases including `OmrPlatform.cmake` and then calling `omr_platform_global_setup()` near the top of your `CMakeLists.txt` should suffice.

### omr_detect_system_information()
System information is first gathered by `omr_detect_system_information()`.  Information about the system provided by CMake is normalized.
For example depending on the host OS, the `CMAKE_SYSTEM_PROCESSOR` might report `AMD64`,`amd64`, or `x86_64`. As part of this process the following variables are set:
- OMR_HOST_ARCH - The cpu architecture that the build is targeting. Convenience boolean variables of the form OMR_ARCH_<ARCH_NAME> are also set, eg OMR_ARCH_X86
- OMR_ENV_TARGET_DATASIZE - Specifies address width. Convenience boolean variables OMR_ENV_DATA32 / OMR_ENV_DATA64 are also set based on this variable
- OMR_HOST_OS - The os that the build is targeting. Convenience bool variables of the form OMR_OS_<OS_NAME> are also set, eg OMR_OS_LINUX
- OMR_TOOLCONFIG - specifies which toolchain we are using, currently the possible values are "msvc", "xlc", or "gnu". Note that currently we lump all gnu-like compilers (eg clang) into "gnu"

In addition, `omr_detect_system_information()` is responsible for performing various checks of the build environment.
For example, here we call `omr_check_dladdr` which will set `OMR_HAVE_DLADDR` if the `dladdr()` function exists.

### Platform Configuration

## OMR Target Support
### Compiler Libraries
