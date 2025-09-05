#include "store/store.hpp"
#include <iostream>

int main() {
    store::FileStore fs("./data-store");
    fs.set_storage_limit(10 * 1024 * 1024);

    std::string sid = "example.bin";
    if (!fs.exists(sid)) {
        std::cout << "create: " << fs.create(sid) << "\n";
    }

    std::vector<std::uint8_t> payload = {1,2,3,4,5};
    std::cout << "append: " << fs.append(sid, payload) << "\n";
    auto content = fs.read(sid, 0, 5);
    std::cout << "read size: " << content.size() << ", used: " << fs.get_used_size() << "\n";
    std::cout << "remove: " << fs.remove(sid) << "\n";
    return 0;
}


