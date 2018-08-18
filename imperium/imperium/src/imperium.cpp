/*******************************************************************************
 * Copyright (c) 2018 IBM Corp. and others
 *
 * This program and the accompanying materials are made available under
 * the terms of the Eclipse Public License 2.0 which accompanies this
 * distribution and is available at https://www.eclipse.org/legal/epl-2.0/
 * or the Apache License, Version 2.0 which accompanies this distribution and
 * is available at https://www.apache.org/licenses/LICENSE-2.0.
 *
 * This Source Code may also be made available under the following
 * Secondary Licenses when the conditions for such availability set
 * forth in the Eclipse Public License, v. 2.0 are satisfied: GNU
 * General Public License, version 2 with the GNU Classpath
 * Exception [1] and GNU General Public License, version 2 with the
 * OpenJDK Assembly Exception [2].
 *
 * [1] https://www.gnu.org/software/classpath/license.html
 * [2] http://openjdk.java.net/legal/assembly-exception.html
 *
 * SPDX-License-Identifier: EPL-2.0 OR Apache-2.0 OR GPL-2.0 WITH Classpath-exception-2.0 OR LicenseRef-GPL-2.0 WITH Assembly-exception
 *******************************************************************************/

 #include "Jit.hpp"
 #include "jitbuilder/env/FrontEnd.hpp"
 #include "ilgen/TypeDictionary.hpp"
 #include "ilgen/JitBuilderRecorderTextFile.hpp"
 #include "ilgen/JitBuilderReplayTextFile.hpp"
 #include "ilgen/MethodBuilderReplay.hpp"
 #include "runtime/CodeCacheManager.hpp"
 #include "runtime/CodeCacheMemorySegment.hpp"
 #include "imperium/imperium.hpp"

 #include <sys/mman.h>

 #include <thread>

 using namespace OMR::Imperium;

