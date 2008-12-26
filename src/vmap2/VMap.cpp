
#include "G3D/BinaryOutput.h"


#include "VMap.h"
#include "G3D/format.h"

using namespace G3D;

namespace VMAP2
{
  extern std::string VMAPDirPrefix;
  const char VMAPMagic[] = { 'V','M','A','P','0','0','3','\0'} ;

  VMap::VMap (int id)
  {
    iMapId = id;
  }

  VMap::~VMap ()
  {
    if (iTree.size () > 0)
      {
        for (VGridTree::Iterator iter = iTree.begin (); iter != iTree.end (); ++iter)
          delete iter->value;

        iTree.clear ();
      }
  }

  bool
  VMap::load ()
  {
    return false;
  }

  void
  VMap::addGrid (VGrid* new_grid)
  {
    iTree.insert (VGridPtr (new_grid));
  }

  void
  VMap::commit ()
  {
    iTree.balance (0,0);

    std::string fileName = G3D::format ("%s%04u.vtr", VMAPDirPrefix.c_str (), iMapId);

    BinaryOutput b (fileName.c_str (), G3D_LITTLE_ENDIAN);

    b.writeBytes (VMAPMagic, sizeof (VMAPMagic));

    iTree.serializeStructure (b);

    for (VGridTree::Iterator it = iTree.begin (); it != iTree.end (); ++it)
      {
        it->value->serialize (b);
      }

    b.commit ();
  }
}


