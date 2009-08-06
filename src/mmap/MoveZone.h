#ifndef _MOVEZONE_H_
#define _MOVEZONE_H_

#include "RStarTree.h"

#include <G3D/AABox.h>
#include <G3D/Table.h>

#include "MoveLayer.h"

//#include "PositionControlArray.h"
#include "MoveMapBoxContainer.h"
#include "MoveMapBox.h"

#define EXTEND_N 0
#define EXTEND_E 1
#define EXTEND_S 2
#define EXTEND_W 3

#define FLOAT_HEIGHT_CANT_REACH 999999.0f

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
    unsigned int DestZoneID;
    MoveZone* DestZone;
    Vector2 center;
    float radius;
    unsigned int direction;
    unsigned int destGridX,destGridY;
    Array<float> tempHeight;
  public:
    MovePortal (Vector2 pCenter,float tHeight,unsigned int pDirection, MoveZone* MZ);
    MovePortal () {};
    ~MovePortal () {};

    void extend();
    void extend(float tHeight);
    void connect(MoveZoneContainer* MZContainer);
    void save(FILE* fp);
    void load(FILE* fp);
    void reconnect(Array<MoveZone*>& moveZoneArray);
    Vector2 getLow2();
    Vector2 getHigh2();
    
    Vector2 getCenter2() {return center; }
    unsigned int getDestinationID() { return DestZoneID; }
    MoveZone* getDestination() { return DestZone; }
    unsigned int getDirection() { return direction; }
    bool isGridPortal() {return (destGridX!=0 || destGridY!=0); };
    void setGridPortal(unsigned int gridX,unsigned int gridY) { destGridX=gridX; destGridY=gridY; };
    void setDestGridAndZone(unsigned int x,unsigned int y,unsigned int zoneID) {destGridX=x,destGridY=y,DestZoneID=zoneID; }
  };

  class MoveZone
  {
  protected:
	unsigned int iIndex;
    AABox iBounds;
    Array<MovePortal*> iPortals;

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
    
    unsigned int getIndex() const { return iIndex; }

    const AABox& getBounds () const { return iBounds; }
    
    void setIndex(unsigned int i) { iIndex=i; }
    
    const Array<MovePortal*>& getPortalArray() const { return iPortals; }
    
    void setPortalArray(Array<MovePortal*> * PArray) { iPortals = *PArray; }
    
    bool operator== (const MoveZone& pMZ) const;
    size_t hashCode ();
    
  };

class MoveZoneGenerator : public MoveZone
  {
  private:
    Vector3 low;
    Vector3 high;
    Vector3 calcBasePos;
    AABox gridBounds;
    G3D::Table<Vector2, Vector3> PortalCells[4];
    G3D::Table<Vector2, Vector3> LayerConnexions[4];
    
    Set<Vector2> *myOutOfZonePoints;
    Set<Vector2> *myReachedPos;
	const MoveMapBox* zMoveMapBox;

	bool Extend(unsigned int direction,unsigned int extendermask);
  public:
    MoveZoneGenerator () { };
    MoveZoneGenerator (const MoveMapBox* iMoveMapBox,unsigned int ZoneIndex,Vector2* sPos,Set<Vector2>* outOfZonePoints,Set<Vector2>* reachedPos,AABox iGridBounds);

    void
    getBoxBounds (AABox& Bounds) const { Bounds=zMoveMapBox->getBounds (); }

    void setBounds (const AABox*  pBox) { iBounds=*pBox; }

    Table<Vector2, Vector3>* getPortalCell(unsigned int direction) { return &PortalCells[direction]; }
    
    Table<Vector2, Vector3>* getLayerConnexions(unsigned int direction) { return &LayerConnexions[direction]; }
        
    float
    getLocalHeightAt(float x, float y)
      {
      unsigned int val = zMoveMapBox->get ((float) x, (float) y);
      if (val == MOVEMAP_VALUE_CANT_REACH)
        return FLOAT_HEIGHT_CANT_REACH;
      else
        return zMoveMapBox->getFloatHeight (val);
      }

    inline unsigned int
    getLocalCompressedHeightAt(float x, float y)
      {
      return zMoveMapBox->get ((float) x, (float) y);
      }

    float
    getGlobalHeightAt(float x, float y)
      {
      const Vector3 basePos = zMoveMapBox->getBasePosition ();
      return getLocalHeightAt(x-basePos.x, y-basePos.z);
      }
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
  protected:
    RStarTree<MoveZone*> *MZ_Tree;
    Array<MoveZone*> pMoveZonesArray;


  public:
    MoveZoneContainer() {};
    ~MoveZoneContainer()
      {
      MZ_Tree->deleteAll();
      }

    MoveZone* getMoveZoneByCoords (const Vector3& pPos) const;
    Array<MoveZone*> getMoveZonesByZRange (const float x, const float y, const float lowZ, const float highZ) const;
        
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
    Array<MoveZone*> getMoveZoneArray() const { return pMoveZonesArray; }
    int getNZones() const { return pMoveZonesArray.size(); }

    MoveZone* getZone(unsigned int i) const { return pMoveZonesArray[i]; }

  };
  
  class MoveZoneContainerGenerator  : public MoveZoneContainer
  {
  private:
    unsigned int moveZoneIndex;
    Array<gridPortal> gridPortals[4];
    Set<Vector2> outOfZonePoints;
    void generate(const MoveMapBox* iMoveMapBoxArray,int iNMoveMapBoxes,AABox gridBounds);
  public:
    MoveZoneContainerGenerator() {};
    MoveZoneContainerGenerator(const MoveMapBox* iMoveMapBoxArray,int iNMoveMapBoxes,AABox gridBounds)
      {
      generate(iMoveMapBoxArray,iNMoveMapBoxes,gridBounds);
      }
    
    void connectPortals(MoveZoneGenerator* iMoveZone,unsigned int direction);
    void connectLayerPortals(MoveZoneGenerator* iMoveZone,unsigned int direction);

    const Array<gridPortal>*
    getGridPortals(unsigned int direction) { return &gridPortals[direction]; }

    void saveGridCnx(const char* filename, const unsigned int direction);

  };

  //========================================================
  // FIXME : originally for aabsptree ; is this still needed ?
  size_t hashCode (const MoveZone* pMZ);
  void getBounds (const MoveZone* pMZ, G3D::AABox& pAABox);
}

#endif /*_MOVEZONE_H_*/
