#ifndef _VMAP_VGRID_H
#define _VMAP_VGRID_H

#include "VTree.h"
#include "Model.h"

#include <string>

namespace VMAP2
{
  class VGrid
  {
  private:
    // load/unload/save manage these
    ModelTree iTree;

    // serialize/deserialize manage these
    G3D::AABox iBounds;
    std::string iFileName;

    bool iLoaded;

  public:
    VGrid (class G3D::BinaryInput& b);
    ~VGrid ();

    void deserialize (class G3D::BinaryInput& b);

    bool operator== (const VGrid& other) const;

    size_t hashCode () const;

    void getBounds (G3D::AABox&) const;

    bool loaded () { return iLoaded; }
    bool load ();
    void unload ();
    
    // used at generation
    void serialize (class G3D::BinaryOutput& b) const;
    void save () const;
    void finish ();
    int size () const { return iTree.size (); }
    
    VGrid (const char* pDestFileName);
    void addModel (Model* new_model);
    
    // used by viewer
    ModelTree& getTree() { return iTree; }
  };
}

#endif //_VMAP_VGRID_H
