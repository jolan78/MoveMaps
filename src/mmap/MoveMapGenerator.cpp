#include "MoveMapGenerator.h"
#include "VectorMoveMap.h"
#include "ModelContainer.h"
#include "SubModel.h"
#include "MoveMapAreaFinder.h"
#include "StepDirections.h"
#include "MoveMapBoxContainer.h"
#include "MoveMapBox.h"

namespace VMAP
{
  //extern Array<AABox> gBoxArray; // +++ debug
  //================================================

  MoveMapGenerator::MoveMapGenerator (const std::string& pDataDirPath,
                                      const std::string& pMMapDirPath)
  {
    iVMapManager = new VMapManager ();
    iDataDirPath = pDataDirPath;
    iMMapDirPath = pMMapDirPath;
  }

  //================================================

  MoveMapGenerator::~MoveMapGenerator ()
  {
    delete iVMapManager;
  }

  //================================================

  void
  MoveMapGenerator::fillNeighborPositionsInLayer (Vector3 pSourcePos, MoveLayer* pSourceMoveLayer, MoveLayer* pDestMoveLayer, MoveLayerConnectionPointsContainer& pConnPointsCont)
  {
    MoveLayerConnectionPoint& source2Dest = pConnPointsCont.getMoveLayerConnectionPoints (pSourceMoveLayer, pSourcePos).createAndGetMoveLayerConnectionPoint (pDestMoveLayer);

    for (int i = 0; i < D_SIZE; ++i)
      {
        Vector3 testPos = pSourcePos + StepDirections::getDiffVector (i);
        if (pDestMoveLayer->containsXZ (testPos))
          {
            if (pDestMoveLayer->getMovePointValue (testPos) == POS_REACHED)
              {
                MoveLayerConnectionPoint& dest2Source = pConnPointsCont.getMoveLayerConnectionPoints (pDestMoveLayer, testPos).createAndGetMoveLayerConnectionPoint (pSourceMoveLayer);
                source2Dest.addPos (testPos);
                dest2Source.addPos (pSourcePos);
              }
          }
      }
  }

  //================================================
#if 0
  // Calculate the move map zones and insert them into the AABSPTree
  // balance the tree and fill the static MoveZoneContainer with the tree

  void
  MoveMapGenerator::calculateMoveZones (const Array<MoveLayer*>& pMoveLayerArray,MoveMapContainer* pMoveMapContainer)
  {
    debugAssert (pMoveLayerArray.size () > 0);

    //AABSPTree<MoveZone*> *tree;
    //Table<MoveLayer*, MoveMapBox*> layerBoxMapping;
    //MoveLayerConnectionPointsContainer moveLayerConnectionPointsContainer;

    //tree = new AABSPTree<MoveZone*>();
    for (int i = 0; i < pMoveLayerArray.size (); ++i)
      {
        MoveLayer* currentLayer = pMoveLayerArray[i];
        MoveZoneContainer currentZoneContainer=MoveZoneContainer(currentLayer);
        
        
        
        /*MoveMapBox* currentMoveMapBox = new MoveMapBox (currentLayer);
        layerBoxMapping.set (currentLayer, currentMoveMapBox);

        Array<Vector3> differenLayerPos;
        currentLayer->getAccessToDifferentLayer (differenLayerPos);
        for (int j = 0; j < differenLayerPos.size (); ++j)
          {
            MoveLayer* connectionLayer = currentLayer->getConnectionLayer (differenLayerPos[j]);
            if (connectionLayer && connectionLayer->size () > 0)
              {
                AABox b;
                connectionLayer->getBounds (b);
                if (b.low () != b.high ())
                  {
                    // source and dest layer are swaped
                    fillNeighborPositionsInLayer (differenLayerPos[j], connectionLayer, currentLayer, moveLayerConnectionPointsContainer);
                  }
                else
                  {
                    int xxxxx = 0;
                  }
              }
            else
              {
                int xxxxx = 0;
              }
          }*/

        //tree->insert (currentZoneContainer);
        pMoveMapContainer->setMoveZonesContainer(currentZoneContainer,i);
      }
   /* tree->balance ();
    Table<MoveMapBox*, unsigned short> boxPositionsTable;
    MoveMapContainer* moveMapContainer = new MoveMapContainer (tree, boxPositionsTable);
    moveMapContainer->fillMoveMapConnectionManagerArray (moveLayerConnectionPointsContainer, layerBoxMapping, boxPositionsTable);
    delete tree;
    return (moveMapContainer);*/
    //return NULL;
  }
#endif
  //================================================

