#include <memory>
#include <string>
#include <atomic>
#include <thread>
#include <chrono>

#include <grpc++/grpc++.h>

#include "bidirectional.grpc.pb.h"

class Client final
{
private:
    using ObserveStream = ::grpc::ClientReaderWriter<::google::protobuf::Empty, ::test::State>;

    std::atomic<bool> run_th_;
    std::thread th_;
    std::string address_;
    std::shared_ptr<test::Bidirectional::Stub> stub_;
    std::unique_ptr<grpc::ClientContext> context_;

    void ObserveLoop();
public:
    explicit Client(const std::string& server_address);
    ~Client() = default;

    void StartLoop();
    void EndLoop();
};

Client::Client(const std::string& server_address) :
    address_{server_address},
    run_th_{true}
{
}

void Client::ObserveLoop()
{
    using namespace std::this_thread; // sleep_for, sleep_until
    using namespace std::chrono; // nanoseconds, system_clock, seconds

    while(run_th_)
    {
        std::shared_ptr<grpc::ChannelCredentials> channel_creds = ::grpc::InsecureChannelCredentials();
        std::shared_ptr<grpc::Channel> channel = ::grpc::CreateChannel(address_, channel_creds);
        stub_ = test::Bidirectional::NewStub(channel);

        context_ = std::make_unique<::grpc::ClientContext>();
        std::unique_ptr<ObserveStream> current_connection = stub_->Hold(context_.get());

        test::State state;
        try
        {
            while (current_connection->Read(&state))
            {
                std::cout << "State updated to " << std::boolalpha << state.on() << std::endl;
            }
        }
        catch(const std::exception& e)
        {
            std::cerr << "Server connection dropped : " << e.what() << std::endl;
            break;
        }

        sleep_for(milliseconds(50));
    }
}

void Client::StartLoop()
{
    if (!th_.joinable())
    {
        th_ = std::thread(&Client::ObserveLoop, this);
    }
}

void Client::EndLoop()
{
    if (th_.joinable())
    {
        run_th_ = false;
        if (context_) {
            context_->TryCancel();
        }
        th_.join();
    }
}

int main()
{
    Client client("localhost:4323");
    client.StartLoop();

    std::cout << "Press [ENTER] to exit the client app" << std::endl;
    std::cin.get();

    client.EndLoop();

    return 0;
}
