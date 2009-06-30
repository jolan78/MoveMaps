#include "AABSPTree.h"

#include <G3D/G3DAll.h>

#include "MoveMapBoxContainer.h"

template<> struct GHashCode<short>
{

  size_t operator()(short key) const
  {
    return static_cast<size_t> (key);
  }
};

namespace VMAP
{


  //=============================================================

  // Sort short

  int
  compareShorts (const void* pV1, const void* pV2)
  {
    short* v1 = (short*) pV1;
    short* v2 = (short*) pV2;
    return (*v1) - (*v1);
  }

  //=============================================================
  //==========  MoveMapConnectionManager ========================
  //=============================================================

  MoveMapConnectionManager::MoveMapConnectionManager (MoveMapConnctionHandle* pMoveMapConnctionHandle, MoveMapConnctionAxisArray* pMoveMapConnctionAxisArray, MoveMapDestHandle* pMoveMapDestHandle, float pGranularity)
  {
    iMoveMapConnctionHandle = pMoveMapConnctionHandle;
    iMoveMapConnctionAxisArray = pMoveMapConnctionAxisArray;
    iMoveMapDestHandle = pMoveMapDestHandle;
    //DEBUG
    printf("#### graaa ####\n");
    iGranularity = pGranularity;
  }

  //=============================================================

  MoveMapConnectionManager::MoveMapConnectionManager (const Table<Vector3, MoveLayerConnectionPoints>& pPosConnTable, const Table<MoveLayer*, MoveMapBox*>& pLayerMapBoxTable, const Table<MoveMapBox*, unsigned short>& pBoxPositionsTable)
  {
    Array<MoveMapConnctionHandle> handleArray;
    Array<MoveMapConnctionAxisArray> axisArray;
    Array<MoveMapDestHandle> destHandleArray;

    Array<Vector3> posArray;
    pPosConnTable.getKeys (posArray);
    G3D::Table<short, G3D::Table<short, MoveLayerConnectionPoints > > xzTable;
    for (int i = 0; i < posArray.size (); ++i)
      {
        float testx = posArray[i][Vector3::X_AXIS];
        debugAssert (posArray[i][Vector3::X_AXIS] == ((float) ((short) posArray[i][Vector3::X_AXIS])));
        short x = (short) posArray[i][Vector3::X_AXIS];
        if (!xzTable.containsKey (x))
          {
            Table<short, MoveLayerConnectionPoints> zt;
            xzTable.set (x, zt);
          }
        Table<short, MoveLayerConnectionPoints>& zt = xzTable.get (x);
        float testz = posArray[i][Vector3::Z_AXIS];
        debugAssert (posArray[i][Vector3::Z_AXIS] == ((float) ((short) posArray[i][Vector3::Z_AXIS])));
        short z = (short) posArray[i][Vector3::Z_AXIS];
        if (!zt.containsKey (z))
          {
            zt.set (z, pPosConnTable.get (posArray[i]));
          }
      }

    size_t handleArrayPos, axisArrayPos, destHandleArrayPos;
    handleArrayPos = axisArrayPos = destHandleArrayPos = 0;

    Array<short> xPosArray;
    xzTable.getKeys (xPosArray);
    MoveMapConnctionAxisArray moveMapConnctionAxisArray (xPosArray.size (), 0);
    axisArray.push_back (moveMapConnctionAxisArray);
    ++axisArrayPos;
    qsort (xPosArray.getCArray (), xPosArray.size (), sizeof (short), &compareShorts);
    for (int xp = 0; xp < xPosArray.size (); ++xp)
      {
        short x = xPosArray[xp];
        MoveMapConnctionHandle moveMapConnctionHandle (x, (unsigned short) axisArrayPos);
        ++axisArrayPos;
        handleArray.push_back (moveMapConnctionHandle);
      }
    handleArrayPos += xPosArray.size ();
    int destHandleGroupId = 0;
    for (int xp = 0; xp < xPosArray.size (); ++xp)
      {
        short x = xPosArray[xp];
        const Table<short, MoveLayerConnectionPoints>& zTable = xzTable.get (x);
        Array<short> zPosArray;
        zTable.getKeys (zPosArray);
        MoveMapConnctionAxisArray zConnctionAxisArray (zPosArray.size (), (unsigned short) handleArrayPos);
        axisArray.push_back (zConnctionAxisArray);
        qsort (zPosArray.getCArray (), zPosArray.size (), sizeof (short), &compareShorts);
        for (int zp = 0; zp < zPosArray.size (); ++zp)
          {
            short z = zPosArray[zp];
            const MoveLayerConnectionPoints& moveLayerConnectionPoints = zTable.get (z);
            MoveMapDestHandleArray moveMapDestHandleArray (moveLayerConnectionPoints, destHandleGroupId, pLayerMapBoxTable, pBoxPositionsTable);
            ++destHandleGroupId;
            MoveMapConnctionHandle moveMapConnctionHandle (z, (unsigned short) destHandleArrayPos);
            handleArray.push_back (moveMapConnctionHandle);
            for (size_t destp = 0; destp < moveMapDestHandleArray.size (); ++destp)
              {
                destHandleArray.push_back (moveMapDestHandleArray.get ((unsigned short) destp));
              }
            destHandleArrayPos += moveMapDestHandleArray.size ();
          }
        handleArrayPos += zPosArray.size ();
      }

    iMoveMapConnctionHandle = new MoveMapConnctionHandle[handleArray.size ()];
    memcpy (iMoveMapConnctionHandle, handleArray.getCArray (), handleArray.size () * sizeof (MoveMapConnctionHandle));

    iMoveMapConnctionAxisArray = new MoveMapConnctionAxisArray[axisArray.size ()];
    memcpy (iMoveMapConnctionAxisArray, axisArray.getCArray (), axisArray.size () * sizeof (MoveMapConnctionAxisArray));

    iMoveMapDestHandle = new MoveMapDestHandle[destHandleArray.size ()];
    memcpy (iMoveMapDestHandle, destHandleArray.getCArray (), destHandleArray.size () * sizeof (MoveMapDestHandle));
  }