  // Calculate the move map boxes and insert them into the AABSPTree
  // balance the tree and fill the static MoveMapContainer with the tree

  MoveMapContainer*
  MoveMapGenerator::calculateMoveLayerConnections (const Array<MoveLayer*>& pMoveLayerArray)
  {
    debugAssert (pMoveLayerArray.size () > 0);

    AABSPTree<MoveMapBox*> *tree;
    Table<MoveLayer*, MoveMapBox*> layerBoxMapping;
    MoveLayerConnectionPointsContainer moveLayerConnectionPointsContainer;

    tree = new AABSPTree<MoveMapBox*>();

    for (int i = 0; i < pMoveLayerArray.size (); ++i)
      {
        MoveLayer* currentLayer = pMoveLayerArray[i];
        MoveMapBox* currentMoveMapBox = new MoveMapBox (currentLayer);
        layerBoxMapping.set (currentLayer, currentMoveMapBox);

        Array<Vector3> differenLayerPos;
        currentLayer->getAccessToDifferentLayer (differenLayerPos);
        for (int j = 0; j < differenLayerPos.size (); ++j)
          {
            MoveLayer* connectionLayer = currentLayer->getConnectionLayer (differenLayerPos[j]);
            if (connectionLayer && connectionLayer->size () > 0)
              {
                AABox b;
                connectionLayer->getBounds (b);
                if (b.low () != b.high ())
                  {
                    // source and dest layer are swaped
                    fillNeighborPositionsInLayer (differenLayerPos[j], connectionLayer, currentLayer, moveLayerConnectionPointsContainer);
                  }
                else
                  {
                    int xxxxx = 0;
                  }
              }
            else
              {
                int xxxxx = 0;
              }
          }

        tree->insert (currentMoveMapBox);
      }
    tree->balance ();
    Table<MoveMapBox*, unsigned short> boxPositionsTable;
    MoveMapContainer* moveMapContainer = new MoveMapContainer (tree, boxPositionsTable);
    moveMapContainer->fillMoveMapConnectionManagerArray (moveLayerConnectionPointsContainer, layerBoxMapping, boxPositionsTable);
    delete tree;
    return (moveMapContainer);
  }

  //================================================

  void
  MoveMapGenerator::fillMCList (Array<ModelContainer*>& McList, int mapId, int x, int y, Vector3& low, Vector3& high, Vector3& oriLow, Vector3& oriHigh)
  {
    MapTree* mt = iVMapManager->getInstanceMapTree (mapId);
    std::string dirFileName = iVMapManager->getDirFileName (mapId, x, y);
    if (!mt->hasDirFile (dirFileName))
      dirFileName = iVMapManager->getDirFileName (mapId);
    if (mt->hasDirFile (dirFileName))
      {
        Array<std::string> fileNames = mt->getDirFiles (dirFileName).getFiles ();
        for (int i = 0; i < fileNames.size (); ++i)
          {
            ModelContainer* mc;
            std::string name;
            name = fileNames[i];

            mc = mt->getModelContainer (name);
            McList.push_back (mc);
            AABox box = mc->getAABoxBounds ();

            low = low.min (box.low ());
            high = high.max (box.high ());
            oriLow = oriLow.min (low);
            oriHigh = oriHigh.max (high);
          }
      }
  }

