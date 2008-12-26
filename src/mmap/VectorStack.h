#ifndef _VECTORSTACK_H
#define _VECTORSTACK_H


#include <G3D/Vector3.h>
#include <G3D/Array.h>

using namespace G3D;

namespace VMAP
{
#define MAX_HIGHT 699
  //================================================

  class VectorStack : public Array<Vector3>
  {
  public:

    inline void
    pop_back ()
    {
      Array<Vector3>::pop_back ();
    }

    inline void
    push_back (const Vector3& pVal)
    {
      if (abs (pVal.y) < MAX_HIGHT)
        {
          Array<Vector3>::push_back (pVal);
        }
      //            //++ debug
      //            if(/*pVal.x == 15473.000 && pVal.z == 15083.000 && */pVal.y < -500) {
      //                int sss = 0;
      //            }
    }

    inline unsigned int
    size () const
    {
      return (Array<Vector3>::size ());
    }

    inline Vector3
    last ()
    {
      return (Array<Vector3>::last ());
    }
    //		inline void deleteAll() { Array<Vector3>::deleteAll(); }

    //inline Vector3& operator[](int n) { return( Array<Vector3>::operator[](n)); }
    //inline Vector3& operator[](unsigned int n) { return( Array<Vector3>::operator[](n)); }
  };

  //================================================
}

#endif
