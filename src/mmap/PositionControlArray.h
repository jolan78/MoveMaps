#ifndef _POSITIONCONTROLARRAY_H
#define _POSITIONCONTROLARRAY_H

#include "BitArray.h"
#include <G3D/Vector3.h>
#include <G3D/AABox.h>

using namespace G3D;

namespace VMAP
{

  /**
  Definition for the 2 bit array to define which positions are reached and which can not be reached
   */

  enum POSITIONS_VALUES
  {
    POS_UNDEF,
    POS_REACHED,
    POS_NOT_REACHED,
    POS_CAN_REACH,
  };

  template<class STORE_T, unsigned int BITS_PER_MAP_VALUE>
  class PositionControlArray
  {
  private:
    unsigned int iSizeX;
    unsigned int iSizeY;
    //unsigned int iMaxSizeHeight;
    //unsigned int iCurrentHeight;
    unsigned int iDownSize;
    unsigned int iUpSize;
    unsigned int iFillValue;
    float iStepwidth;
    Array<BitArray<STORE_T, BITS_PER_MAP_VALUE>*> iBitArrays;
    Vector3 iBasePos;
    AABox iBounds;
  private:

    void
    extendArray (int height)
    {
      if (height < 0)
        {
          //insert new layer arrays to the beginning 
          for (int i = 0; i < (-height); ++i)
            {
              BitArray<STORE_T, BITS_PER_MAP_VALUE>* newArray = new BitArray<STORE_T, BITS_PER_MAP_VALUE > (iSizeX, iSizeY);
              if (iFillValue != 0)
                {
                  newArray->fill (iFillValue);
                }
              iBitArrays.insert (0, newArray);
              iDownSize += iStepwidth;
            }
          Vector3 bp (getBasePos ().x, getBasePos ().y, getBasePos ().z);
          iBounds.set (bp - Vector3 (0, iDownSize, 0)
                       , bp + Vector3 (iSizeX, iUpSize, iSizeY));
        }
      if (height >= (int) getHeightArraySize ())
        {
          //insert new layer arrays to the end 
          int heightDiff = height - getHeightArraySize () + 1;
          for (int i = 0; i < heightDiff; ++i)
            {
              BitArray<STORE_T, BITS_PER_MAP_VALUE>* newArray = new BitArray<STORE_T, BITS_PER_MAP_VALUE > (iSizeX, iSizeY);
              if (iFillValue != 0)
                {
                  newArray->fill (iFillValue);
                }
              iBitArrays.push_back (newArray);
              iUpSize += iStepwidth;
            }
          Vector3 bp (getBasePos ().x, getBasePos ().y, getBasePos ().z);
          iBounds.set (bp - Vector3 (0, iDownSize, 0)
                       , bp + Vector3 (iSizeX, iUpSize, iSizeY));
        }
    }

  public:

    PositionControlArray (const Vector3& pBasePos, unsigned int pSizeX, unsigned int pSizeY, int pStepwidth)
    {
      iBasePos = pBasePos;
      iSizeX = pSizeX;
      iSizeY = pSizeY;
      //iCurrentHeight = pStepwidth;
      iDownSize = 0;
      iUpSize = pStepwidth;
      iStepwidth = pStepwidth;
      iFillValue = 0;

      iBitArrays.push_back (new BitArray<STORE_T, BITS_PER_MAP_VALUE > (pSizeX, pSizeY));

      Vector3 bp (pBasePos.x, pBasePos.z, pBasePos.z);
      iBounds.set (bp - Vector3 (0, iDownSize, 0)
                   , bp + Vector3 (pSizeX, iUpSize, pSizeY));
    }

    //=========================================================

    ~PositionControlArray ()
    {
      // Never called
      for (unsigned int i = 0; i < getHeightArraySize (); ++i)
        {
          delete iBitArrays[i];
        }
    }

    inline unsigned int
    getSizeX () const
    {
      return iSizeX;
    }

