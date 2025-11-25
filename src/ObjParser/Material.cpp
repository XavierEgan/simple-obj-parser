#include "Material.hpp"

pt::Material::Material(const std::string& name) : name(name) {}

const std::string& pt::Material::getName() const {
    return name;
}