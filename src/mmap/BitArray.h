#ifndef _BITARRAY_H
#define _BITARRAY_H

#include "G3D/System.h"
#include <memory.h>

namespace VMAP
{
  //================================================

  //========================================================

  //typedef unsigned char mapStore_t;
  // has to be < sizeof(mapStore_t) and 2^x

  //========================================================

  template<class mapStore_t, unsigned int BITS_PER_MAP_VALUE>
  class BitArray
  {
  private:
    unsigned int iSizeX;
    unsigned int iArraySizeX;
    unsigned int iSizeY;
    unsigned int iOffset;
    unsigned int iMask;
    mapStore_t* iMap; // this might be only the relative address
    bool iOwnMemManagement;
  private:

    mapStore_t*
    getArray ()
    {
      return iMap;
    }

    void
    setMask ()
    {
      iMask = 1;
      for (int b = 0; b < (BITS_PER_MAP_VALUE - 1); ++b)
        {
          iMask <<= 1;
          iMask |= 1;
        }
    }

  public:

    BitArray ()
    {
      iMap = 0;
      iOwnMemManagement = false;
      iOffset = 0;
    }

    BitArray (unsigned int pSizeX, unsigned int pSizeY)
    {
      initArray (pSizeX, pSizeY);
    }

    void
    initArray (unsigned int pSizeX, unsigned int pSizeY)
    {
      iOffset = 0;
      iArraySizeX = (pSizeX + ((sizeof (mapStore_t)*8) / BITS_PER_MAP_VALUE) - 1) / ((sizeof (mapStore_t)*8) / BITS_PER_MAP_VALUE);
      iSizeX = pSizeX;
      iSizeY = pSizeY;
      iMap = new mapStore_t[iArraySizeX * iSizeY]; // we store multiple values in each map store
      memset (iMap, 0, iArraySizeX * iSizeY);
      iOwnMemManagement = true;
      setMask ();
    }

    ~BitArray ()
    {
      if (iMap != 0 && iOwnMemManagement) delete [] iMap;
    }

    unsigned int
    getArraySize () const
    {
      return (iArraySizeX * iSizeY);
    }

    unsigned int
    getSizeX () const
    {
      return iSizeX;
    }

    unsigned int
    getSizeY () const
    {
      return iSizeY;
    }

    unsigned int
    get (float pXPos, float pYPos) const
    {
      return (get (iMap, pXPos, pYPos));
    }

    unsigned int
    get (const mapStore_t* pMapBaseAdr, float pXPos, float pYPos) const
    {
      int x = (int) (pXPos);
      int y = (int) (pYPos);
      debugAssert (x >= 0 && x < (int) iSizeX && y >= 0 && y < (int) iSizeY);
      int pos = y * iArraySizeX + x / ((sizeof (mapStore_t)*8) / BITS_PER_MAP_VALUE);
      mapStore_t* map = (mapStore_t*) (((unsigned char*) pMapBaseAdr) + iOffset);
      if (sizeof (mapStore_t)*8 != BITS_PER_MAP_VALUE)
        {
          return ((map[pos] >> (x % ((sizeof (mapStore_t)*8) / BITS_PER_MAP_VALUE)) * BITS_PER_MAP_VALUE) & iMask);
        }
      else
        {
          return map[pos];
        }
    }

    void
    set (unsigned int pValue, float pXPos, float pYPos)
    {
      set (iMap, pValue, pXPos, pYPos);
    }

    void
    set (const mapStore_t* pMapBaseAdr, unsigned int pValue, float pXPos, float pYPos)
    {
      int x = (int) (pXPos);
      int y = (int) (pYPos);
      debugAssert (x >= 0 && x < (int) iSizeX && y >= 0 && y < (int) iSizeY);
      int pos = y * iArraySizeX + x / ((sizeof (mapStore_t)*8) / BITS_PER_MAP_VALUE);
      mapStore_t* map = (mapStore_t*) (((unsigned char*) pMapBaseAdr) + iOffset);
      if (sizeof (mapStore_t)*8 != BITS_PER_MAP_VALUE)
        {
          mapStore_t val = map[pos];
          unsigned int shifts = (x % ((sizeof (mapStore_t)*8) / BITS_PER_MAP_VALUE)) * BITS_PER_MAP_VALUE;
          mapStore_t notMask = ~(iMask << shifts);
          val &= notMask;
          val |= pValue << shifts;
          map[pos] = val;
        }
      else
        {
          map[pos] = pValue;
        }
    }

    //================================================

    unsigned int
    getMask ()
    {
      return iMask;
    }

    //================================================

    void
    fill (unsigned int pValue)
    {
      for (unsigned int x = 0; x < iSizeX; ++x)
        {
          for (unsigned int y = 0; y < iSizeY; ++y)
            {
              set (pValue, x, y);
            }
        }
    }
  };
  //================================================
}

#endif
