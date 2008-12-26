#ifndef _STEPDIRECTIONS_H
#define _STEPDIRECTIONS_H


#include <G3D/Vector3.h>

using namespace G3D;

namespace VMAP
{

  enum DIRS
  {
    D_SE,
    D_SW,
    D_S,
    D_NE,
    D_NW,
    D_N,
    D_E,
    D_W,
    D_SIZE,
  };

  //================================================
  extern const Vector3 wayChangesN[8];

  class StepDirections
  {
  public:

    const static Vector3&
    getDiffVector (int pDir)
    {
      return (wayChangesN[pDir]);
    }
  };
  //================================================
}

#endif /* _STEPDIRECTIONS_H */