  //=============================================================

  MoveMapConnectionManager::~MoveMapConnectionManager ()
  {
    if (iMoveMapConnctionHandle) delete[] iMoveMapConnctionHandle;
    if (iMoveMapConnctionAxisArray) delete[] iMoveMapConnctionAxisArray;
    if (iMoveMapDestHandle) delete[] iMoveMapDestHandle;
  }


  // Compare function for the bsearch

  int
  connectionHandleCompare (const void* pV1, const void* pV2)
  {
    return (((MoveMapConnctionHandle*) pV2)->getPosition () - ((MoveMapConnctionHandle*) pV1)->getPosition ());
  }

  //=============================================================

  /**
   * use bsearch to find the element and return the resulting index or -1 if no element was found
   */
  int
  MoveMapConnectionManager::_findValueInArray (int pIndex, int pSize, unsigned int pSearchVaue) const
  {
    int result = -1;
    
    // DEBUG : is this sorted ???
    printf ("looking for value %u at %d out of %d\n",pSearchVaue,pIndex,pSize);
    for (int i = pIndex;i<pSize-pIndex;++i)
      printf("%d\n",iMoveMapConnctionHandle[pIndex+i].getPosition ());
    
    // GUBED
    MoveMapConnctionHandle* foundElem = (MoveMapConnctionHandle*) bsearch (&pSearchVaue, &iMoveMapConnctionHandle[pIndex], pSize, sizeof (MoveMapConnctionHandle), &connectionHandleCompare);
    
    if (foundElem != 0)
      {
        printf("found\n");
        result = foundElem->getIndex ();
      }
    assert (false);
    return (result);
  }

  //=============================================================

  /**
   * return the index or -1 if no connection was found
   */
  int
  MoveMapConnectionManager::findMapIndex (const Vector3& pPos) const
  {
    int result = -1;
//DEBUG
printf ("looking for %f,%f,%f\ngranularity %f\n",pPos.x,pPos.y,pPos.z,iGranularity);
    // FIXME : iGranularity is never set ?
    int x = (int) (pPos.x /* * iGranularity*/ + 0.5);
    int y = (int) (pPos.y /* * iGranularity*/ + 0.5);
    int z = (int) (pPos.z /* * iGranularity*/ + 0.5);

    int yindex = _findValueInArray (iMoveMapConnctionAxisArray[0].iIndex, iMoveMapConnctionAxisArray[0].iNElements, x);
    if (yindex != -1)
      {
        int zindex = _findValueInArray (iMoveMapConnctionAxisArray[yindex].iIndex, iMoveMapConnctionAxisArray[yindex].iNElements, y);
        if (zindex != -1)
          {
            result = _findValueInArray (iMoveMapConnctionAxisArray[zindex].iIndex, iMoveMapConnctionAxisArray[zindex].iNElements, z);
          }
      }
    return (result);
  }

