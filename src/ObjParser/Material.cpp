#include "Material.hpp"

objParser::Material(const std::string& name) : name(name) {}

const std::string& objParser::Material::getName() const {
    return name;
}