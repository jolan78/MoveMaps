#ifndef VMAP_MAP_ASSEMBLER_H
#define VMAP_MAP_ASSEMBLER_H

#include "VMap.h"

#include <G3D/Vector3.h>

#include <string>

#include "CoordModelMapping.h"

namespace VMAP2
{
  class MapAssembler
  {
  private:
    std::string iSrcDirName;
    std::string iDestDirName;
    
    // manages iSrcDirName/dir
    CoordModelMapping iCoordModelMapping;
    
    // for unique names
    G3D::Table<std::string, unsigned int > iUniqueNameIds;
    unsigned int iCurrentUniqueNameId;
  public:
    MapAssembler (const std::string& pSrcDirName, const std::string& pDestDirName);
    
    int ConvertWorld ();
    bool ProcessGrid (int mapId, int x, int y, VMap& map);

    VGrid* createVGrid (const G3D::Array<std::string>& pPosFileNames, const char* pDestFileName);
    void createModelsFromFile (std::string& pModelFilename, ModelPosition& pModelPosition, VGrid& grid);

    
    
    void addWorldAreaMapId(unsigned int pMapId) { iCoordModelMapping.addWorldAreaMap(pMapId); }
  };
}

#endif //VMAP_MAP_ASSEMBLER_H