//  #define DEBUG

   /****************
   * ClientChannel *
   ****************/

  /*
   * Initializes OMR thread library
   * Attaches self (main) thread
   * And initializes the monitor
   */
  ClientChannel::ClientChannel(std::string serverAddress)
     {
      attachSelf();

      _writerStatus = ThreadStatus::INITIALIZATION;
      _readerStatus = ThreadStatus::INITIALIZATION;

      #ifdef DEBUG
      std::cout << "imperium: initializing monitor" << std::endl;
      #endif

      if(J9THREAD_SUCCESS != omrthread_monitor_init(&_threadMonitor, 0))
      {
        #ifdef DEBUG
        std::cout << "imperium: ERROR initializing thread monitor" << std::endl;
        #endif
      }

      if(J9THREAD_SUCCESS != omrthread_monitor_init(&_queueMonitor, 0))
      {
        #ifdef DEBUG
        std::cout << "imperium: ERROR initializing queue monitor" << std::endl;
        #endif
      }     
      
      std::shared_ptr<Channel> channel = grpc::CreateChannel(
              serverAddress, grpc::InsecureChannelCredentials());

      _stub = ImperiumRPC::NewStub(channel);

      requestCodeCache();
      
      // create code cache and send initial to server (message: init code cache)
      // _stream = _stub->CompileMethodAsync(&_context);

      // ClientMessage --> CompileRequest
      // ServerReponse --> CompileComplete

      // Call createWriterThread which creates a thread that writes
      // to the server based on the queue
      // It will handle sending off .out files to the server
      // createWriterThread();

      // Call createReaderThread which creates a thread that waits
      // and listens for messages from the server
      // createReaderThread();
     }

  ClientChannel::~ClientChannel()
     {
      #ifdef DEBUG
      std::cout << "imperium: ClientChannel destructor called!!" << std::endl;
      #endif      
     }

  void ClientChannel::requestCodeCache() 
     {
      ClientContext codeCacheContext;
      CodeCacheRequest request;
      CodeCacheResponse reply;
      Status status = _stub->RequestCodeCache(&codeCacheContext, request, &reply);

      #ifdef DEBUG
      std::cout << "imperium: RECEIVED REPLY FOR CODE CACHE ******** base address: " << std::hex << reply.codecachebaseaddress() << " charlie address: " << (reply.codecachebaseaddress() - 0x8) << " size: " << std::dec << reply.size() << std::endl;
      #endif      
      }

  void ClientChannel::requestCompileSync(char * fileName, uint8_t ** entry, TR::MethodBuilder *mb) 
     {
        ClientContext codeCacheContext;
        ServerResponse reply;

        int32_t rc = recordMethodBuilder(mb);

        std::string fileString;
        std::ifstream _file(fileName);
        std::string line;

        if(_file.is_open())
        {
        while(getline(_file,line))
            {
            fileString += line + '\n';
            }
        }
        else
        {
        #ifdef DEBUG
        std::cerr << "imperium: Error while trying to open: " << fileName << std::endl;
        #endif      
        }

        ClientMessage request = constructMessage(fileString, reinterpret_cast<uint64_t>(entry));
        std::remove(fileName);

        std::cout << std::endl << "Requested server compilation..." << std::endl;
        std::cout << "Waiting for server response... " << std::endl;

        Status status = _stub->CompileMethod(&codeCacheContext, request, &reply);
        
        auto fe = JitBuilder::FrontEnd::instance();
        auto codeCacheManager = fe->codeCacheManager();
        auto codeCache = codeCacheManager.getFirstCodeCache();
        void * mem = codeCache->getWarmCodeAlloc();
        void * codeBase = codeCache->getCodeBase();
        void * memLocation = (void *) ((uint64_t) reply.codecacheoffset() + (uint64_t) codeBase);

        memcpy(memLocation , (const void *) reply.instructions().c_str(), reply.size());
        typedef int32_t (SimpleMethodFct)(int32_t);
        SimpleMethodFct *incr = (SimpleMethodFct *) memLocation;
        *entry = (uint8_t *) memLocation;
     }

  void ClientChannel::requestCompile(char * fileName, uint8_t ** entry, TR::MethodBuilder *mb)
     {
       int32_t rc = recordMethodBuilder(mb);

       std::string fileString;
       std::ifstream _file(fileName);
       std::string line;

       if(_file.is_open())
       {
        while(getline(_file,line))
          {
          fileString += line + '\n';
          }
       }
       else
       {
        #ifdef DEBUG
        std::cerr << "imperium: Error while trying to open: " << fileName << std::endl;
        #endif      
       }

      ClientMessage m = constructMessage(fileString, reinterpret_cast<uint64_t>(entry));
      addMessageToTheQueue(m);
      std::remove(fileName);
     }  

  void ClientChannel::shutdown()
  {
    signalNoJobsLeft();

    if(J9THREAD_SUCCESS != omrthread_monitor_destroy(_threadMonitor))
      #ifdef DEBUG
      std::cerr << "imperium: ERROR destroying thread monitor" << std::endl;
      #endif
    if(J9THREAD_SUCCESS != omrthread_monitor_destroy(_queueMonitor))
      #ifdef DEBUG
      std::cerr << "imperium: ERROR destroying queue monitor" << std::endl;
      #endif

    omrthread_detach(omrthread_self());

    #ifdef DEBUG
    std::cout << "imperium: ClientChannel shutdown COMPLETE" << std::endl;
    #endif
  }

  omrthread_t ClientChannel::attachSelf()
     {
       // Attach self thread
       omrthread_t thisThread;
       omrthread_attach_ex(&thisThread,
                           J9THREAD_ATTR_DEFAULT /* default attr */);
       return thisThread;
     }

  void ClientChannel::createReaderThread()
     {
       omrthread_entrypoint_t entpoint = readerThread;
       omrthread_t newThread;

       intptr_t ret1 = omrthread_create(&newThread, 0, J9THREAD_PRIORITY_MAX, 0, entpoint, (void *) this);
     }

  void ClientChannel::createWriterThread()
     {
       omrthread_entrypoint_t entpoint = writerThread;
       omrthread_t newThread;

       intptr_t ret1 = omrthread_create(&newThread, 0, J9THREAD_PRIORITY_MAX, 0, entpoint, (void *) this);
     }

  // Signal that there are no more jobs to be added to the queue
  void ClientChannel::signalNoJobsLeft()
     {

     // Must grab both monitors before sending the shutdown signal
     if(J9THREAD_SUCCESS != omrthread_monitor_enter(_threadMonitor))
        #ifdef DEBUG
        std::cout << "imperium: NO_JOBS_LEFT - ERROR while entering thread monitor" << std::endl;
        #endif
     if(J9THREAD_SUCCESS != omrthread_monitor_enter(_queueMonitor))
        #ifdef DEBUG
        std::cout << "imperium: NO_JOBS_LEFT - ERROR while entering queue monitor" << std::endl;
        #endif       

     if (_writerStatus != ThreadStatus::SHUTDOWN_COMPLETE) {
       _writerStatus = ThreadStatus::SHUTDOWN_REQUESTED;
     }
     if (_readerStatus != ThreadStatus::SHUTDOWN_COMPLETE) {
       _readerStatus = ThreadStatus::SHUTDOWN_REQUESTED;
     }

     #ifdef DEBUG
     std::cout << "imperium: *!*!*!*!*!*!*! NO JOBS LEFT *!*!*!*!*!*!*!" << std::endl;
     #endif

     if(J9THREAD_SUCCESS != omrthread_monitor_notify_all(_threadMonitor))
     {
      #ifdef DEBUG
      std::cout << "imperium: NO_JOBS_LEFT - ERROR while notifying all thread monitor" << std::endl;
      #endif  
     }
     if(J9THREAD_SUCCESS != omrthread_monitor_exit(_threadMonitor))
     {
      #ifdef DEBUG
      std::cout << "imperium: NO_JOBS_LEFT - ERROR while exiting thread monitor" << std::endl;
      #endif   
     }
     // The writer thread could be waiting for more jobs to be added to the queue
     if(J9THREAD_SUCCESS != omrthread_monitor_notify_all(_queueMonitor))
     {
      #ifdef DEBUG
      std::cout << "imperium: NO_JOBS_LEFT - ERROR while notifying all queue monitor" << std::endl;
      #endif  
     }

     if(J9THREAD_SUCCESS != omrthread_monitor_exit(_queueMonitor))
     {
      #ifdef DEBUG
      std::cout << "imperium: NO_JOBS_LEFT - ERROR while exiting queue monitor" << std::endl;
      #endif       
     }

     }

  // TODO: ifdef couts
  void ClientChannel::waitForThreadsCompletion()
     {

     if(J9THREAD_SUCCESS != omrthread_monitor_enter(_threadMonitor))
        std::cout << "NO_JOBS_LEFT: ERROR WHILE ENTERING" << std::endl;

     while(_writerStatus != ThreadStatus::SHUTDOWN_COMPLETE || _readerStatus != ThreadStatus::SHUTDOWN_COMPLETE) {
        omrthread_monitor_wait(_threadMonitor);
     }

     if(J9THREAD_SUCCESS != omrthread_monitor_exit(_threadMonitor))
       std::cout << "NO_JOBS_LEFT: ERROR WHILE EXITING" << std::endl;

     std::cout << "\n\n*!*!*!*!*!*!*! FINISHED WAITING FOR THREAD COMPLETION *!*!*!*!*!*!*!" << "\n\n";
     }

  int ClientChannel::readerThread(void * data)
     {
       ClientChannel *s = (ClientChannel *) data;
       s->handleRead();

       return 1;
     }

  int ClientChannel::writerThread(void * data)
    {
     ClientChannel *s = (ClientChannel *) data;
     s->handleWrite();

     return 1;
     }

  ClientMessage ClientChannel::constructMessage(std::string file, uint64_t address)
     {
       ClientMessage clientMessage;
       clientMessage.set_file(file);
       clientMessage.set_address(address);

       return clientMessage;
     }

  // TODO: ifdef couts
  bool ClientChannel::addMessageToTheQueue(ClientMessage message)
     {
      if(J9THREAD_SUCCESS != omrthread_monitor_enter(_queueMonitor))
         std::cout << "ERROR WHILE ENTERING MONITOR on addJobToTheQueue" << std::endl;

      _queueJobs.push(message);
      std::cout << "++ Added message: " << message.file() << " to the queue." << std::endl;
      std::cout << "++ There are now " << _queueJobs.size() << " jobs in the queue." << std::endl;

      // Notify that there are new jobs to be processed in the queue
      omrthread_monitor_notify_all(_queueMonitor);

      if(J9THREAD_SUCCESS != omrthread_monitor_exit(_queueMonitor))
         std::cout << "ERROR WHILE EXITING MONITOR on addJobToTheQueue" << std::endl;

      return true;
     }

  bool ClientChannel::isQueueEmpty()
     {
      return _queueJobs.empty();
     }

  // TODO: ifdef couts
  ClientMessage ClientChannel::getNextMessage()
     {
      if(isQueueEmpty())
         {
          TR_ASSERT_FATAL(0, "Queue is empty, there are no messages.");
         }
      ClientMessage nextMessage = _queueJobs.front();
      _queueJobs.pop();
      std::cout << "-- About to process job from the queue..." << std::endl;
      std::cout << "-- There are now " << _queueJobs.size() << " jobs in the queue." << std::endl;
      return nextMessage;
     }

  // TODO: ifdef couts
  void ClientChannel::handleRead()
     {
       std::cout << "******************************** Calling handle read..." << std::endl;
       ServerResponse retBytes;
            // TODO: we don't need to read results if we're shutting down.  Right
            // now it will block until the stream is closed.
            static int count = 0;
            while(_stream->Read(&retBytes))
              {
              // print out address received as well
              std::cout << "Client received: " << retBytes.instructions() << ", with size: " << retBytes.size() << ", with address: " << std::hex << retBytes.codecacheoffset() << std::dec << ". Count: " << (++count) << std::endl;
              
              memcpy((void*) retBytes.codecacheoffset(), (const void *) retBytes.instructions().c_str(), retBytes.size());
              typedef int32_t (SimpleMethodFct)(int32_t);
              SimpleMethodFct *incr = (SimpleMethodFct *) retBytes.codecacheoffset();

//                 int32_t v;
//                 v=3; std::cout << "incr(" << v << ") == " << incr(v) << "\n";
//                 v=6; std::cout << "incr(" << v << ") == " << incr(v) << "\n";
//                 v=12; std::cout << "incr(" << v << ") == " << incr(v) << "\n";
//                 v=-19; std::cout << "incr(" << v << ") == " << incr(v) << "\n";
              }

       // Check if status is OK
       Status status = _stream->Finish();

       if (!status.ok())
         {
         std::cout << "OMR SendOutFiles rpc failed." << std::endl;
         std::cout << status.error_message() << std::endl;

         if(status.error_code() == grpc::UNKNOWN)
            {
              std::cout << "UNKNOWN error at server side!!!!!" << std::endl;
              std::cout << "Server side application throws an exception (or does something other than returning a Status code to terminate an RPC)" << std::endl;
            }

         std::cout << "Error details: " << status.error_details() << std::endl;
         std::cout << "Status error code: " << status.error_code() << std::endl;
         }

         omrthread_monitor_enter(_threadMonitor);
         std::cout << "readerStatus changed to SHUTDOWN_COMPLETE!!!!" << std::endl;
         _readerStatus = ThreadStatus::SHUTDOWN_COMPLETE;
         omrthread_monitor_notify_all(_threadMonitor);
         omrthread_exit(_threadMonitor);
     }

  // TODO: ifdef couts
  // Called by thread
  void ClientChannel::handleWrite()
     {

       omrthread_monitor_enter(_queueMonitor);
       while(_writerStatus != ThreadStatus::SHUTDOWN_REQUESTED || !isQueueEmpty())
          {
          if (!isQueueEmpty())
            {
            ClientMessage message = getNextMessage();

            // exit the monitor so more items can be added to the queue
            omrthread_monitor_exit(_queueMonitor);

            _stream->Write(message);
            std::cout << "JUST WROTE SOMETHING TO THE SERVER!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << std::endl;

            omrthread_monitor_enter(_queueMonitor);
            } else
            {
              std::cout << "Waiting inside ClientChannel::handleWrite." << std::endl;
              omrthread_monitor_wait(_queueMonitor);
            }
          }

       omrthread_monitor_exit(_queueMonitor);

       _stream->WritesDone();

       omrthread_monitor_enter(_threadMonitor);
       std::cout << "writeStatus changed to SHUTDOWN_COMPLETE!!!!" << std::endl;
       _writerStatus = ThreadStatus::SHUTDOWN_COMPLETE;
       omrthread_monitor_notify_all(_threadMonitor);
       omrthread_exit(_threadMonitor);
       }

     /****************
     * ServerChannel *
     ****************/

     // TODO: ifdef couts
     ServerChannel::ServerChannel() :
        _functions()
        {
          omrthread_t thisThread;
          omrthread_attach_ex(&thisThread,
                              J9THREAD_ATTR_DEFAULT /* default attr */);

          if(J9THREAD_SUCCESS != omrthread_monitor_init(&_compileMonitor, 0))
             std::cout << "ERROR INITIALIZING monitor" << std::endl;
        }

     // TODO: ifdef couts
     ServerChannel::~ServerChannel()
        {
          if(J9THREAD_SUCCESS != omrthread_monitor_destroy(_compileMonitor))
             std::cout << "ERROR WHILE destroying monitor" << std::endl;

          omrthread_detach(omrthread_self());
        }

     // TODO: ifdef couts
     Status ServerChannel::RequestCodeCache(ServerContext* context, const CodeCacheRequest* request, CodeCacheResponse* reply)  
        {
          #ifdef DEBUG
          std::cout << "imperium: REQUESTING CODE CACHE" << std::endl;
          #endif  

          auto fe = JitBuilder::FrontEnd::instance();
          auto codeCacheManager = fe->codeCacheManager();
          auto codeCache = codeCacheManager.getFirstCodeCache();
          void * codeBase = codeCache->getCodeBase();
          void * codeTop = codeCache->getCodeTop();
          uint64_t size = (uint64_t) codeTop - (uint64_t) codeBase;

          #ifdef DEBUG
          std::cout << "imperium: CODE CACHE - Size: " << size << ", with base address: " << std::hex << (uint64_t) codeBase << std::dec << std::endl;
          #endif  

          reply->set_size(size);
          reply->set_codecachebaseaddress((uint64_t) codeBase);

          return Status::OK;
        }
     
     // TODO: ifdef couts
     Status ServerChannel::CompileMethodAsync(ServerContext* context,
                      ServerReaderWriter<ServerResponse, ClientMessage>* stream)
        {

          ClientMessage clientMessage;
          ServerResponse reply;
          std::cout << "Server waiting for message from client..." << std::endl;

          omrthread_t thisThread;
          omrthread_attach_ex(&thisThread,
                              J9THREAD_ATTR_DEFAULT /* default attr */);

          while (stream->Read(&clientMessage))
             {
                generateServerResponse(&clientMessage, &reply);
                stream->Write(reply);
                // Sleeps for 1 second
                // std::this_thread::sleep_for (std::chrono::seconds(1));
             }

          omrthread_detach(thisThread);

          if (context->IsCancelled()) {
            return Status(StatusCode::CANCELLED, "Deadline exceeded or Client cancelled, abandoning.");
          }
          return Status::OK;
        }

     Status ServerChannel::CompileMethod(ServerContext* context, const ClientMessage* request, ServerResponse* reply)  
        {
          #ifdef DEBUG
          std::cout << "imperium: COMPILE METHOD SYNC" << std::endl;
          #endif  
          
          omrthread_t thisThread;
          omrthread_attach_ex(&thisThread,
                              J9THREAD_ATTR_DEFAULT /* default attr */);

          auto fe = JitBuilder::FrontEnd::instance();
          auto codeCacheManager = fe->codeCacheManager();
          auto codeCache = codeCacheManager.getFirstCodeCache();
          void * codeBase = codeCache->getCodeBase();
          void * codeTop = codeCache->getCodeTop();
          uint64_t size = (uint64_t) codeTop - (uint64_t) codeBase;
          
          #ifdef DEBUG
          std::cout << "imperium: CODE CACHE - Size: " << size << ", with base address: " << std::hex << (uint64_t) codeBase << std::dec << std::endl;
          #endif  

          generateServerResponse(request, reply);
          omrthread_detach(thisThread);

          return Status::OK;
        }
     
     void ServerChannel::generateServerResponse(const ClientMessage * clientMessage, ServerResponse * reply)
     {
        std::cout << "Client request received!" << std::endl;
        std::cout << "Generating response..." << std::endl;

        #ifdef DEBUG
        std::cout << "imperium: Server received file: " << clientMessage->file() << std::endl;
        std::cout << "imperium: Server entry point address: " << std::hex << clientMessage->address() << std::dec << std::endl;
        #endif

        omrthread_monitor_enter(_compileMonitor);
        
        #ifdef DEBUG
        std::cout << "imperium: generateServerResponse - entered monitor" << std::endl;
        #endif

        uint8_t *entry = 0;
        uint64_t sizeCode = 0, offset = 0;

        // std::map<const char *, CachedMethodData>::iterator it = _functions.find(strdup(clientMessage->file().c_str()));
        std::map<std::string, CachedMethodData>::iterator it = _functions.find(clientMessage->file());

        if(it == _functions.end())
           {
            TR::JitBuilderReplayTextFile replay(clientMessage->file());

            #ifdef DEBUG
            std::cout << "imperium: generateServerResponse - replay"<< std::endl;
            #endif

            TR::JitBuilderRecorderTextFile recorder(NULL, "mandelbrot.out");

            #ifdef DEBUG
            std::cout << "imperium: generateServerResponse - Thread ID: " << omrthread_self() << std::endl;
            #endif

            TR::TypeDictionary types;

            #ifdef DEBUG
            std::cout << "imperium: generateServerResponse - verify output file" << std::endl;
            #endif

            TR::MethodBuilderReplay mb(&types, &replay, &recorder); // Process Constructor

            std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();

            // Only one thread can compile at a time.

            int32_t rc = compileMethodBuilder(&mb, &entry); // Process buildIL
            omrthread_monitor_exit(_compileMonitor);

            auto fe = JitBuilder::FrontEnd::instance();
            auto codeCacheManager = fe->codeCacheManager();
            auto codeCache = codeCacheManager.getFirstCodeCache();
            void * mem = codeCache->getWarmCodeAlloc();
            void * codeBase = codeCache->getCodeBase();

            offset = (uint64_t) entry - (uint64_t) codeBase;
            sizeCode = (uint64_t) mem - (uint64_t) entry;
            
            #ifdef DEBUG
            std::cout << "imperium: generateServerResponse - ENTRY: " << (void*)(entry) << " *********************** " << std::endl;
            std::cout << "imperium: generateServerResponse - MEM: " << (mem) << " *********************** " << std::endl;
            std::cout << "imperium: generateServerResponse - CODESIZE: " << sizeCode << " *********************** " << std::endl;
            #endif


            std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>( t2 - t1 ).count();

            std::cout << "Duration of compilation on server: " << duration << " microseconds." << std::endl << std::endl;

            CachedMethodData data;
            data.entry = entry;
            data.offset = offset;
            data.sizeCode = sizeCode;
            _functions.insert(std::make_pair(clientMessage->file(), data));
           }
        else
           {
            std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();

            std::cout << "No need to compile!" << std::endl;
            std::cout << "Sending cached data to client..." << std::endl;

            CachedMethodData data = it->second;
            entry = data.entry;
            offset = data.offset;
            sizeCode = data.sizeCode;

            #ifdef DEBUG
            std::cout << "imperium: generateServerResponse - Cached data. Entry: " << (void *)entry << ", offset: " << offset << ", sizeCode: " << sizeCode << std::endl;
            #endif           
           
            std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>( t2 - t1 ).count();
            std::cout << "Server response generated in: " << duration << " microseconds." << std::endl << std::endl;
           }

        reply->set_instructions((char*) entry, sizeCode);
        reply->set_size(sizeCode);
        reply->set_codecacheoffset(offset);
        std::cout << "***" << std::endl << std::endl;
        std::cout << "Waiting for client requests..." << std::endl << std::endl;
     }

     bool ServerChannel::RunServer(const char * port)
     {
        std::string server_address(port); // "localhost:50055"

        ServerBuilder builder;
        builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
        builder.RegisterService(this);
        std::unique_ptr<Server> server(builder.BuildAndStart());
        std::cout << std::endl << "Server listening on " << server_address << std::endl;
        std::cout << "Waiting for client requests..." << std::endl << std::endl;
        server->Wait();

        return true;
     }