  bool
  MoveMapGenerator::generateMoveMaps (int mapId, int x, int y, bool GenCoords)
  {
    GridMapManager* gridMapManager = new GridMapManager (iDataDirPath.c_str (), mapId);

    int vmapok1 = iVMapManager->loadMap ((iDataDirPath + std::string ("/vmaps")).c_str (), mapId, x, y);
    if (!vmapok1)
      return false;
    //load nearby cells too,because some objects may be spanned accross 2 cells
    iVMapManager->loadMap ((iDataDirPath + std::string ("/vmaps")).c_str (), mapId, x, y + 1);
    iVMapManager->loadMap ((iDataDirPath + std::string ("/vmaps")).c_str (), mapId, x, y - 1);
    iVMapManager->loadMap ((iDataDirPath + std::string ("/vmaps")).c_str (), mapId, x + 1, y);
    iVMapManager->loadMap ((iDataDirPath + std::string ("/vmaps")).c_str (), mapId, x - 1, y);


    MapTree* mt = iVMapManager->getInstanceMapTree (mapId);
    Vector3 low = Vector3 (inf (), inf (), inf ());
    Vector3 high = Vector3 (-inf (), -inf (), -inf ());
    Vector3 oriLow = Vector3 (inf (), inf (), inf ());
    Vector3 oriHigh = Vector3 (-inf (), -inf (), -inf ());

    AABSPTree<Triangle>* resultTree = new AABSPTree<Triangle > ();
    std::auto_ptr< AABSPTree<Triangle> > rt_ptr (resultTree);

    Array<ModelContainer*> McList;

    fillMCList (McList, mapId, x, y, low, high, oriLow, oriHigh);

    Vector3 plow = low;
    Vector3 phigh = high;
    Vector3 poriLow, poriHigh; // = oriLow,poriHigh = oriHigh;

    Vector3 innerLow (y * 533.3333f, low.y, x * 533.3333f);
    Vector3 innerHigh ((y + 1)*533.3333f, high.y, (x + 1)*533.3333f);
    poriLow = Vector3 (int(innerLow.x) + 1, innerLow.y, int(innerLow.z) + 1);
    poriHigh = Vector3 (int(innerHigh.x), innerHigh.y, int(innerHigh.z));
    //innerLow = innerLow.max (low);
    //innerHigh = innerHigh.min (high);

    /*printf("plow      %f %f\n",plow.x,plow.z); // Checking coords code
    printf("innerLow  %f %f\n",innerLow.x,innerLow.z);
    printf("phigh     %f %f\n",phigh.x,phigh.z);
    printf("innerHigh %f %f\n\n",innerHigh.x,innerHigh.z);*/

    // 3rd try
    if (plow.x < poriLow.x) // Go Left 1
      plow.x = poriLow.x - 1;
    if (plow.z < poriLow.z)
      plow.z = poriLow.z - 1;
    if (phigh.x > poriHigh.x)
      phigh.x = poriHigh.x + 2;
    if (phigh.z > poriHigh.z)
      phigh.z = poriHigh.z + 2;

    if (plow.x > poriLow.x - 1 && (poriLow.x - plow.x) < 2)
      plow.x = poriLow.x - 1;
    if (plow.z > poriLow.z - 1 && (poriLow.z - plow.z) < 2)
      plow.z = poriLow.z - 1;
    if (phigh.x < poriHigh.x + 2 && (poriHigh.x - phigh.x) < 2)
      phigh.x = poriHigh.x + 2;
    if (phigh.z < poriHigh.z + 2 && (poriHigh.z - phigh.z) < 2)
      phigh.z = poriHigh.z + 2;

    innerLow = innerLow.min(plow);
    innerHigh = innerHigh.max(phigh);
    /*printf("plow      %f %f\n",plow.x,plow.z); // Checking coords code
    printf("phigh     %f %f\n",phigh.x,phigh.z);*/

    VectorMoveMap* vectorMoveMap = new VectorMoveMap (mapId, x, y, gridMapManager, iVMapManager);
    return calculate (vectorMoveMap, innerLow, innerHigh, plow, phigh, poriLow, poriHigh, resultTree, mapId, x, y, GenCoords);
  }

