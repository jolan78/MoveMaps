#ifndef _VMAP_VMAP_H_
#define _VMAP_VMAP_H_

#include "VTree.h"
#include "VGrid.h"

namespace VMAP2
{
  class VMap
  {
  private:
    VGridTree iTree;

    int iMapId;
  public:
    VMap (int id);
    ~VMap ();

    /// note that load ,will only load 
    /// the structure ,not the 3d data
    bool load ();

    /// Used when generating
    void addGrid (VGrid* new_grid);

    void commit ();
  };
}
#endif //_VMAP_VMAP_H_
