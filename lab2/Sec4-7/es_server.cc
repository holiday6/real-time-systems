/*
 * Copyright 2020 Chao Wang
 * This code is based on an example from the official gRPC GitHub
 * 
 *
 * Copyright 2015 gRPC authors.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <iostream>
#include <memory>
#include <string>

#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>
#include <grpcpp/ext/proto_server_reflection_plugin.h>

#include "es.grpc.pb.h"

#include <unistd.h>
#include <sched.h>
#include <cmath>

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerReader;
using grpc::ServerWriter;
using grpc::Status;

using es::EventService;
using es::NoUse;
using es::TopicData;
using es::TopicRequest;
pthread_mutex_t mutex_section1 = PTHREAD_MUTEX_INITIALIZER;

int r1 = 0, r2 = 0, r3 = 0;

void workload_1ms(void)
{
  int repeat = 35000; // tune this for the right amount of workload
  for (int i = 0; i <= repeat; i++)
  {
    sqrt(sqrt(i));
    // add some computation here (e.g., use sqrt() in cmath)
  }
}
void pinCPU(int cpu_number)
{
  cpu_set_t mask;
  CPU_ZERO(&mask);

  CPU_SET(cpu_number, &mask);

  if (sched_setaffinity(0, sizeof(cpu_set_t), &mask) == -1)
  {
    perror("sched_setaffinity");
    exit(EXIT_FAILURE);
  }
}

void setSchedulingPolicy(int newPolicy, int priority)
{
  sched_param sched;
  int oldPolicy;
  if (pthread_getschedparam(pthread_self(), &oldPolicy, &sched))
  {
    perror("pthread_setschedparam");
    exit(EXIT_FAILURE);
  }
  sched.sched_priority = priority;
  if (pthread_setschedparam(pthread_self(), newPolicy, &sched))
  {
    perror("pthread_setschedparam");
    exit(EXIT_FAILURE);
  }
}
void *do_one_thing(void *pnum_times)
{
  pinCPU(1);
  setSchedulingPolicy(SCHED_FIFO, 99);
  int i, j, x;

  int period = 50000; //microseconds
  int delta;

  //printf("doing one thing\n");
  std::chrono::system_clock::time_point startTime = std::chrono::system_clock::now();

  pthread_mutex_lock(&mutex_section1);
  for (j = 0; j < 1; j++)
  {
    workload_1ms();
  }
  pthread_mutex_unlock(&mutex_section1);
  std::chrono::system_clock::time_point endTime = std::chrono::system_clock::now();
  delta = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count();
  std::cout<<delta<<std::endl;
  return (NULL);
}

void *do_another_thing(void *pnum_times)
{
  pinCPU(1);
  setSchedulingPolicy(SCHED_FIFO, 98);
  int i, j, x;

  int period = 100000; //microseconds
  int delta;

  //printf("doing another thing\n");
  std::chrono::system_clock::time_point startTime = std::chrono::system_clock::now();
  for (int i = 0; i < 50; i++)
  {
    workload_1ms();
  }
  std::chrono::system_clock::time_point endTime = std::chrono::system_clock::now();
  delta = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count();

  return (NULL);
}

void *do_third_thing(void *pnum_times)
{
  pinCPU(1);
  setSchedulingPolicy(SCHED_FIFO, 97);
  int i, j, x;

  int period = 200000; //microseconds
  int delta;

  //printf("doing another \n");
  std::chrono::system_clock::time_point startTime = std::chrono::system_clock::now();
  workload_1ms();
  pthread_mutex_lock(&mutex_section1);
  for (int i = 0; i < 8; i++)
  {
    workload_1ms();
  }
  pthread_mutex_unlock(&mutex_section1);
  workload_1ms();

  std::chrono::system_clock::time_point endTime = std::chrono::system_clock::now();
  delta = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count();

  (*(int *)pnum_times)++;

  return (NULL);
}
// Logic and data behind the server's behavior.
class EventServiceImpl final : public EventService::Service
{

public:
  Status Subscribe(ServerContext *context,
                   const TopicRequest *request,
                   ServerWriter<TopicData> *writer) override
  {
    //TODO

    // if (request->topic() == H)
    // {
    //   stream_data = H_data;
    // }
    // else if (request->topic() == M)
    // {
    //   stream_data = M_data;
    // }
    // else if (request->topic() == L)
    // {
    //   stream_data = L_data;
    // }

    TopicData td;
    while(stream_data.size()==0){}
    
    td = stream_data.front();
    writer->Write(td);
    stream_data.pop_back();
    std::cout<< stream_data.size() <<std::endl;
    return Status::OK;
  }

  Status Publish(ServerContext *context,
                 ServerReader<TopicData> *reader,
                 NoUse *nouse) override
  {
    //TODO

    TopicData td;
    std::cout << "Start to receive data...\n";
    while (reader->Read(&td))
    {
      std::cout << "{" << td.topic() << ": " << td.data() << "}  ";
    }
    std::cout << "received all data\n";
    int r1;
    TopicData temp = td;
    if (temp.topic() == H)
    {
      if (pthread_create(&high,
                         NULL,
                         do_one_thing,
                         (void *)&r1) != 0)
        perror("pthread_create"), exit(1);
      std::cout<<"thread high created"<<std::endl;
      if (pthread_join(high, NULL) != 0)
        perror("pthread_join"), exit(1);
      
    }
    else if (temp.topic() == M)
    {
      if (pthread_create(&middle,
                         NULL,
                         do_another_thing,
                         (void *)&r1) != 0)
        perror("pthread_create"), exit(1);
      std::cout<<"thread middle created"<<std::endl;
      if (pthread_join(middle, NULL) != 0)
        perror("pthread_join"), exit(1);
      
    }
    else if (temp.topic() == L)
    {
      if (pthread_create(&low,
                         NULL,
                         do_third_thing,
                         (void *)&r1) != 0)
        perror("pthread_create"), exit(1);
      std::cout<<"thread low created"<<std::endl;
      if (pthread_join(low, NULL) != 0)
        perror("pthread_join"), exit(1);
      
    }
    stream_data.push_back(temp);
    std::cout << "done" << std::endl;
    return Status::OK;
  }

  //TODO
private:
  std::string H = "H", M = "M", L = "L";
  pthread_t high, middle, low;
  std::vector<TopicData> stream_data;
  // std::vector<TopicData> H_data;
  // std::vector<TopicData> M_data;
  // std::vector<TopicData> L_data;
};

void RunServer()
{
  std::string server_address("0.0.0.0:50051");
  EventServiceImpl service;

  grpc::EnableDefaultHealthCheckService(true);
  grpc::reflection::InitProtoReflectionServerBuilderPlugin();
  ServerBuilder builder;
  // Listen on the given address without any authentication mechanism.
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  // Register "service" as the instance through which we'll communicate with
  // clients. In this case it corresponds to an *synchronous* service.
  builder.RegisterService(&service);
  // Finally assemble the server.
  std::unique_ptr<Server> server(builder.BuildAndStart());
  std::cout << "Server listening on " << server_address << std::endl;

  // Wait for the server to shutdown. Note that some other thread must be
  // responsible for shutting down the server for this call to ever return.
  server->Wait();
}

int main(int argc, char **argv)
{
  RunServer();

  return 0;
}
