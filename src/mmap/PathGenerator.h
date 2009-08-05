#ifndef PATHGENERATOR_H
#define PATHGENERATOR_H

#include "G3D/Array.h"
#include "G3D/Vector3.h"

#include "MoveZone.h"

#define PATH_FOUND 0
#define ERR_ORIG_NOT_FOUND 1
#define ERR_DEST_NOT_FOUND 2
#define PATH_NOT_FOUND 3

#define STRAIGHT_COST 10
#define DIAGONAL_COST 14

// TODO : re-check
#define MANHATTAN_DIST 1
#define DIAGONAL_DIST 2

// slower but more accurate pathfinding
// useless when using withStreching()
//#define THREE_PTS_PER_PORTAL

using namespace G3D;

struct PathNode {
  VMAP::MoveZone* moveZone;
  VMAP::MovePortal* movePortal;
  Vector2 position;
  unsigned int score;
  unsigned int distDone;
  unsigned int distRemain;
  PathNode* parent;
};

namespace VMAP
  {
  class PathGenerator {
  private:
    Array<Vector3> Path;
    Vector3 pOrig;
    Vector3 pDest;
    unsigned int pDistCalc;
    bool pPostStretch;
    const MoveZoneContainer* pMoveZoneContainer;
    
    Array<PathNode*> openZones;
    Array<PathNode*> closedZones;
    // FIXME : temporary solution: we must add zones to open list if we pass thru a different portal
    //Table<unsigned int,PathNode*> openMZTable;
    Table<MovePortal*,PathNode*> _openMZTable;
    Array<unsigned int> closedMZId;

    unsigned int getFastDistance(Vector3 orig,Vector2 dest) { return getFastDistance(orig.xy(),dest); }
    
    unsigned int getFastDistance(Vector2 orig,Vector3 dest) { return getFastDistance(orig,dest.xy()); }
    
    unsigned int getFastDistance(Vector3 orig,Vector3 dest) { return getFastDistance(orig.xy(),dest.xy()); }

    unsigned int getFastDistance(Vector2 orig,Vector2 dest)
    {
      if ( pDistCalc == MANHATTAN_DIST )
        {
        return (abs(orig.x-dest.x) + abs(orig.y-dest.y)) * STRAIGHT_COST;
        }
      else
        {
        unsigned int diagonalSteps = min(abs(orig.x-dest.x), abs(orig.y-dest.y));
        unsigned int straightSteps = (abs(orig.x-dest.x) + abs(orig.y-dest.y)) - 2 * diagonalSteps;
        return diagonalSteps * DIAGONAL_COST + straightSteps * STRAIGHT_COST;
        }
    }
  
  void insertPathNode(PathNode* PN)
    {
    // dychotomy : about 10 time less comparisons
    int min=0;
    int max=openZones.size();
    unsigned int middle=0;
    
    while (max>min)
      {
      middle=min+((max-min)>>1); // >>1 => /2
      if (PN->score < openZones[middle]->score)
        min=middle+1;
      else if (PN->score > openZones[middle]->score)
        max=middle-1;
      else
        {
        min=middle;
        break;
        }
      }
    openZones.insert(min,PN);
    /*
    // regular insert
    int nodeIdx=openZones.size()-1;
    while (nodeIdx >= 0)
      {
      if (PN->score <= openZones[nodeIdx]->score)
        break;
      nodeIdx--;
      }
    openZones.insert(nodeIdx+1,PN);
    */
    }
  
  public:
    PathGenerator(Vector3 orig,Vector3 dest,const MoveZoneContainer* MZContainer) {pOrig=orig,pDest=dest,pMoveZoneContainer=MZContainer,pDistCalc=MANHATTAN_DIST,pPostStretch=false ;} // TODO: use a load / unload manager

    ~PathGenerator()
      {
      openZones.deleteAll();
      // FIXME closedZones.deleteAll();
      }

    void setDistanceCalc(unsigned int dcalc) { pDistCalc=dcalc; }
    void withStreching() { pPostStretch=true; }
    
    void PrintPath()
      {
      printf("Opened %u, closed %u zones\n",_openMZTable.size(),closedMZId.size());
      /*if (!Path.size())
        {
        printf("open:\n");
        for (Table<MovePortal*,PathNode*>::Iterator itr = _openMZTable.begin();itr != _openMZTable.end();++itr)
          printf("%u (%u)\n",(*itr).value->moveZone->getIndex(),(*itr).key);
        printf("closed:\n");
        for (unsigned int i=0;i<closedMZId.size();++i)
          printf("%u\n",closedMZId[i]);
        }
      for (unsigned int i=0;i<Path.size();++i)
        printf("%f,%f,%f\n",Path[i].x,Path[i].y,Path[i].z);*/
      }

    float getRealDistance()
      {
      float dist=0;
      Vector2 prev=pOrig.xy();
      for (unsigned int i=1;i<Path.size();++i)
        {
        dist+= (Path[i].xy() - prev).length();
        prev=Path[i].xy();
        }
      return dist;
      }

    Array<Vector3>&
    getPathArray()
      {
      return Path;
      }

    Array<unsigned int>&
    getVisitedCells()
      {
      return closedMZId;
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
      _openMZTable.set(NULL/*PN->moveZone->getIndex()*/,PN);

      do {
        PN = openZones.pop(); // shrink array, slower but safer, perhaps not necessary shrink now ?
        assert(PN->moveZone);
        closedMZId.push_back(PN->moveZone->getIndex());
        closedZones.push_back(PN);

        if (PN->moveZone == destMZ)
          {
          /* we added the dest zone to the closed list,
          thats when we consider we found the path instead of when we add it to the open list
          if we use weighted path */
          do {
            // TODO : that's where we can stretch the path, or we can do it in movement generator
            Path.insert(0,Vector3(PN->position,PN->moveZone->getBounds().high().z));

            if (pPostStretch)
              {
              bool passtru=false;
              Vector2 from=PN->position;
              do {
                passtru=false;
                PN=PN->parent;
                if (PN != NULL && PN->parent!=NULL)
                  {
                  Vector2 to=PN->parent->position;
                  unsigned int pdir=PN->movePortal->getDirection();
                  
                  if (pdir == EXTEND_E || pdir == EXTEND_W)
                    {
                    float y = from.y + (to.y - from.y) * abs( (PN->position.x - from.x) / (to.x - from.x) );
                    if(y < PN->movePortal->getLow2().y )
                      PN->position.y=PN->movePortal->getLow2().y;
                    else if (y > PN->movePortal->getHigh2().y)
                      PN->position.y=PN->movePortal->getHigh2().y;
                    else
                      passtru=true;
                    }
                  else
                    {
                    float x = from.x + (to.x - from.x) * abs( (PN->position.y - from.y) / (to.y - from.y) );
                    if(x < PN->movePortal->getLow2().x)
                      PN->position.x=PN->movePortal->getLow2().x;
                    else if (x > PN->movePortal->getHigh2().x)
                      PN->position.x=PN->movePortal->getHigh2().x;
                    else
                      passtru=true;
                    }
                  }
              } while (passtru);
              }
            else
              PN=PN->parent;
          } while(PN != NULL);
          Path.append(pDest);
          return PATH_FOUND;
          }

        Array<MovePortal*> PortalArray = PN->moveZone->getPortalArray();

        for (Array<MovePortal*>::Iterator p=PortalArray.begin(); p!=PortalArray.end(); ++p)
          {
          if (!(*p)->isGridPortal() && !closedMZId.contains((*p)->getDestinationID()))
            { // it's not in the closed list
            
            #ifdef THREE_PTS_PER_PORTAL
            for (unsigned int pnb=0; pnb<3;++pnb)
              {
              Vector2 portalCenter;
              switch (pnb)
                {
                case 0:
                  portalCenter=(*p)->getLow2();
                  break;
                case 1:
                  portalCenter=(*p)->getCenter2();
                  break;
                case 2:
                  portalCenter=(*p)->getHigh2();
                  break;
                }
            #else
            Vector2 portalCenter=(*p)->getCenter2();
            #endif

            PathNode* destPN;
            if (_openMZTable.get((*p)/*->getDestinationID()*/,destPN) && destPN->movePortal == (*p))
              {
              assert(destPN->moveZone);
              unsigned int distDone=PN->distDone + getFastDistance(PN->position,portalCenter);
              if (distDone < destPN->distDone)
                { // this node is better than the one in the open list, replace it
                // TODO : lot of duplicated code here ...
                destPN->position=portalCenter;
                destPN->movePortal=(*p);
                destPN->distDone=distDone;
                destPN->distRemain=getFastDistance(portalCenter,pDest);
                destPN->score=destPN->distDone + destPN->distRemain;
                destPN->parent=PN;
                openZones.remove(openZones.findIndex(destPN));// remove() is slower but fastRemove() will cause problems with insert position searching
                
                insertPathNode(destPN);
                }
              }
            else
              { // this node is not in the open list, add it
              destPN = new PathNode;
              destPN->moveZone=(*p)->getDestination();
              destPN->movePortal=(*p);
              assert(destPN->moveZone);
              destPN->position=portalCenter;
              destPN->distDone=PN->distDone + getFastDistance(PN->position,portalCenter);
              destPN->distRemain=getFastDistance(portalCenter,pDest);
              destPN->score=destPN->distDone + destPN->distRemain;
              destPN->parent=PN;
              
              insertPathNode(destPN);
              _openMZTable.set(/*destPN->moveZone->getIndex()*/destPN->movePortal,destPN);
              }
            }
          // TODO : else manage nearby grid load
          
          #ifdef THREE_PTS_PER_PORTAL
            }
          #endif
          }
      } while (openZones.size());
      return PATH_NOT_FOUND;
      }
  };
};
#endif /* PATHGENERATOR_H */
