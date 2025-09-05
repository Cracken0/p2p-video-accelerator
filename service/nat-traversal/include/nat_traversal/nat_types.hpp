#pragma once

#include <string>

namespace nat {

enum class NatType {
    OpenInternet,
    FullCone,
    RestrictedCone,
    PortRestrictedCone,
    Symmetric,
    Unknown
};

inline std::string toString(NatType t) {
    switch (t) {
        case NatType::OpenInternet: return "OpenInternet";
        case NatType::FullCone: return "FullCone";
        case NatType::RestrictedCone: return "RestrictedCone";
        case NatType::PortRestrictedCone: return "PortRestrictedCone";
        case NatType::Symmetric: return "Symmetric";
        default: return "Unknown";
    }
}

struct Candidate {
    std::string ip;
    uint16_t port{0};
};

} // namespace nat







