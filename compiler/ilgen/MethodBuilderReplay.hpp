/*******************************************************************************
 * Copyright (c) 2018, 2018 IBM Corp. and others
 *
 * This program and the accompanying materials are made available under
 * the terms of the Eclipse Public License 2.0 which accompanies this
 * distribution and is available at http://eclipse.org/legal/epl-2.0
 * or the Apache License, Version 2.0 which accompanies this distribution
 * and is available at https://www.apache.org/licenses/LICENSE-2.0.
 *
 * This Source Code may also be made available under the following Secondary
 * Licenses when the conditions for such availability set forth in the
 * Eclipse Public License, v. 2.0 are satisfied: GNU General Public License,
 * version 2 with the GNU Classpath Exception [1] and GNU General Public
 * License, version 2 with the OpenJDK Assembly Exception [2].
 *
 * [1] https://www.gnu.org/software/classpath/license.html
 * [2] http://openjdk.java.net/legal/assembly-exception.html
 *
 * SPDX-License-Identifier: EPL-2.0 OR Apache-2.0 OR GPL-2.0 WITH Classpath-exception-2.0 OR LicenseRef-GPL-2.0 WITH Assembly-exception
 *******************************************************************************/

 #ifndef OMR_METHODBUILDERREPLAY_INCL
 #define OMR_METHODBUILDERREPLAY_INCL

 #ifndef TR_METHODBUILDERREPLAY_DEFINED
 #define TR_METHODBUILDERREPLAY_DEFINED
 #define PUT_OMR_METHODBUILDERREPLAY_INTO_TR
 #endif


 #include "ilgen/MethodBuilder.hpp"

 // Maximum length of _definingLine string (including null terminator)
 #define MAX_LINE_NUM_LEN 7

 class TR_BitVector;
 namespace TR { class BytecodeBuilder; }
 namespace OMR { class VirtualMachineState; }

 namespace OMR
 {

 class MethodBuilderReplay : public TR::MethodBuilder
    {
    public:

    MethodBuilderReplay(TR::TypeDictionary *types, TR::JitBuilderReplay *replay);

    void setReplay(TR::JitBuilderReplay *replay) { _replay = replay; }
    virtual bool buildIL();

    protected:

    TR::JitBuilderReplay         * _replay;

    };

 } // namespace OMR


 #if defined(PUT_OMR_METHODBUILDERREPLAY_INTO_TR)

 namespace TR
 {
    class MethodBuilderReplay : public OMR::MethodBuilderReplay
       {
       public:
          // MethodBuilderReplay(TR::TypeDictionary *types, TR::JitBuilderReplay *replay)
          //    : OMR::MethodBuilderReplay(types, replay)
          //    { }
          MethodBuilderReplay(TR::TypeDictionary *types, TR::JitBuilderReplay *replay=NULL)
             : OMR::MethodBuilderReplay(types, replay)
             { }
       };

 } // namespace TR

 #endif // defined(PUT_OMR_METHODBUILDERREPLAY_INTO_TR)

 #endif // !defined(OMR_METHODBUILDERREPLAY_INCL)
