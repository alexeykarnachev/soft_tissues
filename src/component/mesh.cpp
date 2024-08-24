#include "mesh.hpp"

#include "entt/entity/fwd.hpp"

namespace soft_tissues::mesh {

MyMesh::MyMesh(entt::entity entity, std::string mesh_key, std::string material_pbr_key)
    : entity(entity)
    , mesh_key(mesh_key)
    , material_pbr_key(material_pbr_key)
    , constant_color(BLANK) {}

}  // namespace soft_tissues::mesh
