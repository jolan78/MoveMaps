#ifndef _VMAP_MODEL_H
#define _VMAP_MODEL_H

#include "ModelTriangle.h"
#include "VTree.h"

namespace VMAP2
{

  class Model
  {
  public:
    typedef G3D::AABSPTree<ModelTriangle> TriangleTree;

  private:
    TriangleTree iTree;
    G3D::AABox iBounds;

  public:
    Model (class G3D::BinaryInput& b);
    Model (G3D::Array<G3D::Vector3>& vertexArray, G3D::Array<int>& indexArray);

    ~Model () { }

    void serialize (class G3D::BinaryOutput& b);
    void deserialize (class G3D::BinaryInput& b);

    bool operator== (const Model& other) const;

    size_t hashCode () const;

    void getBounds (G3D::AABox&) const;
    
    // used by viewer
    TriangleTree& getTree() { return iTree; }
  };

  inline
  Model::Model (class G3D::BinaryInput& b)
  {
    Model::deserialize (b);
  }

  inline void
  Model::getBounds (G3D::AABox& b) const
  {
    b = iBounds;
  }

  inline bool
  Model::operator== (const Model& other) const
  {
    return this == &other;
  }

  inline size_t
  Model::hashCode () const
  {
    return iBounds.hashCode ();
  }
}

#endif //_VMAP_MODEL_H
