#ifndef _MOVEMAPBOX_H_
#define _MOVEMAPBOX_H_

#include <G3D/AABox.h>

#include "BitArray.h"
#include "MoveLayer.h"

using namespace G3D;

namespace VMAP
{
  //========================================================
  /*
      enum FIELDVALUES {
          BLOCKED = 0,
          ACCESSABLE,
          CONNECTION, 		// can access and has connection to the map upwards or downwards
      };
   */

#define MOVEMAP_VALUE_CANT_REACH 0xff

  class MoveMapBox : public BitArray<unsigned char, 8 >
  {
  private:
    AABox iBounds;
    //		short *iHeightArray;
    //        Vector3 iBasePos;
  public:

    MoveMapBox () { };
    MoveMapBox (unsigned int pSizeX, unsigned int pSizeY);
    MoveMapBox (MoveLayer* pMoveLayer);
    ~MoveMapBox ();

    const AABox&
    getBounds () const
    {
      return iBounds;
    }

    const AABox&
    getAABoxBounds () const
    {
      return iBounds;
    }

    void
    setBounds (const AABox& pBox)
    {
      iBounds = pBox;
    }

    void
    setBounds (const Vector3& pLo, const Vector3& pHi)
    {
      iBounds.set (pLo, pHi);
    }

    const Vector3&
    getBasePosition () const
    {
      return iBounds.low ();
    }

    bool
    contains (const Vector3& pPos) const
    {
      return iBounds.contains (pPos);
    }

    inline unsigned char
    getCharHeight (float pHeight) const
    {
      float diff = pHeight - iBounds.low ().y;
      unsigned char result = (unsigned char) ((diff * 4) + 0.5f);
      debugAssert (result != MOVEMAP_VALUE_CANT_REACH);
      return result;
    }

    inline float
    getFloatHeight (unsigned char pHeight) const
    {
      float result = iBounds.low ().y + (((float) pHeight) / 4);
      return result;
    }

    bool operator== (const MoveMapBox& pMM2) const;

    size_t
    hashCode ()
    {
      return (getAABoxBounds ().hashCode ());
    }
  };

  //========================================================

  class RawMoveMapDestHandle
  {
  public:
    Vector3 iSourcePosition;
    Vector3 iDestPosition;
    MoveMapBox *iSourceBox;
    MoveMapBox *iDestBox;
  };


  //========================================================

  size_t hashCode (const MoveMapBox& pMM);
  //bool operator==(const MoveMapBox& pMM1, const MoveMapBox& pMM2);
  size_t hashCode (const MoveMapBox* pMM);
  void getBounds (const MoveMapBox& pMM, G3D::AABox& pAABox);
  void getBounds (const MoveMapBox* pMM, G3D::AABox& pAABox);
}


#endif /*_MOVEMAPBOX_H_*/