  //=============================================================
  //========== MoveMapDestHandleArray ===========================
  //=============================================================

  MoveMapDestHandleArray::MoveMapDestHandleArray (const MoveLayerConnectionPoints& pMoveLayerConnectionPoints, unsigned short pGroupId, const Table<MoveLayer*, MoveMapBox*>& pLayerMapBoxTable, const Table<MoveMapBox*, unsigned short>& pMapBoxPosTable)
  {
    Array<MoveLayer*> layerArray;
    pMoveLayerConnectionPoints.getKeys (layerArray);
    for (int i = 0; i < layerArray.size (); ++i)
      {
        const MoveLayerConnectionPoint& moveLayerConnectionPoint = pMoveLayerConnectionPoints.getMoveLayerConnectionPoint (layerArray[i]);
        const Array<Vector3>& destArray = moveLayerConnectionPoint.getDestArray ();
        MoveMapBox* mmb = pLayerMapBoxTable.get (layerArray[i]);
        unsigned short moveMapBoxPos = pMapBoxPosTable.get (mmb);
        for (int j = 0; j < destArray.size (); ++j)
          {
            const Vector3& pos = destArray[j];
            debugAssert (pos.x == ((float) ((short) pos.x)));
            debugAssert (pos.z == ((float) ((short) pos.z)));
            MoveMapDestHandle moveMapDestHandle (pGroupId, moveMapBoxPos, (short) pos.x, (short) pos.z);
            iDestHandleArray.push_back (moveMapDestHandle);
          }
      }
  }

  //=============================================================
  //==========  MoveMapContainer ================================
  //=============================================================

  MoveMapContainer::MoveMapContainer ()
  {
    iMoveMapBoxArray = NULL;
    iTreeNodes = NULL;
    iMoveMapConnectionManager = NULL;
  }

  //=============================================================

  MoveMapContainer::~MoveMapContainer ()
  {
    if (iMoveMapBoxArray != NULL) delete[] iMoveMapBoxArray;
    if (iTreeNodes != NULL) delete[] iTreeNodes;
    if (iMoveMapConnectionManager != NULL) delete iMoveMapConnectionManager;
  }

  //=============================================================

  /**
  Fill the static arrays and the table to have a mapping between MoveMapBox* and its pos in the array.
  We will need that table later on.
   */

  MoveMapContainer::MoveMapContainer (AABSPTree<MoveMapBox*>* pBoxTree, Table<MoveMapBox*, unsigned short>& pBoxPositionsTable)
  {
    int nMoveMapBoxes, nNodes;
    nMoveMapBoxes = nNodes = 0;

    countMoveMapBoxesAndNode (*pBoxTree->root, nMoveMapBoxes, nNodes);
    iNMoveMapBoxes = nMoveMapBoxes;
    iNTreeNodes = nNodes;
    iMoveMapBoxArray = new MoveMapBox[iNMoveMapBoxes];
    iTreeNodes = new TreeNode[iNTreeNodes];
    
    int moveMapBoxPos, treeNodePos;
    moveMapBoxPos = treeNodePos = 0;

    Vector3 lo = Vector3 (inf (), inf (), inf ());
    Vector3 hi = Vector3 (-inf (), -inf (), -inf ());
    Vector3 finalLo, finalHi;
    finalLo = lo;
    finalHi = hi;

    fillContainer (*pBoxTree->root, moveMapBoxPos, treeNodePos, lo, hi, finalLo, finalHi, pBoxPositionsTable);
    setBounds (finalLo, finalHi);
  }


  //=============================================================

  void
  MoveMapContainer::countMoveMapBoxesAndNode (AABSPTree<MoveMapBox*>::Node& pNode, int& nBoxes, int& nNodes)
  {
    nBoxes += pNode.valueArray.size ();
    ++nNodes;

    if (pNode.child[0] != 0)
      {
        countMoveMapBoxesAndNode (*pNode.child[0], nBoxes, nNodes);
      }
    if (pNode.child[1] != 0)
      {
        countMoveMapBoxesAndNode (*pNode.child[1], nBoxes, nNodes);
      }
  }

