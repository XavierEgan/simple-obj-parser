#include "Material.hpp"

objParser::Material::Material(const std::string& name) : name(name) {}

const std::string& objParser::Material::getName() const {
    return name;
}