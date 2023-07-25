#include <string>
#include <iostream>
#include <memory>

#include "sample.grpc.pb.h"

#include <grpc++/grpc++.h>

class Client {
public:
    Client(std::shared_ptr<grpc::Channel> channel) :_stub{sample::SampleService::NewStub(channel)} {}

    std::string SampleMethod(const std::string& request_sample_field) {
        // Prepare request
        sample::SampleRequest request;
        request.set_request_sample_field(request_sample_field);

        // Send request
        sample::SampleResponse response;
        grpc::ClientContext context;
        grpc::Status status;
        status = _stub->SampleMethod(&context, request, &response);

        // Handle response
        if (status.ok()) {
            return response.response_sample_field();
        } else {
            std::cerr << status.error_code() << ": " << status.error_message() << std::endl;
            return "RPC failed";
        }
    }

private:
    std::unique_ptr<sample::SampleService::Stub> _stub;
};

int main(int argc, char** argv)
{
    std::string server_address{"localhost:2510"};
    Client client{grpc::CreateChannel(server_address, grpc::InsecureChannelCredentials())};
    std::string request_sample_field{"world"};
    std::string response_sample_field = client.SampleMethod(request_sample_field);
    std::cout << "Client received: " << response_sample_field << std::endl;
    return 0;
}
