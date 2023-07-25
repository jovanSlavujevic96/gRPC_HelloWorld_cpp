#include <memory>
#include <iostream>
#include <string>

#include "sample.grpc.pb.h"
#include <grpc++/grpc++.h>

class SampleServiceImpl final : public sample::SampleService::Service
{
    grpc::Status SampleMethod(grpc::ServerContext* context, const sample::SampleRequest* request, sample::SampleResponse* response) override
    {
        response->set_response_sample_field("Hello " + request->request_sample_field());
        return grpc::Status::OK;
    }
};

void RunServer() {
    std::string server_address{"localhost:2510"};
    SampleServiceImpl service;

    // Build server
    grpc::ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
    std::unique_ptr<grpc::Server> server{builder.BuildAndStart()};

    // Run server
    std::cout << "Server listening on " << server_address << std::endl;
    server->Wait();
}

int main(int argc, char** argv) {
    RunServer();
    return 0;
}