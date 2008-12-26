#include "MoveMapBox.h"

namespace VMAP
{

  size_t
  hashCode (const MoveMapBox& pMM)
  {
    return (((MoveMapBox&) pMM).getAABoxBounds ().hashCode ());
  }

  //=============================================================

  size_t
  hashCode (const MoveMapBox* pMM)
  {
    return (((MoveMapBox*) pMM)->getAABoxBounds ().hashCode ());
  }

  //==========================================================

  bool MoveMapBox::operator== (const MoveMapBox& pMM2) const
  {
    bool result = false;

    if (getBounds () == pMM2.getBounds () &&
        getBasePosition () == pMM2.getBasePosition ())
      {
        result = true;
      }
    return result;
  }

  //=============================================================
  /*
      bool operator==(const MoveMapBox* pMM1, const MoveMapBox* pMM2) {
          return((*pMM1) == (*pMM2));

      }  
   */
  //=============================================================

  /*    bool operator==(const MoveMapBox& pMM1, const MoveMapBox& pMM2) {
          bool result = false;
          if(pMM1.getArraySize() ==  pMM2.getArraySize() && pMM1.getSizeX() == pMM2.getSizeX() && pMM1.getSizeY() == pMM2.getSizeY()) {
              if(pMM1.getAABoxBounds() == pMM2.getAABoxBounds()) {
                  result = true;
              }
          }
          return result;
      }
   */
  //=============================================================

  void
  getBounds (const MoveMapBox& pMM, G3D::AABox& pAABox)
  {
    pAABox = pMM.getAABoxBounds ();
  }

  //=============================================================

  void
  getBounds (const MoveMapBox* pMM, G3D::AABox& pAABox)
  {
    pAABox = pMM->getAABoxBounds ();
  }

  //=============================================================
  //==========  MoveMapBox ======================================
  //=============================================================

  MoveMapBox::MoveMapBox (unsigned int pSizeX, unsigned int pSizeY) : BitArray<unsigned char, 8 > (pSizeX, pSizeY) {
 }

  //=============================================================
  // Create the moveMapBox out of the MoveLayer

  MoveMapBox::MoveMapBox (MoveLayer* pMoveLayer) : BitArray<unsigned char, 8 > ()
  {
    pMoveLayer->getBounds (iBounds);
    float sizediff = iBounds.high ().x - iBounds.low ().x;
    if (sizediff > ((float) ((int) sizediff)))
      {
        sizediff += 1;
      }
    int sizeX = (int) sizediff;

    sizediff = iBounds.high ().z - iBounds.low ().z;
    if (sizediff > ((float) ((int) sizediff)))
      {
        sizediff += 1;
      }
    int sizeY = (int) sizediff;
    const PositionControlArray<unsigned short, 16 > * pcArray = pMoveLayer->getMovePointsArray ();
    initArray (sizeX, sizeY);
    Vector3 diffBaseV = iBounds.low () - pMoveLayer->getMovePointsArray ()->getBasePos ();
    float debugheight = 0;
    for (int y = 0; y < sizeY; ++y)
      {
        for (int x = 0; x < sizeX; ++x)
          {
            unsigned short val = pcArray->directGet (x + diffBaseV.x, y + diffBaseV.z, 0);
            if (val != MAP_VALUE_UNDEF && val != MAP_VALUE_CANT_REACH)
              {
                float height = SHORTHEIGHT2FLOAT (val);
                set (getCharHeight (height), x, y);
                debugheight = height;
              }
            else
              {
                //debugPrintf("MoveMapBox::MoveMapBox() out not reached x=%d, y=%d\n",x,y);

                //set(getCharHeight(debugheight), x,y);
                set (MOVEMAP_VALUE_CANT_REACH, x, y);
              }
          }
      }

  }

  //=============================================================

  MoveMapBox::~MoveMapBox () { }
}