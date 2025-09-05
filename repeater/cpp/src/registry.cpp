#include "repeater/server.hpp"

namespace repeater {

void FieldRegistry::register_getter(const std::string &fieldName, GetHandler getter) {
    getters_[fieldName] = std::move(getter);
}

void FieldRegistry::register_setter(const std::string &fieldName, SetHandler setter) {
    setters_[fieldName] = std::move(setter);
}

FieldRegistry::GetHandler FieldRegistry::find_getter(const std::string &fieldName) const {
    auto it = getters_.find(fieldName);
    if (it != getters_.end()) return it->second;
    return nullptr;
}

FieldRegistry::SetHandler FieldRegistry::find_setter(const std::string &fieldName) const {
    auto it = setters_.find(fieldName);
    if (it != setters_.end()) return it->second;
    return nullptr;
}

} // namespace repeater


