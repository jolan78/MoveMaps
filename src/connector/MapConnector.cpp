
#include "G3D/Array.h"


#include "G3D/Box.h"


#include "G3D/Array.h"

#include "MapConnector.h"
#include "Config/Config.h"

using namespace G3D;

namespace VMAP
{
  extern std::string startCoordsPath;
  extern std::string gDataDir;
  extern std::string gMMapDataDir;
//  extern std::string gvMapDataDir;
  extern std::string g3dDataDir;


  //==========================================

  MapConnector::MapConnector (int mapId, int ax, int ay, int bx, int by) :
  iVMapManager ()
  {
    iMap = mapId;
    
    // A must be < B
    if (ax>bx)
      {
      int tmp=ax;
      ax=bx;
      bx=tmp;
      }
    if (ay>by)
      {
      int tmp=ay;
      ay=by;
      by=tmp;
      }
    if (abs(ax-bx) + abs(ay-by) != 1)
      {
      printf("the two grids must be contigous\n");
      exit(1);
      }
    if (ax==bx)
      vertical=true;
    else
      vertical=false;
    
    iax = ax;
    iay = ay;
    ibx = bx;
    iby = by;
  }
  //===================================================

  MapConnector::~MapConnector (void)
  {
  }

  //===================================================
  
  void
  MapConnector::run ()
  {
    MoveMapContainer moveMapBoxContainerA;
    MoveMapContainer moveMapBoxContainerB;
    Table<unsigned int, Table<unsigned int,float> > connexionPtsAtoB;
    Table<unsigned int, Table<unsigned int,float> > connexionPtsBtoA;
    /*Table<Vector2, unsigned int> landingPointsMZA;
    Table<Vector2,float> landingPointsDHeightA;
    Table<Vector2, unsigned int> landingPointsMZB;
    Table<Vector2,float> landingPointsDHeightB;*/

    char buffer[50];
    
    sprintf (buffer, "%03u_%02u_%02u", iMap, iax, iay);
    moveMapBoxContainerA.load (gMMapDataDir.c_str(),buffer);
    MoveZoneContainer* iMoveZoneContainerA = moveMapBoxContainerA.getMoveZoneContainer();
    sprintf (buffer, "%03u_%02u_%02u", iMap, ibx, iby);
    moveMapBoxContainerB.load (gMMapDataDir.c_str(),buffer);
    MoveZoneContainer* iMoveZoneContainerB = moveMapBoxContainerB.getMoveZoneContainer();
    
    FILE *CnxPts;
    int bufferSize = 500;
    char readBuffer[500];

    unsigned int mzID, portalID;
    float z;
 
    sprintf (buffer, "%s/grid_cnx_%03u_%02u_%02u_%02u_%02u.tmp", startCoordsPath.c_str(),iMap, iax, iay, ibx, iby);
    CnxPts = fopen (buffer, "rb");
    while (fgets (readBuffer, bufferSize - 1, CnxPts))
      {
      sscanf (readBuffer, "%u,%u,%f", &mzID, &portalID, &z);
      Table<unsigned int,float> vals;
      if (connexionPtsAtoB.get(mzID,vals))
        {
        vals.set(portalID,z);
        }
      else
        {
        vals.set(portalID,z);
        connexionPtsAtoB.set(mzID,vals);
        }
      }
    fclose (CnxPts);

    
    sprintf (buffer, "%s/grid_cnx_%03u_%02u_%02u_%02u_%02u.tmp", startCoordsPath.c_str(),iMap, ibx, iby, iax, iay);
    CnxPts = fopen (buffer, "rb");
    while (fgets (readBuffer, bufferSize - 1, CnxPts))
      {
      sscanf (readBuffer, "%u,%u,%f", &mzID, &portalID, &z);
      Table<unsigned int,float> vals;
      if (connexionPtsBtoA.get(mzID,vals))
        {
        vals.set(portalID,z);
        }
      else
        {
        vals.set(portalID,z);
        connexionPtsBtoA.set(mzID,vals);
        }
      }
    fclose (CnxPts);
    
    unsigned short direction=(vertical?EXTEND_N:EXTEND_E);
    connect(connexionPtsAtoB,iMoveZoneContainerA, iMoveZoneContainerB, direction,ibx,iby);
    Vector3 dummy; // TODO: overload MoveMapBoxContainer::save
    moveMapBoxContainerA.save (gMMapDataDir.c_str (), dummy, dummy, dummy, dummy, iMap, iax, iay, false);

    direction=(vertical?EXTEND_S:EXTEND_W);
    connect(connexionPtsBtoA,iMoveZoneContainerB,iMoveZoneContainerA, direction,iax, iay);
    moveMapBoxContainerB.save (gMMapDataDir.c_str (), dummy, dummy, dummy, dummy, iMap, ibx, iby, false);
  }

  void
  MapConnector::connect (Table<unsigned int, Table<unsigned int,float> > connexionPts,MoveZoneContainer* fromMoveZoneContainer,MoveZoneContainer* destMoveZoneContainer, unsigned short direction, int dx, int dy)
  {
    Array<unsigned int> MZArray = connexionPts.getKeys();
    MZArray.sort();
    MoveZone* destMZ=NULL;
    MoveZone* prevdestMZ=NULL;
    MovePortal* prevMP=NULL;
    Vector2 prevTestVector2;
    for (unsigned int i=0; i<MZArray.size(); ++i)
      {
      const MoveZone* fromMZ=fromMoveZoneContainer->getZone(MZArray[i]);
      Array<MovePortal*> fromPortalArray=fromMZ->getPortalArray();
      Array<unsigned int> fromPortalidArray = connexionPts[MZArray[i]].getKeys();
      fromPortalidArray.sort();
      for(unsigned int j=0;j < fromPortalidArray.size(); ++j)
        {
        MovePortal* MP=fromPortalArray[fromPortalidArray[j]];
        Vector2 testVector2=MP->getLow2();
        float z=connexionPts[MZArray[i]][fromPortalidArray[j]];
        switch (direction)
          {
          case EXTEND_N :
            testVector2.x+=1;
          break;
          case EXTEND_S :
            testVector2.x-=1;
          break;
          case EXTEND_E :
            testVector2.y+=1;
          break;
          case EXTEND_W :
            testVector2.y-=1;
          break;
          
          }
        
        printf("at %f,%f\n",testVector2.x,testVector2.y);
        destMZ=NULL;
        Array<MoveZone*> matchMZArray=destMoveZoneContainer->getMoveZonesByZRange(testVector2.x,testVector2.y,z - 0.5, z + 0.5); // z can be slightly different
        if (matchMZArray.size() == 0)
          {
          printf("no match\n");
          prevMP=NULL;
          }
        else if (matchMZArray.size() > 1)
          {
          printf("%u matches\n",matchMZArray.size() );
          prevMP=NULL;
          }
        else
          {
          printf("one match\n");
          destMZ=matchMZArray[i];
          if (prevdestMZ == destMZ && prevMP && (prevTestVector2 - testVector2).length() <= 1)
            {
            // join portals
            prevMP->extend();
            fromPortalArray.remove(fromPortalidArray[j]);
            }
          else
            {
            MP->setDestGrid(dx,dy);
            }
          
          prevMP=MP;
          }
        prevTestVector2=testVector2;
        }
      prevdestMZ=destMZ;
      }
  }

}