  //================================================

  bool
  MoveMapGenerator::calculate (VectorMoveMap* pVectorMoveMap, const Vector3& pInnerLow, const Vector3& pInnerHigh, const Vector3& pLow, const Vector3& pHigh, const Vector3& oriLow, const Vector3& oriHigh, AABSPTree<Triangle>* pResultTree, unsigned int MapId, unsigned int x, unsigned int y, bool GenCoords)
  {
    CalcHelper calcHelper = CalcHelper ();
    calcHelper.iLow = pLow;
    calcHelper.iHigh = pHigh;
    calcHelper.iInnerLow = pInnerLow;
    calcHelper.iInnerHigh = pInnerHigh;
    int sizex = (int) pInnerHigh.x - pInnerLow.x + 2 + 1 / pVectorMoveMap->getResolution ();
    int sizey = (int) pInnerHigh.z - pInnerLow.z + 2 + 1 / pVectorMoveMap->getResolution ();

    printf ("Size: %d x %d\n", sizex, sizey);
    Vector3 basePos = pLow - Vector3 (1, 0, 1);

    calcHelper.iPositionControlArray = new PositionControlArray<unsigned char, 1 > *[D_SIZE];
    calcHelper.iSourcePointPositionControl = new PositionControlArray<unsigned char, 2 > (basePos, sizex, sizey, 1);
    for (int i = 0; i < D_SIZE; ++i)
      {
        calcHelper.iPositionControlArray[i] = new PositionControlArray<unsigned char, 1 > (basePos, sizex, sizey, 1);
      }
    calcHelper.iHightStoreMap = NULL;
    calcHelper.iHightStoreMap = new PositionControlArray<unsigned int, 32 > (basePos, sizex, sizey, 2);
    calcHelper.iHightStoreMap->fill (calcHelper.iHightStoreMap->getMaxValue ());

    calcHelper.iBounds = AABox (pLow, pHigh); // box to check if we are in the horizontal range
    //calcHelper.iBounds = AABox (oriLow, oriHigh); // box to check if we are in the horizontal range
    calcHelper.iTree = pResultTree;

    calcHelper.calcHeight = 0;
    calcHelper.getHeight = 0;

    if (!pVectorMoveMap->processMapGrid (calcHelper))
      return false;

    MoveMapContainer* moveMapContainer = calculateMoveLayerConnections (calcHelper.iMoveLayers);
  time_t sec1;
  time (&sec1);

    //calculateMoveZones(calcHelper.iMoveLayers /*moveMapContainer <- to get conexions with getVectorMoveLayerConnectionPointsTable ?? */, moveMapContainer);
   // MoveZoneContainer MZContainer=MoveZoneContainer(calcHelper.iMoveLayers);
/*    printf("calcHelper.iBounds : %f,%f %f,%f\n",calcHelper.iBounds.low().x,calcHelper.iBounds.low().z,calcHelper.iBounds.high().x,calcHelper.iBounds.high().z);
    printf("calcHelper.iLow : %f,%f\n",calcHelper.iLow.x,calcHelper.iLow.z);
    printf("calcHelper.iHigh : %f,%f\n",calcHelper.iHigh.x,calcHelper.iHigh.z);
    printf("calcHelper.iInnerLow : %f,%f\n",calcHelper.iInnerLow.x,calcHelper.iInnerLow.z);
    printf("calcHelper.iInnerHigh : %f,%f\n",calcHelper.iInnerHigh.x,calcHelper.iInnerHigh.z);*/

    moveMapContainer->setMoveZonesContainer(calcHelper.iBounds);
  time_t sec2;
  time (&sec2);
  long diff = sec2 - sec1;
  printf ("MoveZone Time = %d\n", diff);

    // Time to save this to disk
    moveMapContainer->save (iMMapDirPath.c_str (), pInnerLow, pInnerHigh, oriLow, oriHigh, MapId, x, y, GenCoords);
    return true;
  }

}
