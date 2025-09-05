#include "tcp_like/reliable_session.hpp"
#include <iostream>
#include <thread>

int main() {
    tcp_like::ReliableSession server;
    tcp_like::Endpoint local{"0.0.0.0", 50000};
    tcp_like::Endpoint clientHint{"127.0.0.1", 50001}; // demo peer (acceptFrom)
    if (!server.listen(local)) {
        std::cerr << "listen failed\n";
        return 1;
    }
    server.acceptFrom(clientHint);
    std::cout << "echo_server waiting for handshake...\n";

    for (;;) {
        server.update();
        if (server.isEstablished()) {
            std::vector<uint8_t> payload;
            while (server.receive(payload)) {
                server.send(payload);
            }
        }
        auto st = server.getStats();
        std::cout << "send rate(bps)=" << st.sendRateBps << ", recv rate(bps)=" << st.recvRateBps << "\r" << std::flush;
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}