    inline unsigned int
    getSizeY () const
    {
      return iSizeY;
    }
    // single distance from the center in z dimension
    //inline unsigned int getSizeHeight() const { return iSizeHeight; }
    // return how many z array we have

    inline unsigned int
    getHeightArraySize () const
    {
      float layers = (float) (iDownSize + iUpSize) / (float) iStepwidth;
      unsigned int result = layers;
      if (layers > (float) result)
        {
          ++result;
        }
      if (result == 0)
        {
          result = 1;
        }
      return result;
    }

    inline const Vector3&
    getBasePos () const
    {
      return (iBasePos);
    }

    inline float
    getStepwidth () const
    {
      return iStepwidth;
    }

    inline const AABox&
    getBounds () const
    {
      return (iBounds);
    }

    inline bool
    contains (const Vector3& pPos) const
    {
      return iBounds.contains (pPos);
    }

    inline int
    getHeightValueFromVector (const Vector3& pPos) const
    {
      return ((pPos.y - getBasePos ().y + iStepwidth / 2.0f) / iStepwidth + iDownSize / iStepwidth);
    }
    //=========================================================

    unsigned int
    get (const Vector3& pPos)
    {
      int height = getHeightValueFromVector (pPos);
      extendArray (height);
      if (height < 0)
        {
          //printf("Height was %d returning 0\n",height);
          return 0;
        }
      debugAssert (height >= 0 && (unsigned int) height < getHeightArraySize ());
      //printf("At %f, %f heigh stored is %u\n",pPos.x-iBasePos.x, pPos.z-iBasePos.z,iBitArrays[height]->get(pPos.x-iBasePos.x, pPos.z-iBasePos.z));
      return (iBitArrays[height]->get (pPos.x - iBasePos.x, pPos.z - iBasePos.z));
    }
    /*    enum POSITIONS_VALUES {
            POS_UNDEF,
            POS_REACHED,
            POS_NOT_REACHED,
            POS_CAN_REACH,
        };*/

    //=========================================================

    unsigned int
    directGet (int x, int y, int height) const
    {
      return (iBitArrays[height]->get ((float) x, (float) y));
    }

    //=========================================================

    void
    set (unsigned int pValue, const Vector3& pPos)
    {
      int height = getHeightValueFromVector (pPos);
      extendArray (height);
      height = getHeightValueFromVector (pPos);
      debugAssert (height >= 0 && (unsigned int) height < getHeightArraySize ());
      iBitArrays[height]->set (pValue, pPos.x - iBasePos.x, pPos.z - iBasePos.z);
    }

    //=========================================================

    void
    add (int pValue, const Vector3& pPos)
    {
      int height = getHeightValueFromVector (pPos);
      debugAssert (height >= 0 && (unsigned int) height < getHeightArraySize ());
      int x, z;
      x = pPos.x - iBasePos.x;
      z = pPos.z - iBasePos.z;
      int val = (int) iBitArrays[height]->get (x, z);
      val += pValue;
      debugAssert (val >= 0);
      iBitArrays[height]->set (val, x, z);
    }


    //=========================================================

    void
    fill (unsigned int pValue)
    {
      iFillValue = pValue;
      unsigned int sz = getHeightArraySize ();
      for (unsigned int height = 0; height < sz; ++height)
        {
          iBitArrays[height]->fill (pValue);
        }
    }
    //=========================================================

    unsigned int
    getMaxValue ()
    {
      return (iBitArrays[0]->getMask ());
    }

    //=========================================================
    /**
    Search fo the given value in the entire z dimension for the given x,z
    If the value is found, write the z position to the result array
     */
    //        void fillPositionZArray(unsigned int pX, unsigned int pY, unsigned int pCompareValue, int *pPositionHeightArray, int& nElements) {
    //            nElements = 0;
    //            for(unsigned int height=0; height<getHeightArraySize(); ++height) {
    //                if(iBitArrays[height]->get(pX, pY) == pCompareValue) {
    //                    pPositionHeightArray[nElements] = height - iSizeHeight/iStepwidth;;
    //                    nElements++;
    //                }
    //            }
    //        }

  };

}

#endif
