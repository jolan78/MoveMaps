#ifndef VMAP_VTREE_H
#define VMAP_VTREE_H

#include "G3D/AABox.h"

namespace VMAP2
{
  // template of a pointer to 
  // class that we can insert in the bpstree
  template<class Pointer>
  class AAPtr
  {
  public:
    AAPtr ()
    {
      throw 1;
    }
    
    AAPtr (Pointer g) : value (g) { }

    inline bool
    operator== (const AAPtr& other) const
    {
      return *(this->value) == *(other.value);
    }

    inline size_t
    hashCode () const
    {
      return value->hashCode ();
    }

    Pointer value;
  };

  class Model;
  typedef AAPtr<Model*> ModelPtr;
  
  class VGrid;
  typedef AAPtr<VGrid*> VGridPtr;
  
  class ModelTriangle;
}

void getBounds (const VMAP2::ModelPtr& m, G3D::AABox& b);
void getBounds (const VMAP2::VGridPtr& m, G3D::AABox& b);
void getBounds (const VMAP2::ModelTriangle& m,G3D::AABox& b);

#include "G3D/AABSPTree.h"

namespace VMAP2
{
  typedef G3D::AABSPTree<ModelPtr> ModelTree;
  typedef G3D::AABSPTree<VGridPtr> VGridTree;
}


#endif //VMAP_VTREE_H
