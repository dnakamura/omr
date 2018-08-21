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

 #include <cstring>
 #include <queue>
 #include <iostream>
 #include <stdlib.h>
 #include <fstream>
 #include <string>
 #include <vector>
 #include <map>

 #include "omrthread.h"

 // gRPC includes
 #include "infra/Assert.hpp"
 #include <grpc/grpc.h>
 #include <grpcpp/server.h>
 #include <grpcpp/server_builder.h>
 #include <grpcpp/server_context.h>
 #include <grpcpp/security/server_credentials.h>
 #include <grpc/support/log.h>
 #include "imperium.grpc.pb.h"

 #include <grpcpp/grpcpp.h>

 // Server
 using grpc::Server;
 using grpc::ServerBuilder;
 using grpc::ServerContext;
 using grpc::ServerReaderWriter;
 using grpc::Status;
 using grpc::StatusCode;

 // Client
 using grpc::Channel;
 using grpc::ClientContext;
 using grpc::ClientReader;
 using grpc::ClientReaderWriter;
 using grpc::ClientWriter;

 using imperium::ClientMessage;
 using imperium::ServerResponse;
 using imperium::ImperiumRPC;
 using imperium::CodeCacheRequest;
 using imperium::CodeCacheResponse;


 namespace TR {class MethodBuilder;}

// include header files for proto stuff HERE

namespace OMR
{
   namespace Imperium
   {
      class ServerChannel final : public ImperiumRPC::Service
      {
         public:
         ServerChannel();
         ~ServerChannel();

         Status CompileMethodAsync(ServerContext* context,
                           ServerReaderWriter<ServerResponse, ClientMessage>* stream) override;
         Status RequestCodeCache(ServerContext* context,
                           const CodeCacheRequest* request, CodeCacheResponse* reply) override;
         Status CompileMethod(ServerContext* context,
                           const ClientMessage* request, ServerResponse* reply) override;
         void generateServerResponse(const ClientMessage * clientMessage, ServerResponse * reply);

         // Server-facing
         bool RunServer(const char * port);

         private:
         omrthread_monitor_t _compileMonitor;

         typedef struct CachedMethodData
            {
             uint8_t * entry;
             uint64_t offset;
             uint64_t sizeCode;
            } CachedMethodData;

         std::map<std::string, CachedMethodData> _functions;
      };

      class ClientChannel
      {
         public:
         enum ThreadStatus {
         INITIALIZATION,
         RUNNING,
         NO_JOBS_LEFT,
         SHUTDOWN_REQUESTED,
         SHUTDOWN_COMPLETE,
         ERROR
         };

         ClientChannel(std::string serverAddress);
         ~ClientChannel();

         typedef std::shared_ptr<ClientReaderWriter<ClientMessage, ServerResponse>> sharedPtr;

         void requestCompile(char * fileName, uint8_t ** entryPoint, TR::MethodBuilder *mb);
         void requestCompileSync(char * fileName, uint8_t ** entry, TR::MethodBuilder *mb);

         void shutdown();

         private:
         sharedPtr _stream;
         ClientContext _context;
         ClientContext _codeCacheContext;
         omrthread_monitor_t _threadMonitor;
         omrthread_monitor_t _queueMonitor;
         ThreadStatus _writerStatus;
         ThreadStatus _readerStatus;
         std::queue<ClientMessage> _queueJobs;
         std::unique_ptr<ImperiumRPC::Stub> _stub;
         void * _virtualCodeAddress;

         ClientMessage constructMessage(std::string file, uint64_t address);
         void requestCodeCache();
         bool addMessageToTheQueue(ClientMessage message);
         void signalNoJobsLeft();
         void waitForThreadsCompletion();
         omrthread_t attachSelf();
         ClientMessage getNextMessage();
         bool isWriteComplete();
         bool isReadComplete();
         void createWriterThread();
         void createReaderThread();
         static int writerThread(void * data);
         static int readerThread(void *data);
         void handleWrite();
         void handleRead();
         bool isQueueEmpty();
      };
   }
} // namespace OMR
