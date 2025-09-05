#pragma once

#include <string>
#include <vector>
#include <cstdint>

#include "p2p/dht.hpp"
#include "p2p/router.hpp"
#include "store/store.hpp"

namespace p2p {

class DataApi {
public:
    DataApi(KademliaTable &dht, RouterTable &router, store::FileStore &store);

    std::vector<std::uint8_t> getData(const std::string &sid, int begin);
    int putData(const std::string &sid, const std::vector<std::uint8_t> &data);
    bool createData(const std::string &sid, const std::vector<std::uint8_t> &data);
    std::vector<std::uint8_t> findData(const std::string &sid);

private:
    KademliaTable &dht_;
    RouterTable &router_;
    store::FileStore &store_;
};

} // namespace p2p


