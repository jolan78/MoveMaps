#ifndef _MOVEZONE_H_
#define _MOVEZONE_H_

//#include "AABSPTree.h"
#include "RStarTree.h"

#include <G3D/AABox.h>
#include <G3D/Table.h>

#include "MoveLayer.h"

#include "PositionControlArray.h"
#include "MoveMapBoxContainer.h"
#include "MoveMapBox.h"

#define EXTEND_N 0
#define EXTEND_E 1
#define EXTEND_S 2
#define EXTEND_W 3

#define FLOAT_HEIGHT_CANT_REACH 999999.0f
//#define MAX_ZONES_PER_LAYER 500000 // enough to handle 1x1 zones for a 600x600 layer

using namespace G3D;



namespace VMAP
{
  class MoveMapConnectionManager;
  class MoveZone;
  class MoveZoneContainer;
  
  struct gridPortal
  {
    unsigned int MoveZoneId;
    float fromx;
    float fromy;
    float destz;
  };
  
  class MovePortal
  {
  private:
    //unsigned int DestLayerID;
    unsigned int DestZoneID;
    MoveZone* DestZone;
    float X1, Y1, Z1;
    float X2, Y2, Z2;
    unsigned int direction;
    unsigned int destGridX,destGridY;
    Array<float> tempHeight;
  public:
    MovePortal (float pX1,float pY1,float pZ1,float tHeight,unsigned int pDirection, MoveZone* MZ);
    MovePortal (Vector3 low,Vector3 high/*debug*/,unsigned int destID,unsigned int pDirection);
    MovePortal () {};
    ~MovePortal () {};

    void extend();
    void extend(float tHeight);
    void connect(MoveZoneContainer* MZContainer);
    void save(FILE* fp);
    void load(FILE* fp);
    void reconnect(Array<MoveZone*>& moveZoneArray);

    Vector3 getLow() {return Vector3(X1,Y1,Z1); }
    Vector3 getHigh() {return Vector3(X2,Y2,Z2); }
    Vector2 getLow2() {return Vector2(X1,Y1); }
    Vector2 getHigh2() {return Vector2(X2,Y2); }
    Vector2 getCenter2() {return Vector2(X1+(X2-X1)/2, Y1+(Y2-Y1)/2); }
    unsigned int getDestinationID() { return DestZoneID; }
    MoveZone* getDestination() { return DestZone; }
    unsigned int getDirection() { return direction; }
    bool isGridPortal() {return (destGridX!=0 || destGridY!=0); };
    void setGridPortal(unsigned int gridX,unsigned int gridY) { destGridX=gridX; destGridY=gridY; };
    void setDestGridAndZone(unsigned int x,unsigned int y,unsigned int zoneID) {destGridX=x,destGridY=y,DestZoneID=zoneID; }
  };

  class MoveZone
  {
  private:
    AABox iBounds;
    Vector3 low;
    Vector3 high;
    Vector3 calcBasePos;
    AABox gridBounds;
    Array<MovePortal*> iPortals;
    G3D::Table<Vector2, Vector3> PortalCells[4];
    G3D::Table<Vector2, Vector3> LayerConnexions[4];
    
    Set<Vector2> *myOutOfZonePoints;
    Set<Vector2> *myReachedPos;
	const MoveMapBox* zMoveMapBox;
	
	bool Extend(unsigned int direction,unsigned int extendermask);
	
	unsigned int iIndex;
  public:
    MoveZone () { };
    MoveZone (const MoveMapBox* iMoveMapBox,unsigned int ZoneIndex,Vector2* sPos,Set<Vector2>* outOfZonePoints,Set<Vector2>* reachedPos,AABox iGridBounds);
    ~MoveZone ()
      {
      iPortals.deleteAll();
      }
    void save(FILE* fp);
    void load(FILE* fp);
    void reconnectPortals(Array<MoveZone*>& moveZoneArray);
    
    const AABox&
    getBounds () const
    {
      return iBounds;
    }
    
    void
    getBoxBounds (AABox& Bounds) const
    {
      Bounds=zMoveMapBox->getBounds ();
    }
    void
    setBounds (const AABox*  pBox)
    {
      iBounds=*pBox;
    }
    
    Table<Vector2, Vector3>*
    getPortalCell(unsigned int direction)
      {
      return &PortalCells[direction];
      }
    
    Table<Vector2, Vector3>*
    getLayerConnexions(unsigned int direction)
      {
      return &LayerConnexions[direction];
      }
    
    unsigned int 
    getIndex() const
      {
      return iIndex;
      }
    
    void
    setIndex(unsigned int i)
      {
      iIndex=i;
      }
    
    const Array<MovePortal*>&
    getPortalArray() const
      {
      return iPortals;
      }
    
    void
    setPortalArray(Array<MovePortal*> * PArray)
      {
      iPortals = *PArray;
      }
    
