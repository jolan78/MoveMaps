
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
    Table<unsigned int, Table<Vector2,float> > connexionPtsAtoB;
    Table<unsigned int, Table<Vector2,float> > connexionPtsBtoA;
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
    float x,y,z;
 printf ("load grid_cnx_%03u_%02u_%02u_%02u_%02u\n",iMap, iax, iay, ibx, iby);
    sprintf (buffer, "%s/grid_cnx_%03u_%02u_%02u_%02u_%02u.tmp", startCoordsPath.c_str(),iMap, iax, iay, ibx, iby);
    CnxPts = fopen (buffer, "rb");
    if (!CnxPts)
      {
      printf("ERROR : Failed to open %s\n",buffer);
      exit(1);
      }
    while (fgets (readBuffer, bufferSize - 1, CnxPts))
      {
      sscanf (readBuffer, "%u,%f,%f,%f", &mzID, &x, &y, &z);
      Vector2 fromPt=Vector2(x,y);
      Table<Vector2,float> vals;
      connexionPtsAtoB.get(mzID,vals);
      vals.set(fromPt,z);
      connexionPtsAtoB.set(mzID,vals);
      }
    fclose (CnxPts);

 printf ("load grid_cnx_%03u_%02u_%02u_%02u_%02u\n",iMap, ibx, iby, iax, iay);
    sprintf (buffer, "%s/grid_cnx_%03u_%02u_%02u_%02u_%02u.tmp", startCoordsPath.c_str(),iMap, ibx, iby, iax, iay);
    CnxPts = fopen (buffer, "rb");
    if (!CnxPts)
      {
      printf("ERROR : Failed to open %s\n",buffer);
      exit(1);
      }
    while (fgets (readBuffer, bufferSize - 1, CnxPts))
      {
      sscanf (readBuffer, "%u,%f,%f,%f", &mzID, &x, &y, &z);
      Vector2 fromPt=Vector2(x,y);
      Table<Vector2,float> vals;
      connexionPtsBtoA.get(mzID,vals);
      vals.set(fromPt,z);
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
  // to sort Array<Vector2>
  static bool Vector2LT(const Vector2& elem1, const Vector2& elem2)
    {
    return elem1.x < elem2.x || elem1.y < elem2.y;
    }

  void
  MapConnector::connect (Table<unsigned int, Table<Vector2,float> > connexionPts,MoveZoneContainer* fromMoveZoneContainer,MoveZoneContainer* destMoveZoneContainer, unsigned short direction, int dx, int dy)
  {
    Array<unsigned int> MZArray = connexionPts.getKeys();
    MZArray.sort();
    MoveZone* destMZ=NULL;
    MoveZone* prevdestMZ=NULL;
    MovePortal* prevMP=NULL;
    Vector2 prevTestVector2;
    for (unsigned int i=0; i<MZArray.size(); ++i)
      {
      MoveZone* fromMZ=fromMoveZoneContainer->getZone(MZArray[i]);
      Array<MovePortal*> fromPortalArray=fromMZ->getPortalArray();
      Table<Vector2,MovePortal*> fromPortalTable;
      for (unsigned int j=0;j<fromPortalArray.size();++j)
        {
        if(fromPortalArray[j]->getDirection() == direction)
          fromPortalTable.set(fromPortalArray[j]->getLow2(),fromPortalArray[j]);
        }
      Array<Vector2> fromPtArray = connexionPts[MZArray[i]].getKeys();
      fromPtArray.sort(Vector2LT);
      printf("in movezone %d : %u out of %u portals\n",fromMZ->getIndex(),fromPtArray.size(),fromPortalArray.size());
      
      prevMP=NULL;
      for (unsigned int pindex=0;pindex<fromPortalArray.size();pindex++)
        printf ("MP[%u] : %f,%f -> %f,%f\n",pindex,fromPortalArray[pindex]->getLow2().x,fromPortalArray[pindex]->getLow2().y,fromPortalArray[pindex]->getHigh2().x,fromPortalArray[pindex]->getHigh2().y);
      unsigned int deletedPortals=0;
      
      for(unsigned int j=0;j < fromPtArray.size(); ++j)
        {
        MovePortal* MP=fromPortalTable[fromPtArray[j]];
        Vector2 testVector2=MP->getLow2();
        float z=connexionPts[MZArray[i]][fromPtArray[j]];
        printf("from %f,%f (%f)\n",testVector2.x,testVector2.y,z);
        printf ("MP[%u] : %f,%f\n",j,fromPortalTable[fromPtArray[j]]->getLow2().x,fromPortalTable[fromPtArray[j]]->getLow2().y);
        if (MP->getLow2() != MP->getHigh2())
          {
          printf("ERROR : MP %u of movezone %u is already extended (already connected ?)\n",j,fromMZ->getIndex());
          printf("%f,%f -> %f,%f\naborting\n",MP->getLow2().x,MP->getLow2().y,MP->getHigh2().x,MP->getHigh2().y);
          exit (0);
          }

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
            MP=prevMP;
            printf("prev mp is now %f,%f -> %f,%f\n",prevMP->getLow2().x,prevMP->getLow2().y,prevMP->getHigh2().x,prevMP->getHigh2().y);
            fromPortalArray.remove(fromPortalArray.findIndex(fromPortalTable[fromPtArray[j]]));
            deletedPortals++;
            }
          else
            {
            
            printf("new portal beacause %s %s %s dist=%f\n",(prevdestMZ != destMZ?"mz differs":""),(prevMP?"":"no prevMP"),((prevTestVector2 - testVector2).length() > 1?"too far":""),(prevTestVector2 - testVector2).length());

            MP->setDestGridAndZone(dx,dy,destMZ->getIndex());
            
            }
          
          prevMP=MP;
          }
        prevTestVector2=testVector2;
        prevdestMZ=destMZ;

        }
      fromMZ->setPortalArray(&fromPortalArray);
      }
  }

}