  //=============================================================

  /**
  Fill the static arrays and the table to have a mapping between MoveMapBox* and its pos in the array.
  We will need that table later on.
   */
  void
  MoveMapContainer::fillContainer (const AABSPTree<MoveMapBox *>::Node& pNode, int &pMoveMapBoxPos, int &pTreeNodePos, Vector3& pLo, Vector3& pHi, Vector3& pFinalLo, Vector3& pFinalHi, Table<MoveMapBox*, unsigned short>& pBoxPositionsTable)
  {
    TreeNode treeNode = TreeNode (pNode.valueArray.size (), pMoveMapBoxPos);
    treeNode.setSplitAxis (pNode.splitAxis);
    treeNode.setSplitLocation (pNode.splitLocation);
    int currentTreeNodePos = pTreeNodePos++;

    Vector3 lo = Vector3 (inf (), inf (), inf ());
    Vector3 hi = Vector3 (-inf (), -inf (), -inf ());

    for (int i = 0; i < pNode.valueArray.size (); i++)
      {
        G3D::_AABSPTree::Handle<MoveMapBox *>* h = pNode.valueArray[i];
        MoveMapBox* m = h->value;
        setMoveMapBox (*m, pMoveMapBoxPos);
        pBoxPositionsTable.set (m, pMoveMapBoxPos);
        ++pMoveMapBoxPos;

        const AABox& box = m->getAABoxBounds ();
        lo = lo.min (box.low ());
        hi = hi.max (box.high ());
        pFinalLo = pFinalLo.min (lo);
        pFinalHi = pFinalHi.max (hi);
      }

    if (pNode.child[0] != 0)
      {
        treeNode.setChildPos (0, pTreeNodePos);
        fillContainer (*pNode.child[0], pMoveMapBoxPos, pTreeNodePos, lo, hi, pFinalLo, pFinalHi, pBoxPositionsTable);
      }
    if (pNode.child[1] != 0)
      {
        treeNode.setChildPos (1, pTreeNodePos);
        fillContainer (*pNode.child[1], pMoveMapBoxPos, pTreeNodePos, lo, hi, pFinalLo, pFinalHi, pBoxPositionsTable);
      }

    pLo = pLo.min (lo);
    pHi = pHi.max (hi);

    treeNode.setBounds (lo, hi);
    setTreeNode (treeNode, currentTreeNodePos);
  }

  //=============================================================

  void
  MoveMapContainer::fillMoveMapConnectionManagerArray (const MoveLayerConnectionPointsContainer& pMoveLayerConnectionPointsContainer, const Table<MoveLayer*, MoveMapBox*>& pLayerMapBoxTable, const Table<MoveMapBox*, unsigned short>& pBoxPositionsTable)
  {
    Array<MoveLayer*> layerArray;
    pMoveLayerConnectionPointsContainer.getLayerKeys (layerArray);
    iMoveMapConnectionManager = new MoveMapConnectionManager[layerArray.size ()];
    for (int i = 0; i < layerArray.size (); ++i)
      {
        const Table<Vector3, MoveLayerConnectionPoints>& posConnTable = pMoveLayerConnectionPointsContainer.getVectorMoveLayerConnectionPointsTable (layerArray[i]);
        MoveMapConnectionManager moveMapConnectionManager (posConnTable, pLayerMapBoxTable, pBoxPositionsTable);
        iMoveMapConnectionManager[i] = moveMapConnectionManager;
      }
  }

  void
  MoveMapContainer::setMoveZonesContainer (AABox gridBounds)
  {
    iMoveZoneContainer = new MoveZoneContainer(iMoveMapBoxArray,iNMoveMapBoxes,gridBounds); 
  }


  const char MMAP_VERSION[] = "MMAP_1.00";
  extern std::string startCoordsPath;
  
