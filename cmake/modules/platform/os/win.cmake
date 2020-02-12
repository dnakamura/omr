###############################################################################
# Copyright (c) 2017, 2020 IBM Corp. and others
#
# This program and the accompanying materials are made available under
# the terms of the Eclipse Public License 2.0 which accompanies this
# distribution and is available at http://eclipse.org/legal/epl-2.0
# or the Apache License, Version 2.0 which accompanies this distribution
# and is available at https://www.apache.org/licenses/LICENSE-2.0.
#
# This Source Code may also be made available under the following Secondary
# Licenses when the conditions for such availability set forth in the
# Eclipse Public License, v. 2.0 are satisfied: GNU General Public License,
# version 2 with the GNU Classpath Exception [1] and GNU General Public
# License, version 2 with the OpenJDK Assembly Exception [2].
#
# [1] https://www.gnu.org/software/classpath/license.html
# [2] http://openjdk.java.net/legal/assembly-exception.html
#
# SPDX-License-Identifier: EPL-2.0 OR Apache-2.0 OR GPL-2.0 WITH Classpath-exception-2.0 OR LicenseRef-GPL-2.0 WITH Assembly-exception
#############################################################################

list(APPEND OMR_PLATFORM_DEFINITIONS
	-DOMR_OS_WINDOWS
	-D_WINSOCKAPI_
	-D_HAS_EXCEPTIONS=0

	# Set minimum required system to Win 7, so we can use GetCurrentProcessorNumberEx
	-D_WIN32_WINNT=0x0601
	-DWINVER=0x0601
)

get_filename_component(kit_dir "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows Kits\\Installed Roots;KitsRoot]" REALPATH)
if(OMR_ENV_DATA64)
	set(kit_dir "${kit_dir}/bin/x64")
else()
	set(kit_dir "${kit_dir}/bin/x86")
endif()
# find the message compiler
find_program(CMAKE_MC_COMPILER mc.exe HINTS "${kit_dir}")
if(NOT CMAKE_MC_COMPILER)
	message(SEND_ERROR "Failed to find message compiler (mc.exe)")
endif()
