#include <string>
#include <iostream>
#include <memory>

#include "oneway.grpc.pb.h"

#include <grpc++/grpc++.h>

class Client final
{
private:
    std::unique_ptr<test::OneWay::Stub> stub_;

public:
    Client(std::shared_ptr<grpc::Channel> channel) : stub_{test::OneWay::NewStub(channel)} {}

    std::string Greet(const std::string& request_sample_field);
};

std::string Client::Greet(const std::string& request_sample_field)
{
    // Prepare request
    test::Request request;
    request.set_word(request_sample_field);

    // Send request
    test::Response response;
    grpc::ClientContext context;
    grpc::Status status;
    status = stub_->Greet(&context, request, &response);

    // Handle response
    if (status.ok())
    {
        return response.word();
    }
    else
    {
        std::cerr << status.error_code() << ": " << status.error_message() << std::endl;
        return "RPC failed";
    }
}

int main(int argc, char** argv)
{
    std::string server_address{"localhost:2510"};
    std::shared_ptr<grpc::Channel> channel = grpc::CreateChannel(server_address, grpc::InsecureChannelCredentials());

    Client client{channel};

    std::string request_sample_field{"world"};
    std::string response_sample_field = client.Greet(request_sample_field);
    std::cout << "Client received: " << response_sample_field << std::endl;

    return 0;
}