  void
  MoveMapContainer::save (const char* pDir, const Vector3& pInnerLow, const Vector3& pInnerHigh, const Vector3& oriLow, const Vector3& oriHigh, unsigned int MapId, unsigned int mapx, unsigned int mapy, bool GenCoords)
  {
    char mmapname[15];
    sprintf (mmapname, "/%03u_%02u_%02u.mmap", MapId, mapx, mapy);
    std::string nName, nName2;
    nName = pDir + (std::string)mmapname;

    FILE *output = fopen (nName.c_str (), "wb");

    // write version header
    fwrite (MMAP_VERSION, 1, 9, output);

    double full = 64.0 * 533.33333333;
    double mid = full / 2.0;

    fwrite (&iNMoveMapBoxes, 1, 4, output);

    for (unsigned int i = 0; i < iNMoveMapBoxes; ++i)
      {
        MoveMapBox *iTmp = &iMoveMapBoxArray[i];
        fwrite (&iTmp->getBounds ().low ().x, 4, 1, output);
        fwrite (&iTmp->getBounds ().low ().y, 4, 1, output);
        fwrite (&iTmp->getBounds ().low ().z, 4, 1, output);
        fwrite (&iTmp->getBounds ().high ().x, 4, 1, output);
        fwrite (&iTmp->getBounds ().high ().y, 4, 1, output);
        fwrite (&iTmp->getBounds ().high ().z, 4, 1, output);
        unsigned int maxX, maxY;
        maxX = iTmp->getSizeX ();
        fwrite (&maxX, 4, 1, output);
        maxY = iTmp->getSizeY ();
        fwrite (&maxY, 4, 1, output);
        const Vector3 basePos = iTmp->getBasePosition ();
        
        
        Table<Vector2,Array<Vector3> > pPosTable;
        if (GenCoords)
          {
          
          // read already known starting points
          // record rounded to avoid float comparison error
          for(unsigned int i=0;i<4;++i)
            {
            int x_val = mapx;
            int y_val = mapy;
            Array<Vector3> pPosArray;
            switch(i)
              {
              case 0:
                x_val++;
                break;
              case 1:
                x_val--;
                break;
              case 2:
                y_val++;
                break;
              case 3:
                y_val--;
                break;
              }
            char coordname[255];
            sprintf (coordname, "%s/%03u_%02u_%02u.txt",startCoordsPath.c_str(), MapId, x_val, y_val);
            FILE* f = fopen (coordname, "rb");
            if (f)
              {
              
              int bufferSize = 500;
              char buffer[500];
              while (fgets (buffer, bufferSize - 1, f))
                {
                float y, x, z;
                sscanf (buffer, "%f,%f,%f", &x, &y, &z); //"%d, %d, %d, %f, %f, %f", &map, &tilex, &tiley, &x, &y, &z);
                pPosArray.push_back (Vector3 ((int)(1000*x), (int)(1000*y), (int)z));
                }
              fclose (f);
              }
              
            sprintf (coordname, "%s/%03u_%02u_%02u_run.txt",startCoordsPath.c_str(), MapId, x_val, y_val);
            f = fopen (coordname, "rb");
            if (f)
              {
              
              int bufferSize = 500;
              char buffer[500];
              while (fgets (buffer, bufferSize - 1, f))
                {
                float y, x, z;
                sscanf (buffer, "%f,%f,%f", &x, &y, &z); //"%d, %d, %d, %f, %f, %f", &map, &tilex, &tiley, &x, &y, &z);
                pPosArray.push_back (Vector3 ((int)(1000*x), (int)(1000*y), (int)z));
                }
              fclose (f);
              }
              
            pPosTable.set(Vector2(x_val,y_val),pPosArray);
            }
          }

/* DEBUG :        
        printf("B low %f,%f high %f,%f\n",-((mid + oriLow.x) - full),-((mid + oriLow.z) - full),-((mid + oriHigh.x) - full),-((mid + oriHigh.z) - full));
        printf("S low %f,%f high %f,%f\n",-((mid + basePos.x) - full),-((mid + basePos.z) - full),-((mid + basePos.x+maxX) - full),-((mid + basePos.z+maxY) - full));
*/
        for (unsigned int x = 0; x < maxX; ++x)
          {
            for (unsigned int y = 0; y < maxY; ++y)
              {
                unsigned int val = iTmp->get ((float) x, (float) y);
                if (GenCoords && val != MOVEMAP_VALUE_CANT_REACH)
                  {
                    float cx = basePos.x + x;
                    float cy = basePos.z + y;

                    //if (cx < oriLow.x+1.0f || cx > oriHigh.x-2.0f || cy < oriLow.z+1.0f || cy > oriHigh.z-2.0f)
                    int x_val = mapx;
                    int y_val = mapy;
                    if (cx < oriLow.x)
                      y_val = mapy - 1;
                    if (cy < oriLow.z)
                      x_val = mapx - 1;
                    if (cx > oriHigh.x)
                      y_val = mapy + 1;
                    if (cy > oriHigh.z)
                      x_val = mapx + 1;

                    //if (cx < oriLow.x-1 || cx > oriHigh.x+1 || cy < oriLow.z-1 || cy > oriHigh.z+1)
                    if (x_val == mapx && y_val == mapy && (abs(cx - oriLow.x) < 0.01f || abs(cy - oriLow.z) < 0.01f || abs( cx - oriHigh.x) < 0.01f || abs(cy - oriHigh.z) < 0.01f) )
                      {
                        char coordname[15];

                        float mangosx, mangosy;
                        mangosx = -((mid + cy) - full);
                        mangosy = -((mid + cx) - full);
                        
                        sprintf (coordname, "%03u_%02u_%02u_run.txt", MapId, x_val, y_val);
                        nName = startCoordsPath + "/" + (std::string)coordname;
                        FILE *Coord = fopen (nName.c_str (), "ab");

                        fprintf (Coord, "%f,%f,%f\n", mangosx, mangosy, iTmp->getFloatHeight (val));
                        fclose (Coord);
                        }

                    if (x_val != mapx || y_val != mapy)
                      {
                        char coordname[15];

                        float mangosx, mangosy;
                        mangosx = -((mid + cy) - full);
                        mangosy = -((mid + cx) - full);
                        
                        if (pPosTable.containsKey(Vector2(x_val,y_val))) // corner grids are not handeld for now
                          {
                          Array<Vector3> pPosArray = pPosTable.get(Vector2(x_val,y_val));
                          if (!pPosArray.contains(Vector3((int)(1000*mangosx), (int)(1000*mangosy), (int)(iTmp->getFloatHeight (val)))))
                            {
                            //double x_offset = (double(mangosx) - (533.33333f / 2)) / 533.33333f;
                            //double y_offset = (double(mangosy) - (533.33333f / 2)) / 533.33333f;
                            //int x_val = 63 - int(x_offset + 32 + 0.5);
                            //int y_val = 63 - int(y_offset + 32 + 0.5);
                            sprintf (coordname, "%03u_%02u_%02u.txt", MapId, x_val, y_val);
                            nName = startCoordsPath + "/" + (std::string)coordname;
                            FILE *Coord = fopen (nName.c_str (), "ab");

                            fprintf (Coord, "%f,%f,%f\n", mangosx, mangosy, iTmp->getFloatHeight (val));
                            fclose (Coord);
                          }
                        }
                      }
                  }
                fwrite (&val, 1, 1, output);
              }
          }
      }

    fwrite (&iNTreeNodes, 4, 1, output);

    for (unsigned int i = 0; i < iNTreeNodes; ++i)
      {
        TreeNode *iTmp = &iTreeNodes[i];
        AABox pBox;
        unsigned int ui;
        float f;
        int it;
        unsigned short us;
        iTmp->getBounds (pBox);
        fwrite (&pBox.low ().x, 4, 1, output);
        fwrite (&pBox.low ().y, 4, 1, output);
        fwrite (&pBox.low ().z, 4, 1, output);
        fwrite (&pBox.high ().x, 4, 1, output);
        fwrite (&pBox.high ().y, 4, 1, output);
        fwrite (&pBox.high ().z, 4, 1, output);
        ui = iTmp->getStartPosition ();
        fwrite (&ui, 4, 1, output);
        f = iTmp->getSplitLocation ();
        fwrite (&f, 4, 1, output);
        ui = iTmp->getSplitAxis ();
        fwrite (&ui, 4, 1, output);
        it = iTmp->getChildval (0);
        fwrite (&it, 4, 1, output);
        it = iTmp->getChildval (1);
        fwrite (&it, 4, 1, output);
        us = iTmp->getNValues ();
        fwrite (&us, 2, 1, output);
      }

      iMoveZoneContainer->save(output);
      /*
      int NZones = iMoveZoneContainer->getNZones();
      fwrite (&NZones, 4, 1, output);
       
      for (unsigned int j=0; j < NZones; j++)
      {
        const MoveZone* iZone = iMoveZoneContainer->getZone(j);
        fwrite (&iZone->getBounds ().low ().x, 4, 1, output);
        fwrite (&iZone->getBounds ().low ().y, 4, 1, output);
        fwrite (&iZone->getBounds ().low ().z, 4, 1, output);
        fwrite (&iZone->getBounds ().high ().x, 4, 1, output);
        fwrite (&iZone->getBounds ().high ().y, 4, 1, output);
        fwrite (&iZone->getBounds ().high ().z, 4, 1, output);
        
        Array<MovePortal*> Portals;
        Portals = iZone->getPortalArray();
        unsigned int pArraySize = Portals.size();
        fwrite (&pArraySize, 4, 1, output);
        for (unsigned int p=0; p<pArraySize; ++p)
          {
          unsigned int pdest = Portals[p]->getDestinationID();
          fwrite (&pdest, 4, 1, output);
          unsigned int pdir = Portals[p]->getDirection();
          fwrite (&pdir, 4, 1, output);
          fwrite (&Portals[p]->getLow().x, 4, 1, output);
          fwrite (&Portals[p]->getLow().y, 4, 1, output);
          fwrite (&Portals[p]->getLow().z, 4, 1, output);
          fwrite (&Portals[p]->getHigh().x, 4, 1, output);
          fwrite (&Portals[p]->getHigh().y, 4, 1, output);
          fwrite (&Portals[p]->getHigh().z, 4, 1, output);
          }
      }
    printf ("Saved MoveZones : %d\n", NZones);*/

    fclose (output);

    for (unsigned int direction = 0; direction<4; ++direction)
      {
      //const Table<unsigned int*, float>* iGridPortals=iMoveZoneContainer->getGridPortals(direction);
      const Array<gridPortal>* iGridPortals=iMoveZoneContainer->getGridPortals(direction);
      if (iGridPortals->size() > 0)
        {
        int x_val = mapx;
        int y_val = mapy;

        switch (direction)
          {
          // grid coords are YX
          case EXTEND_N :
            x_val+=1;
          break;
          case EXTEND_S :
            x_val-=1;
          break;
          case EXTEND_E :
            y_val+=1;
          break;
          case EXTEND_W :
            y_val-=1;
          break;
          }
        char filename[15];
        sprintf (filename, "grid_cnx_%03u_%02u_%02u_%02u_%02u.tmp", MapId, mapx, mapy, x_val, y_val);
        nName = startCoordsPath + "/" + (std::string)filename;
        FILE *GridCnx = fopen (nName.c_str (), "wb");
        for(Array<gridPortal>::ConstIterator itr= iGridPortals->begin();itr != iGridPortals->end();++itr)
          {
          //fprintf (GridCnx, "%u,%u,%f\n", itr->key[0], itr->key[1], itr->value);
          fprintf (GridCnx, "%u,%f,%f,%f\n", itr->MoveZoneId, itr->fromx, itr->fromy, itr->destz);
          printf("writing %u,%f,%f,%f to %03u_%02u_%02u_%02u_%02u\n", itr->MoveZoneId, itr->fromx, itr->fromy, itr->destz, MapId, mapx, mapy, x_val, y_val);
          }
        fclose (GridCnx);
        }
      }


    printf ("Mmap saved (%u iNMoveMapBoxes and %u iNTreeNodes)\n", iNMoveMapBoxes, iNTreeNodes);
  }

