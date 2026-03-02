#include "mesh.hpp"

#include "serializers.hpp"

namespace soft_tissues::component {

MyMesh::MyMesh(std::string mesh_key, std::string material_pbr_key)
    : mesh_key(std::move(mesh_key))
    , material_pbr_key(std::move(material_pbr_key))
    , constant_color(BLANK) {}

nlohmann::json MyMesh::to_json() const {
    nlohmann::json json;

    json["mesh_key"] = this->mesh_key;
    json["material_pbr_key"] = this->material_pbr_key;
    json["constant_color"] = this->constant_color;

    return json;
}

MyMesh MyMesh::from_json(const nlohmann::json &json_data) {
    std::string mesh_key = json_data["mesh_key"].get<std::string>();
    std::string material_pbr_key = json_data["material_pbr_key"].get<std::string>();

    MyMesh mesh(mesh_key, material_pbr_key);
    mesh.constant_color = json_data["constant_color"].get<Color>();

    return mesh;
}

}  // namespace soft_tissues::component
