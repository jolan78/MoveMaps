
#include "G3D/Vector3.h"
#include "G3D/vectorMath.h"
#include "Model.h"
#include "G3D/BinaryInput.h"
#include "G3D/BinaryOutput.h"

using namespace G3D;

namespace VMAP2
{
  void Model::serialize (class G3D::BinaryOutput& b)
  {     
    b.writeInt32 (iTree.size ());
    
    iTree.serializeStructure (b);
    
    for (TriangleTree::Iterator it = iTree.begin (); it != iTree.end (); ++it)
      it->serialize (b);
    
    iBounds.serialize (b);
  }
  
  void Model::deserialize (class G3D::BinaryInput& b)
  {
    int32 size = b.readInt32 ();
    
    iTree.deserializeStructure (b);
    
    for (int32 i = 0; i < size; i++)
      iTree.insert (ModelTriangle (b));
    
    iBounds.deserialize (b);
  }

  Model::Model (G3D::Array<G3D::Vector3>& vertexArray, G3D::Array<int>& indexArray)
  {
    // CALC BSPTree
    int nindexes = indexArray.size ();

    // It needs to be checked before creating the model
    debugAssert (nindexes >= 3);

    for (int i = 0; i < nindexes; i += 3)
      {
        iTree.insert (ModelTriangle (
                                     vertexArray[indexArray[i + 2]],
                                     vertexArray[indexArray[i + 1]],
                                     vertexArray[indexArray[i + 0]]
                                     ));
      }

    iTree.balance (0, 0);

    iBounds.set (Vector3::inf (), Vector3::zero () - Vector3::inf ());

    debugAssert (iTree.size () > 0);

    // CALC bounds
    for (TriangleTree::Iterator iter = iTree.begin (); iter != iTree.end (); ++iter)
      {
        AABox box;
        iter->getBounds (box);
        iBounds.set (iBounds.low ().min (box.low ()), iBounds.high ().max (box.high ()));
      }

    debugAssert (iBounds.low ().x < iBounds.high ().x);
    debugAssert (iBounds.low ().y < iBounds.high ().y);
    debugAssert (iBounds.low ().z < iBounds.high ().z); 
  }
}

void
getBounds (const VMAP2::ModelPtr& m,G3D::AABox& b)
{
  m.value->getBounds (b);
}
