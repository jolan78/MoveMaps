#include "GridMapManager.h"

namespace VMAP
{
  //================================================

  GridMapManager::GridMapManager (const char* pBasePath, unsigned int pMapId)
  {
    iBasePath = std::string (pBasePath);
    iMapId = pMapId;

    for (unsigned int idx = 0; idx < MAX_NUMBER_OF_GRIDS; ++idx)
      {
        for (unsigned int j = 0; j < MAX_NUMBER_OF_GRIDS; ++j)
          {
            iGridMaps[idx][j] = NULL;
            iMapExist[idx][j] = true;
          }
      }
  }

  //================================================

  GridMapManager::~GridMapManager () { }

  //================================================

  bool
  GridMapManager::loadMap (int pX, int pY)
  {
    bool result = true;
    if (!iGridMaps[pX][pY] && iMapExist[pX][pY])
      {
        result = false;
        int len = 500;
        char* tmp = new char[len];
        sprintf (tmp, (iBasePath + "/maps/%03u%02u%02u.map").c_str (), iMapId, pX, pY);
        // loading data
        FILE *pf = fopen (tmp, "rb");
        if (pf)
          {
            char magic[8];
            fread (magic, 1, 8, pf);
            GridMap * buf = new GridMap;
            fread (buf, 1, sizeof (GridMap), pf);
            fclose (pf);
            iGridMaps[pX][pY] = buf;
            result = true;
          }
        else
          iMapExist[pX][pY] = false;
        delete [] tmp;
      }
    return result;
  }

  //================================================

  void
  GridMapManager::unloadMap (int pX, int pY)
  {
    if (!iGridMaps[pX][pY])
      {
        delete iGridMaps[pX][pY];
        iGridMaps[pX][pY] = 0;
      }
  }

  //================================================

  float
  GridMapManager::getHeight (float pX, float pY)
  {

    float lx, ly;
    int gx, gy;

    gx = (int) (32 - pX / SIZE_OF_GRIDS); //grid x
    gy = (int) (32 - pY / SIZE_OF_GRIDS); //grid y

    if (!iGridMaps[gx][gy]) //this map is not loaded
      loadMap (gx, gy);

    if (!iMapExist[gx][gy])
      return -100000.0f;

    lx = MAP_RESOLUTION * (32 - pX / SIZE_OF_GRIDS - gx);
    ly = MAP_RESOLUTION * (32 - pY / SIZE_OF_GRIDS - gy);

    int lx_int = (int) lx;
    int ly_int = (int) ly;
    return iGridMaps[gx][gy]->Z[lx_int][ly_int];
  }
  //================================================
}
