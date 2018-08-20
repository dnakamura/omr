/*******************************************************************************
 *
 * (c) Copyright IBM Corp. 2018, 2018
 *
 *  This program and the accompanying materials are made available
 *  under the terms of the Eclipse Public License v1.0 and
 *  Apache License v2.0 which accompanies this distribution.
 *
 *      The Eclipse Public License is available at
 *      http://www.eclipse.org/legal/epl-v10.html
 *
 *      The Apache License v2.0 is available at
 *      http://www.opensource.org/licenses/apache2.0.php
 *
 * Contributors:
 *    Multiple authors (IBM Corp.) - initial implementation and documentation
 ******************************************************************************/

 #ifndef TR_JITBUILDERREPLAY_TEXTFILE_INCL
 #define TR_JITBUILDERREPLAY_TEXTFILE_INCL

#include "ilgen/OMRJitBuilderReplayTextFile.hpp"

 namespace TR
 {
    class JitBuilderReplayTextFile : public OMR::JitBuilderReplayTextFile
       {
       public:
          JitBuilderReplayTextFile(const char *fileName)
             : OMR::JitBuilderReplayTextFile(fileName)
             { }
          JitBuilderReplayTextFile(std::string fileString)
             : OMR::JitBuilderReplayTextFile(fileString)
             { }
          virtual ~JitBuilderReplayTextFile()
             { }
       };

 } // namespace TR

 #endif // !defined(TR_JITBUILDERREPLAY_TEXTFILE_INCL)
