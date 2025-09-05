#include "tcp_like/reliable_session.hpp"
#include <iostream>
#include <thread>

int main() {
    tcp_like::ReliableSession client;
    tcp_like::Endpoint local{"0.0.0.0", 50001};
    tcp_like::Endpoint remote{"127.0.0.1", 50000};
    if (!client.connect(local, remote)) {
        std::cerr << "client connect failed\n";
        return 1;
    }

    std::cout << "echo_client connecting...\n";

    int counter = 0;
    for (;;) {
        client.update();
        if (client.isEstablished()) {
            std::string msg = "hello " + std::to_string(counter++);
            client.send(std::vector<uint8_t>(msg.begin(), msg.end()));
            std::vector<uint8_t> payload;
            while (client.receive(payload)) {
                std::cout << "echo: " << std::string(payload.begin(), payload.end()) << "\n";
            }
        }
        auto st = client.getStats();
        std::cout << "send rate(bps)=" << st.sendRateBps << ", recv rate(bps)=" << st.recvRateBps << "\r" << std::flush;
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
}


