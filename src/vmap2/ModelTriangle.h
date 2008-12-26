#ifndef _VMAP_MODELTRIANGLE_H
#define _VMAP_MODELTRIANGLE_H

#include "G3D/platform.h"
#include "G3D/g3dmath.h"
#include "G3D/Vector3.h"
#include "G3D/debugAssert.h"
#include "G3D/AABox.h"

namespace VMAP2
{
  // A triangle used for our models
  class ModelTriangle
  {
  private:
    G3D::Vector3 _vertex[3];
    ModelTriangle& operator= (const ModelTriangle& other) { }
    
  public:
    ModelTriangle ();
    ModelTriangle (const ModelTriangle& m);
    ModelTriangle (class G3D::BinaryInput& b);
    ModelTriangle (const G3D::Vector3& v0,
                   const G3D::Vector3& v1,
                   const G3D::Vector3& v2);

    ~ModelTriangle ();

    void serialize (class G3D::BinaryOutput& b);
    void deserialize (class G3D::BinaryInput& b);

    const G3D::Vector3& vertex (int n) const;

    bool operator== (const ModelTriangle& other) const;

    size_t hashCode () const;
    void getBounds(G3D::AABox&) const;
  };

  inline
  ModelTriangle::~ModelTriangle () { }

  inline const G3D::Vector3&
  ModelTriangle::vertex (int n) const
  {
    debugAssert ((n >= 0) && (n < 3));
    return _vertex[n];
  }

  inline bool
  ModelTriangle::operator== (const ModelTriangle& other) const
  {
    for (int i = 0; i < 3; ++i)
      {
        if (_vertex[i] != other._vertex[i])
          {
            return false;
          }
      }

    return true;
  }

  inline size_t
  ModelTriangle::hashCode () const
  {
    return
    _vertex[0].hashCode () +
            (_vertex[1].hashCode () >> 2) +
            _vertex[2].hashCode ();
  }
}

#endif //_VMAP_MODELTRIANGLE_H
