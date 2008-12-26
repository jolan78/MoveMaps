#ifndef _MOVEMAPAREAFINDER_H
#define _MOVEMAPAREAFINDER_H

#include "PositionControlArray.h"

#include <G3D/Array.h>

using namespace G3D;

namespace VMAP
{

  //================================================

  class Range
  {
  public:
    int iLow;
    int iHigh;

    inline bool
    contains (int pVal)
    {
      return (iLow <= pVal && iHigh >= pVal);
    }

    inline int
    getDist ()
    {
      return (iHigh - iLow);
    }

    inline std::string
    toString ()
    {
      char buffer[100];
      sprintf (buffer, "low: %d, high: %d", iLow, iHigh);
      return (std::string (buffer));
    }
  };

  //================================================

  class HorizontalSplitter
  {
  private:
    Array<Range> iHeightBoxArray;
  private:
    bool
    getRangeOrNeighbour (int pValue, int& pLowIndex, int& pHighIndex);
  public:

    HorizontalSplitter () { }
    void insert (int* pValues, unsigned int pNValues);

    inline std::string
    toString ()
    {
      std::string result;
      for (int i = 0; i < iHeightBoxArray.size (); ++i)
        {
          result += iHeightBoxArray[i].toString () + std::string ("\n");
        }
      return result;
    }
  };
  //================================================

  class MoveMapAreaFinder
  {
  private:

  public:
    void findAreas (PositionControlArray<unsigned char, 2 > * pPositionControlArray);
  };

  //================================================
}

#endif