  void
  MoveMapContainer::load (const char* pDir, const char* tname)
  {
    std::string nName = pDir;
    nName += "/" ;
    nName += (std::string)tname ;
    nName += ".mmap";

    FILE *input = fopen (nName.c_str (), "rb");
    if (!input)
      {
        printf ("Can't open the input file '%s'\n", nName.c_str ());
        return;
      }
    unsigned int nb;
    // read header version
    char dummy[9];
    fread (&dummy, 1, 9, input);

    fread (&nb, 4, 1, input);
    iNMoveMapBoxes = nb;
    printf ("Red iNMoveMapBoxes : %u\n", iNMoveMapBoxes);
    iMoveMapBoxArray = new MoveMapBox[iNMoveMapBoxes];
    for (unsigned int i = 0; i < iNMoveMapBoxes; ++i)
      {
        Vector3 v1;
        Vector3 v2;
        fread (&v1.x, 4, 1, input);
        fread (&v1.y, 4, 1, input);
        fread (&v1.z, 4, 1, input);
        fread (&v2.x, 4, 1, input);
        fread (&v2.y, 4, 1, input);
        fread (&v2.z, 4, 1, input);
        AABox pBox (v1, v2);
        unsigned int maxX, maxY;
        int val;
        fread (&maxX, 4, 1, input);
        fread (&maxY, 4, 1, input);
        //printf("MaxX: %u, MaxY: %u\n",maxX,maxY);
        MoveMapBox* currentMoveMapBox = new MoveMapBox (maxX, maxY);
        for (unsigned int x = 0; x < maxX; ++x)
          {
            for (unsigned int y = 0; y < maxY; ++y)
              {
                val = 0;
                fread (&val, 1, 1, input);
                currentMoveMapBox->set (val, (float) x, (float) y);
              }
          }
        currentMoveMapBox->setBounds (pBox);
        iMoveMapBoxArray[i] = *currentMoveMapBox;
      }
    fread (&iNTreeNodes, 4, 1, input);
    printf ("Red iNTreeNodes : %u\n", iNTreeNodes);
    iTreeNodes = new TreeNode[iNTreeNodes];
    for (unsigned int i = 0; i < iNTreeNodes; ++i)
      {
        TreeNode iTmp;
        unsigned int sp, ax;
        float sl;
        int ch0, ch1;
        unsigned short nval;
        Vector3 v1;
        Vector3 v2;
        fread (&v1.x, 4, 1, input);
        fread (&v1.y, 4, 1, input);
        fread (&v1.z, 4, 1, input);
        fread (&v2.x, 4, 1, input);
        fread (&v2.y, 4, 1, input);
        fread (&v2.z, 4, 1, input);
        AABox pBox (v1, v2);


        fread (&sp, 4, 1, input);
        fread (&sl, 1, 4, input);
        fread (&ax, 4, 1, input);
        fread (&ch0, 4, 1, input);
        fread (&ch1, 4, 1, input);
        fread (&nval, 2, 1, input);

        TreeNode* CurrentNode = new TreeNode (nval, sp);
        CurrentNode->setStartPosition (sp);
        CurrentNode->setSplitLocation (sl);
        CurrentNode->setSplitAxis (Vector3::Axis (ax));
        CurrentNode->setChildval (0, ch0);
        CurrentNode->setChildval (1, ch1);
        CurrentNode->setBounds (pBox);

        iTreeNodes[i] = *CurrentNode;

      }

      iMoveZoneContainer = new MoveZoneContainer();
      printf("Loading MoveZoneContainer:\n");
      iMoveZoneContainer->load(input);
      /*
      fread (&nb, 4, 1, input);
      //iTmp.setNZones(nb);
      for (unsigned int j=0; j < nb; j++)
        {
        Vector3 v1;
        Vector3 v2;
        fread (&v1.x, 4, 1, input);
        fread (&v1.y, 4, 1, input);
        fread (&v1.z, 4, 1, input);
        fread (&v2.x, 4, 1, input);
        fread (&v2.y, 4, 1, input);
        fread (&v2.z, 4, 1, input);
        
        int nPortals;
        fread (&nPortals, 4, 1, input);
        Array<MovePortal*> * PArray = new Array<MovePortal*>();
        for (unsigned int p=0; p<nPortals; ++p)
            {
            unsigned int destID;
            fread (&destID, 4, 1, input);
            unsigned int direction;
            fread (&direction, 4, 1, input);
            Vector3 low;
            fread (&low.x, 4, 1, input);
            fread (&low.y, 4, 1, input);
            fread (&low.z, 4, 1, input);
            Vector3 high;
            fread (&high.x, 4, 1, input);
            fread (&high.y, 4, 1, input);
            fread (&high.z, 4, 1, input);
            
            MovePortal * portal = new MovePortal(low,high,destID,direction);
            //DEBUG: printf("portal at %f,%f %f\n",portal->getLow().x,portal->getLow().y,portal->getLow().z);
            PArray->append(portal);
            }
        
        //DEBUG: printf("zone load at %f,%f \n",v1.x,v1.y);
        AABox pBox (v1, v2);
        iMoveZoneContainer->setZone(&pBox,j,PArray);
        }
      printf ("Red MoveZones : %u\n", nb);*/

      //iMoveZoneContainer = iTmp; // FIXME : should be a AABSPTree like iTreeNodes
      printf ("zoneContainer loaded : %i zones\n",iMoveZoneContainer->getNZones());
    
    fclose (input);
    printf ("load done.\n");
  }
}
