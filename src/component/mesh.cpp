#include "mesh.hpp"

#include "entt/entity/fwd.hpp"

namespace soft_tissues::mesh {

MyMesh::MyMesh(entt::entity entity, Mesh mesh, pbr::MaterialPBR material)
    : entity(entity)
    , mesh(mesh)
    , material(material)
    , constant_color(BLANK) {}

}  // namespace soft_tissues::mesh
