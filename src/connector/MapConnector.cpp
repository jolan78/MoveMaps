
#include "G3D/Array.h"


#include "G3D/Box.h"


#include "G3D/Array.h"

#include "MapConnector.h"
#include "Config/Config.h"

using namespace G3D;

namespace VMAP
{

  // to sort Array<MovePortal*>
/*  static bool MovePortalLT(MovePortal*const& elem1, MovePortal*const& elem2)
    {
    return elem1->getLow2() < elem2->getLow2();
    }
*/

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
      vertical=false;
    else
      vertical=true;
    
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
 printf ("load grid_cnx_%03u_%02u_%02u_%02u_%02u\n",iMap, iax, iay, ibx, iby);
    sprintf (buffer, "%s/grid_cnx_%03u_%02u_%02u_%02u_%02u.tmp", startCoordsPath.c_str(),iMap, iax, iay, ibx, iby);
    CnxPts = fopen (buffer, "rb");
    while (fgets (readBuffer, bufferSize - 1, CnxPts))
      {
      sscanf (readBuffer, "%u,%u,%f", &mzID, &portalID, &z);
      Table<unsigned int,float> vals;
      connexionPtsAtoB.get(mzID,vals);
      vals.set(portalID,z);
      connexionPtsAtoB.set(mzID,vals);
      }
    fclose (CnxPts);

