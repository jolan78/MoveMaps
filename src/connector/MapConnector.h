#ifndef _MAPCONNECTOR_H
#define _MAPCONNECTOR_H

#include "VMapManager.h"
#include <G3D/G3DAll.h>
#include "ModelContainer.h"
#include "MoveMapBoxContainer.h"
#include "MoveZone.h"

namespace VMAP
{
  //==========================================
  class MapConnector
  {
  private:    
    VARAreaRef iVARAreaRef;
    
    VMapManager iVMapManager;
    
    const MoveZoneContainer* moveMapBoxContainerA;
    const MoveZoneContainer* moveMapBoxContainerB;
    
    int iMap;
    int iax;
    int iay;
    int ibx;
    int iby;
    bool vertical;
  private:
    void connect (Table<unsigned int, Table<Vector2,float> > connexionPts,MoveZoneContainer* fromMoveZoneContainer,MoveZoneContainer* destMoveZoneContainer, unsigned short direction, int dx, int dy);

  public:
    MapConnector (int mapId, int ax, int ay, int bx, int by);
    ~MapConnector (void);

    void run();

  };

  //==========================================
}

#endif
