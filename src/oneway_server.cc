#include <memory>
#include <iostream>
#include <string>

#include "oneway.grpc.pb.h"
#include <grpc++/grpc++.h>

class Service final : public test::OneWay::Service
{
    inline grpc::Status Greet(grpc::ServerContext* context, const test::Request* request, test::Response* response) override
    {
        response->set_word("Hello " + request->word());
        return grpc::Status::OK;
    }
};

void RunServer() {
    std::string server_address{"localhost:2510"};
    Service service;

    // Build server
    grpc::ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
    std::unique_ptr<grpc::Server> server{builder.BuildAndStart()};

    // Run server
    std::cout << "Server listening on " << server_address << std::endl;
    server->Wait();
}

int main(int argc, char** argv)
{
    std::string server_address{"localhost:2510"};
    Service service;

    // Build server
    grpc::ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    // Run server
    std::unique_ptr<grpc::Server> server{builder.BuildAndStart()};

    std::cout << "Server listening on " << server_address << std::endl;
    std::cout << "Press [ENTER] to exit the server app" << std::endl;

    std::cin.get();
    // server->Wait();

    return 0;
}