 printf ("load grid_cnx_%03u_%02u_%02u_%02u_%02u\n",iMap, ibx, iby, iax, iay);
    sprintf (buffer, "%s/grid_cnx_%03u_%02u_%02u_%02u_%02u.tmp", startCoordsPath.c_str(),iMap, ibx, iby, iax, iay);
    CnxPts = fopen (buffer, "rb");
    while (fgets (readBuffer, bufferSize - 1, CnxPts))
      {
      sscanf (readBuffer, "%u,%u,%f", &mzID, &portalID, &z);
      Table<unsigned int,float> vals;
      connexionPtsBtoA.get(mzID,vals);
      vals.set(portalID,z);
      connexionPtsBtoA.set(mzID,vals);
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
      printf("in movezone %d : %u portals\n",fromMZ->getIndex(),fromPortalArray.size());
      
      
      /*fromPortalArray.sort(MovePortalLT);
      for(unsigned int j=0;j < fromPortalArray.size(); ++j)
        {
         MovePortal* MP=fromPortalArray[j];

        if (MP->getDirection() == direction)
          {
          Vector2 testVector2=MP->getLow2();
          float z=connexionPts[MZArray[i]][fromPortalidArray[j]];
          printf("from %f,%f (%f) portalid %u\n",testVector2.x,testVector2.y,z,fromPortalidArray[j]);
          switch (direction)
            {
            case EXTEND_N :
              printf("N\n");
              testVector2.y+=1;
            break;
            case EXTEND_S :
              printf("S\n");
              testVector2.y-=1;
            break;
            case EXTEND_E :
              printf("E\n");
              testVector2.x+=1;
            break;
            case EXTEND_W :
              printf("W\n");
             testVector2.x-=1;
            break;
            
            }
          
          printf("to %f,%f (%f)\n",testVector2.x,testVector2.y,z);
          destMZ=NULL;
          Array<MoveZone*> matchMZArray=destMoveZoneContainer->getMoveZonesByZRange(testVector2.x,testVector2.y,z - 0.5, z + 0.5); // z can be slightly different
          if (matchMZArray.size() == 0)
            {
            printf("no match\n");
            prevMP=NULL;
            destMZ=NULL;
            }
          else if (matchMZArray.size() > 1)
            {
            printf("%u matches\n",matchMZArray.size() );
            for (unsigned int k = 0 ; k< matchMZArray.size(); ++k)
              printf("zone id %u\n",matchMZArray[k]->getIndex());
            prevMP=NULL;
            destMZ=NULL;
            }
          else
            {
            destMZ=matchMZArray[0];
            printf("one match : %u\n",destMZ->getIndex());
  
            if (prevdestMZ == destMZ && prevMP && (prevTestVector2 - testVector2).length() <= 1)
              {
              // join portals
              printf("joining\n");
              prevMP->extend();
              fromPortalArray.remove(fromPortalidArray[j]);
              }
            else
              {
              
              printf("new portal beacause %s %s %s dist=%f\n",(prevdestMZ != destMZ?"mz differs":""),(prevMP?"":"no prevMP"),((prevTestVector2 - testVector2).length() > 1?"too far":""),(prevTestVector2 - testVector2).length());
  
              MP->setDestGrid(dx,dy);
              }
            
            prevMP=MP;
            }
          prevTestVector2=testVector2;
          prevdestMZ=destMZ;
  
          }
        
        }
*/    

      for (unsigned int pindex=0;pindex<fromPortalArray.size();pindex++)
        printf ("MP[%u] : %f,%f\n",pindex,fromPortalArray[pindex]->getLow2().x,fromPortalArray[pindex]->getLow2().y);
      unsigned int deletedPortals=0;
      for(unsigned int j=0;j < fromPortalidArray.size() - deletedPortals; ++j)
        {
        MovePortal* MP=fromPortalArray[fromPortalidArray[j]-deletedPortals];
        Vector2 testVector2=MP->getLow2();
        float z=connexionPts[MZArray[i]][fromPortalidArray[j]];
        printf("from %f,%f (%f) portalid %u\n",testVector2.x,testVector2.y,z,fromPortalidArray[j]);
        printf ("MP[%u] : %f,%f\n",fromPortalidArray[j],fromPortalArray[fromPortalidArray[j]-deletedPortals]->getLow2().x,fromPortalArray[fromPortalidArray[j]-deletedPortals]->getLow2().y);

        switch (direction)
          {
          case EXTEND_N :
            printf("N\n");
            testVector2.y+=1;
          break;
          case EXTEND_S :
            printf("S\n");
            testVector2.y-=1;
          break;
          case EXTEND_E :
            printf("E\n");
            testVector2.x+=1;
          break;
          case EXTEND_W :
            printf("W\n");
           testVector2.x-=1;
          break;
          
          }
        
        printf("to %f,%f (%f)\n",testVector2.x,testVector2.y,z);
        destMZ=NULL;
        Array<MoveZone*> matchMZArray=destMoveZoneContainer->getMoveZonesByZRange(testVector2.x,testVector2.y,z - 0.5, z + 0.5); // z can be slightly different
        if (matchMZArray.size() == 0)
          {
          printf("no match\n");
          prevMP=NULL;
          destMZ=NULL;
          }
        else if (matchMZArray.size() > 1)
          {
          printf("%u matches\n",matchMZArray.size() );
          for (unsigned int k = 0 ; k< matchMZArray.size(); ++k)
            printf("zone id %u\n",matchMZArray[k]->getIndex());
          prevMP=NULL;
          destMZ=NULL;
          }
        else
          {
          destMZ=matchMZArray[0];
          printf("one match : %u\n",destMZ->getIndex());

          if (prevdestMZ == destMZ && prevMP && (prevTestVector2 - testVector2).length() <= 1)
            {
            // join portals
            printf("joining\n");
            prevMP->extend();
            fromPortalArray.remove(fromPortalidArray[j]-deletedPortals);
            deletedPortals++;
            }
          else
            {
            
            printf("new portal beacause %s %s %s dist=%f\n",(prevdestMZ != destMZ?"mz differs":""),(prevMP?"":"no prevMP"),((prevTestVector2 - testVector2).length() > 1?"too far":""),(prevTestVector2 - testVector2).length());

            MP->setDestGrid(dx,dy);
            }
          
          prevMP=MP;
          }
        prevTestVector2=testVector2;
        prevdestMZ=destMZ;

        }
        
      }
  }

}

