#include <memory>
#include <string>
#include <mutex>

#include <grpc++/grpc++.h>

#include "bidirectional.grpc.pb.h"

class Service final : public test::Bidirectional::Service
{
private:
    using NotifyStream = ::grpc::ServerReaderWriter<::test::State, ::google::protobuf::Empty>;

    std::string address_;
    NotifyStream* client_stream_ = nullptr;
    grpc::ServerContext* client_context_ = nullptr;
    std::mutex client_mtx_;
    std::unique_ptr<grpc::Server> server_;

public:
    explicit Service() = default;
    ~Service() override = default;

    void Notify(const ::test::State& state);

    ::grpc::Status Hold(::grpc::ServerContext* context, NotifyStream* stream) override;

    void StartService(grpc::ServerBuilder& builder);
    void StopService();
};

void Service::Notify(const ::test::State& state)
{
    std::unique_lock<std::mutex> lock(client_mtx_);
    if (client_stream_)
    {
        try
        {
            client_stream_->Write(state);
        }
        catch (const std::exception& e)
        {
            std::cerr << "Notification sent failed : " << e.what() << std::endl;
        }
    }
}

::grpc::Status Service::Hold(::grpc::ServerContext* context, NotifyStream* stream)
{
    {
        std::unique_lock<std::mutex> lock(client_mtx_);
        client_stream_ = stream;
        client_context_ = context;
    }
    try
    {
        ::google::protobuf::Empty empty;
        while (stream->Read(&empty));
    }
    catch (const std::exception& e)
    {
        std::cerr << "Client connection dropped : " << e.what() << std::endl;
    }
    {
        std::unique_lock<std::mutex> lock(client_mtx_);
        client_stream_ = nullptr;
        client_context_ = nullptr;
    }
    return grpc::Status::OK;
}

void Service::StartService(grpc::ServerBuilder& builder)
{
    builder.RegisterService(this);
}

void Service::StopService()
{
    std::unique_lock<std::mutex> lock(client_mtx_);
    if (client_context_)
    {
        client_context_->TryCancel();
    }
}

int main()
{
    Service service;
    grpc::ServerBuilder builder;

    builder.AddListeningPort("0.0.0.0:4323", grpc::InsecureServerCredentials());
    service.StartService(builder);

    std::unique_ptr<grpc::Server> server_ = builder.BuildAndStart();
    std::cout << "Server listening on " << "0.0.0.0:4323" << std::endl;

    char input;
    test::State state;
    while(true)
    {
        std::cout << std::endl << "For ON  state press [Y]" << std::endl;
        std::cout << "For OFF state press [N]" << std::endl;
        std::cout << "To exit server press [Q]" << std::endl << std::endl;
        std::cin >> &input;

        input = tolower(input);

        if ('q' == input)
        {
            break;
        }
        else if ('y' == input || 'n' == input)
        {
            state.set_on('y' == input);
            service.Notify(state);
        }
    }

    service.StopService();

    return 0;
}