    float
    getLocalHeightAt(float x, float y)
      {
      #ifdef CHECK_POINTINBOUNDS
      AABox BoxBounds=zMoveMapBox->getBounds ();     
      if(x <BoxBounds.low().x || x > BoxBounds.high().x ||
               y <BoxBounds.low().z || y > BoxBounds.high().z )
        {
        printf("MZ %d MoveMapBox does not contain %f,%f (%f,%f) (%f,%f)\n",getIndex(),x,y,x - BoxBounds.low().x, y - BoxBounds.low().y,x-BoxBounds.low().x,y-BoxBounds.low().y);
        assert(false);
        }
      #endif

      unsigned int val = zMoveMapBox->get ((float) x, (float) y);
      if (val == MOVEMAP_VALUE_CANT_REACH)
        return FLOAT_HEIGHT_CANT_REACH;
      else
        return zMoveMapBox->getFloatHeight (val);
      }

    inline unsigned int
    getLocalCompressedHeightAt(float x, float y)
      {
      #ifdef CHECK_POINTINBOUNDS
      AABox BoxBounds=zMoveMapBox->getBounds ();     
      if(x <BoxBounds.low().x || x > BoxBounds.high().x ||
               y <BoxBounds.low().z || y > BoxBounds.high().z )
        {
        printf("MZ %d MoveMapBox does not contain %f,%f (%f,%f) (%f,%f)\n",getIndex(),x,y,x - BoxBounds.low().x, y - BoxBounds.low().y,x-BoxBounds.low().x,y-BoxBounds.low().y);
        assert(false);
        }
      #endif

      return zMoveMapBox->get ((float) x, (float) y);
      }

    
    float
    getGlobalHeightAt(float x, float y)
      {
      const Vector3 basePos = zMoveMapBox->getBasePosition ();
      return getLocalHeightAt(x-basePos.x, y-basePos.z);
      }

    bool operator== (const MoveZone& pMZ) const;
    size_t hashCode ();
    
  };
  
  // to sort Array<MoveZone*> returned by MZ_Tree->load()
  static bool MoveZoneLT(MoveZone*const& elem1, MoveZone*const& elem2)
    {
    return elem1->getIndex() < elem2->getIndex();
    }


}

// for rstartree
inline void constructor(VMAP::MoveZone* &MZp)
  {
    MZp=new VMAP::MoveZone();
  }
    

namespace VMAP
{
  class MoveZoneContainer
  {
  private:
    RStarTree<MoveZone*> *MZ_Tree;
    Array<MoveZone*> pMoveZonesArray;
    unsigned int moveZoneIndex;
    Array<gridPortal> gridPortals[4];
    Set<Vector2> outOfZonePoints;

    void generate(const MoveMapBox* iMoveMapBoxArray,int iNMoveMapBoxes,AABox gridBounds);

  public:
    MoveZoneContainer() {};
    ~MoveZoneContainer()
      {
      MZ_Tree->deleteAll();
      }
    
    MoveZoneContainer(const MoveMapBox* iMoveMapBoxArray,int iNMoveMapBoxes,AABox gridBounds)
      {
      generate(iMoveMapBoxArray,iNMoveMapBoxes,gridBounds);
      }

    MoveZone* getMoveZoneByCoords (const Vector3& pPos) const;
    Array<MoveZone*> getMoveZonesByZRange (const float x, const float y, const float lowZ, const float highZ) const;
    
    void connectPortals(MoveZone* iMoveZone,unsigned int direction);
    void connectLayerPortals(MoveZone* iMoveZone,unsigned int direction);
    
    void reconnectPortals();
    
    void
    save(FILE* fp)
    {
      MZ_Tree->save(fp);
    }

    void
    load(FILE* fp)
    {
      MZ_Tree = new RStarTree<MoveZone*>(2, 4);
      pMoveZonesArray=MZ_Tree->load(fp);
      pMoveZonesArray.sort(MoveZoneLT);
      reconnectPortals();
    }

    /* for debug only */
    Array<MoveZone*>
    getMoveZoneArray() const
    {
      //return MZ_Tree->getLeavesArray();
      return pMoveZonesArray;
    }

    int
    getNZones() const
    {
      return pMoveZonesArray.size();
    }
    
    MoveZone*
    getZone(unsigned int i) const
    {
      return pMoveZonesArray[i];
    }
    //const Table<unsigned int*, float>*
    const Array<gridPortal>*
    getGridPortals(unsigned int direction)
      {
      return &gridPortals[direction];
      }
    
    void setZone(AABox* pBox,unsigned int i, Array<MovePortal*> * PArray);
  };

  //========================================================

  //size_t hashCode (const MoveZone& pMZ);
  //bool operator==(const MoveZone& pMZ1, const MoveZone& pMZ2);
  size_t hashCode (const MoveZone* pMZ);
  //void getBounds (const MoveZone& pMZ, G3D::AABox& pAABox);
  void getBounds (const MoveZone* pMZ, G3D::AABox& pAABox);
}

#endif /*_MOVEZONE_H_*/
