
#include "ModelTriangle.h"
#include "G3D/BinaryInput.h"
#include "G3D/BinaryOutput.h"

using namespace G3D;

namespace VMAP2
{
  
  ModelTriangle::ModelTriangle ()
  {
    _vertex[0] = Vector3::zero();
    _vertex[1] = Vector3::zero();
    _vertex[2] = Vector3::zero();
  }
  
  ModelTriangle::ModelTriangle (const G3D::Vector3& v0, const G3D::Vector3& v1, const G3D::Vector3& v2)
  {
    _vertex[0] = v0;
    _vertex[1] = v1;
    _vertex[2] = v2;
  }

  ModelTriangle::ModelTriangle (const ModelTriangle& m)
  {
    _vertex[0] = m._vertex[0];
    _vertex[1] = m._vertex[1];
    _vertex[2] = m._vertex[2];
  }

  ModelTriangle::ModelTriangle (class G3D::BinaryInput& b)
  {
    deserialize (b);
  }

  void
  ModelTriangle::serialize (class G3D::BinaryOutput& b)
  {
    _vertex[0].serialize (b);
    _vertex[1].serialize (b);
    _vertex[2].serialize (b);
  }

  void
  ModelTriangle::deserialize (class G3D::BinaryInput& b)
  {
    _vertex[0].deserialize (b);
    _vertex[1].deserialize (b);
    _vertex[2].deserialize (b);
  }

  void
  ModelTriangle::getBounds (G3D::AABox& out) const
  {
    Vector3 lo = _vertex[0];
    Vector3 hi = lo;

    for (int i = 1; i < 3; ++i)
      {
        lo = lo.min (_vertex[i]);
        hi = hi.max (_vertex[i]);
      }

    out = AABox (lo, hi);
  }
}


void
getBounds (const VMAP2::ModelTriangle& m,G3D::AABox& b)
{
  m.getBounds (b);
}

