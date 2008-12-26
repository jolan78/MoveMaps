#ifndef _GRIDMAPMANAGER_H
#define _GRIDMAPMANAGER_H

#include <string>

namespace VMAP
{

#define MAX_NUMBER_OF_GRIDS      64

#define SIZE_OF_GRIDS            533.33333f
#define CENTER_GRID_ID           (MAX_NUMBER_OF_GRIDS/2)

#define CENTER_GRID_OFFSET      (SIZE_OF_GRIDS/2)

#define MAX_NUMBER_OF_CELLS     8
#define SIZE_OF_GRID_CELL       (SIZE_OF_GRIDS/MAX_NUMBER_OF_CELLS)

#define CENTER_GRID_CELL_ID     256
#define CENTER_GRID_CELL_OFFSET (SIZE_OF_GRID_CELL/2)

#define TOTAL_NUMBER_OF_CELLS_PER_MAP    (MAX_NUMBER_OF_GRIDS*MAX_NUMBER_OF_CELLS)

#define MAP_RESOLUTION 256

#define MAP_SIZE                (SIZE_OF_GRIDS*MAX_NUMBER_OF_GRIDS)
#define MAP_HALFSIZE            (MAP_SIZE/2)


  //================================================

  typedef struct
  {
    unsigned short area_flag[16][16];
    unsigned char terrain_type[16][16];
    float liquid_level[128][128];
    float Z[MAP_RESOLUTION][MAP_RESOLUTION];
  } GridMap;

  //================================================

  class GridMapManager
  {
  private:
    GridMap *iGridMaps[MAX_NUMBER_OF_GRIDS][MAX_NUMBER_OF_GRIDS];
    bool iMapExist[MAX_NUMBER_OF_GRIDS][MAX_NUMBER_OF_GRIDS];
    std::string iBasePath;
    unsigned int iMapId;
  public:
    GridMapManager (const char* pBasePath, unsigned int pMapId);
    ~GridMapManager ();

    bool loadMap (int pX, int pY);
    void unloadMap (int pX, int pY);
    float getHeight (float pX, float pY);
  };


  //================================================
}

#endif
