#ifndef PATHGENERATOR_H
#define PATHGENERATOR_H

#include "G3D/Array.h"
#include "G3D/Vector3.h"

#include "MoveZone.h"

#define PATH_FOUND 0
#define ERR_ORIG_NOT_FOUND 1
#define ERR_DEST_NOT_FOUND 2
#define PATH_NOT_FOUND 3

#define STRAIGHT_COST 10;
#define DIAGONAL_COST 14;

//#define MANHATTAN_DIST 1
#define DIAGONAL_DIST 1


using namespace G3D;

struct PathNode {
  VMAP::MoveZone* moveZone;
  Vector2 position;
  unsigned int score;
  unsigned int distDone;
  unsigned int distRemain;
  PathNode* parent;
};



namespace VMAP
  { // FIXME : code style
class PathGenerator {
 private:
  Array<Vector3> Path;
  Vector3 pOrig;
  Vector3 pDest;
  MoveZoneContainer* pMoveZoneContainer;
  
  Array<PathNode*> openZones;
  Table<unsigned int,PathNode*> openMZTable;
  Array<unsigned int> closedMZId;

  unsigned int getFastDistance(Vector3 orig,Vector2 dest) { getFastDistance(orig.xy(),dest); }
  
  unsigned int getFastDistance(Vector2 orig,Vector3 dest) { getFastDistance(orig,dest.xy()); }
  
  unsigned int getFastDistance(Vector3 orig,Vector3 dest) { getFastDistance(orig.xy(),dest.xy()); }

  unsigned int getFastDistance(Vector2 orig,Vector2 dest)
  {
    #ifdef MANHATTAN_DIST
      return STRAIGHT_COST * (abs(orig.x-dest.y) + abs(orig.y-dest.y));
    #endif
    
    #ifdef DIAGONAL_DIST
    unsigned int diagonalSteps = min(abs(orig.x-dest.x), abs(orig.y-dest.y));
    unsigned int straightSteps = (abs(orig.x-dest.x) + abs(orig.y-dest.y)) - 2 * diagonalSteps;
    return diagonalSteps * DIAGONAL_COST + straightSteps * STRAIGHT_COST;
    #endif
  }

 public:
  PathGenerator(Vector3 orig,Vector3 dest,MoveZoneContainer* MZContainer) {pOrig=orig,pDest=dest,pMoveZoneContainer=MZContainer; } // TODO: use a load / unload manager
  
  void PrintPath()
    {
    for (unsigned int i=0;i<Path.size();++i)
      printf("%f,%f,%f\n",Path[i].x,Path[i].y,Path[i].z);
    }
  
  unsigned int
  GeneratePath ()
    {
    MoveZone* startMZ;
    MoveZone* destMZ;
    if (!(startMZ=pMoveZoneContainer->getMoveZoneByCoords(pOrig)))
      return ERR_ORIG_NOT_FOUND;
    if (!(destMZ=pMoveZoneContainer->getMoveZoneByCoords(pDest)))
      return ERR_DEST_NOT_FOUND;
    PathNode* PN = new PathNode;
    PN->moveZone=startMZ;
    PN->position=pOrig.xy();
    PN->distDone=0;
    PN->distRemain=getFastDistance(pOrig,pDest);
    PN->score=PN->distRemain;
    PN->parent=NULL;
    assert(PN->moveZone);
    openZones.append(PN);
    openMZTable.set(PN->moveZone->getIndex(),PN);
    
    
    do {
//printf("openZones: size%u\n",openZones.size());
      PN = openZones.pop(); // shrink array, slower but safer, perhaps not necessary shrink now ?
      assert(PN->moveZone);
      closedMZId.push_back(PN->moveZone->getIndex());
      
//printf("poped\n");
      
      if (PN->moveZone == destMZ)
        {
        /* we added the dest zone to the closed list,
        thats when we consider we found the path instead of when we add it to the open list
        if we use weighted path */
        do {
          // TODO : that's where we should stretch the path
          Path.insert(0,Vector3(PN->position,PN->moveZone->getBounds().high().z));
          PN=PN->parent;
        } while(PN != NULL);
        Path.append(pDest);
        return PATH_FOUND;
        }
//printf("MZ %u\n", PN->moveZone->getIndex());
     
      Array<MovePortal*> PortalArray = PN->moveZone->getPortalArray();
//printf("2\n");
     
      for (Array<MovePortal*>::Iterator p=PortalArray.begin(); p!=PortalArray.end(); ++p)
        {
// printf("Portal -> %u\n",((*p)->getDestinationID()));
            

        if (!(*p)->isGridPortal() && !closedMZId.contains((*p)->getDestinationID()))
          { // it's not in the closed list
          assert (((*p)->getDestinationID()) < 4000);
          Vector2 portalCenter=(*p)->getCenter2();

          PathNode* destPN;
          if (openMZTable.get((*p)->getDestinationID(),destPN))
            {
// printf("4\n");


            assert(destPN->moveZone);
            int distDone=PN->distDone + getFastDistance(PN->position,portalCenter);
            if (distDone < destPN->distDone)
              { // this node is better than the one in the open list, replace it
              // TODO : lot of duplicated code here ...
//printf("update\n");
              destPN->position=portalCenter;
              destPN->distDone=PN->distDone + getFastDistance(PN->position,portalCenter);
              destPN->distRemain=getFastDistance(portalCenter,pDest);
              destPN->score=destPN->distDone + destPN->distRemain;
              destPN->parent=PN;
              assert (destPN->moveZone->getIndex() < 4000);
              unsigned int dbgsize=openZones.size();
//printf("rem\n");
//printf("openZones: rem%u size%u\n",openZones.findIndex(destPN),openZones.size());
              openZones.fastRemove(openZones.findIndex(destPN));
              assert (openZones.size() != dbgsize);
              unsigned int nodeIdx=openZones.size();
              while (nodeIdx > 0)
                {
                nodeIdx--;
                if (destPN->score <= openZones[nodeIdx]->score)
                  break;
                }
//printf("openZones insert at %d size%u\n",nodeIdx,openZones.size());
             openZones.insert(nodeIdx,destPN);
              }
            }
          else
            { // this node is not in the open list, add it
//printf("add\n");
            destPN = new PathNode;
            destPN->moveZone=(*p)->getDestination();
            assert(destPN->moveZone);
            destPN->position=portalCenter;
            destPN->distDone=PN->distDone + getFastDistance(PN->position,portalCenter);
            destPN->distRemain=getFastDistance(portalCenter,pDest);
            destPN->score=destPN->distDone + destPN->distRemain;
            destPN->parent=PN;
            int nodeIdx=openZones.size();
            while (nodeIdx > 0)
              {
              nodeIdx--;
              if (destPN->score <= openZones[nodeIdx]->score)
                break;
              }
//printf("openZones insert at %d size%u\n",nodeIdx,openZones.size());

            openZones.insert(nodeIdx,destPN);
            openMZTable.set(destPN->moveZone->getIndex(),destPN);
            
            }
          }
        // TODO : else manage nearby grid load
        }
      
    } while (openZones.size());
    
    
    return PATH_NOT_FOUND;
    }
  
  

};
};
#endif /* PATHGENERATOR_H */
