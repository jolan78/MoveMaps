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
    // is this function usefull ? :
    assert(false);
    int result = -1;
    
    MoveMapConnctionHandle* foundElem = (MoveMapConnctionHandle*) bsearch (&pSearchVaue, &iMoveMapConnctionHandle[pIndex], pSize, sizeof (MoveMapConnctionHandle), &connectionHandleCompare);
    if (foundElem != 0)
      {
        result = foundElem->getIndex ();
      }
    return (result);
  }

  //=============================================================

  /**
   * return the index or -1 if no connection was found
   */
  int
  MoveMapConnectionManager::findMapIndex (const Vector3& pPos) const
  {
    // is this function usefull ? :
    assert(false);

    int result = -1;
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
    //iTreeNodes = NULL;
    iMoveMapConnectionManager = NULL;
    iMoveZoneContainer = NULL;
  }

  //=============================================================

  MoveMapContainer::~MoveMapContainer ()
  {
    if (iMoveMapBoxArray != NULL) delete[] iMoveMapBoxArray;
    if (iMoveMapConnectionManager != NULL) delete iMoveMapConnectionManager;
    if (iMoveZoneContainer != NULL) delete iMoveZoneContainer;
  }

  //=============================================================

  /**
  Fill the static arrays and the table to have a mapping between MoveMapBox* and its pos in the array.
  We will need that table later on.
   */

  MoveMapContainer::MoveMapContainer (Array<MoveMapBox*>* pBoxTree, Table<MoveMapBox*, unsigned short>& pBoxPositionsTable)
  {
    
    iNMoveMapBoxes = pBoxTree->size();
    iMoveMapBoxArray = new MoveMapBox[iNMoveMapBoxes];
    
    int moveMapBoxPos;
    moveMapBoxPos = 0;

    Vector3 lo = Vector3 (inf (), inf (), inf ());
    Vector3 hi = Vector3 (-inf (), -inf (), -inf ());
    Vector3 finalLo, finalHi;
    finalLo = lo;
    finalHi = hi;

    fillContainer (pBoxTree, moveMapBoxPos, lo, hi, finalLo, finalHi, pBoxPositionsTable);
    setBounds (finalLo, finalHi);
  }

  //=============================================================

  /**
  Fill the static arrays and the table to have a mapping between MoveMapBox* and its pos in the array.
  We will need that table later on.
   */
  void
  MoveMapContainer::fillContainer (Array<MoveMapBox*>* pBoxTree, int &pMoveMapBoxPos, Vector3& pLo, Vector3& pHi, Vector3& pFinalLo, Vector3& pFinalHi, Table<MoveMapBox*, unsigned short>& pBoxPositionsTable)
  {
    Vector3 lo = Vector3 (inf (), inf (), inf ());
    Vector3 hi = Vector3 (-inf (), -inf (), -inf ());

    for (int i = 0; i < pBoxTree->size (); i++)
      {
        
        MoveMapBox* m = (*pBoxTree)[i];
        setMoveMapBox (*m, pMoveMapBoxPos);
        pBoxPositionsTable.set (m, pMoveMapBoxPos);
        ++pMoveMapBoxPos;

        const AABox& box = m->getAABoxBounds ();
        lo = lo.min (box.low ());
        hi = hi.max (box.high ());
        pFinalLo = pFinalLo.min (lo);
        pFinalHi = pFinalHi.max (hi);
      }

    pLo = pLo.min (lo);
    pHi = pHi.max (hi);
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
    MoveZoneContainerGenerator* MZG = new MoveZoneContainerGenerator(iMoveMapBoxArray,iNMoveMapBoxes,gridBounds);
    iMoveZoneContainer = (MoveZoneContainer*)MZG;
    
  }


  const char MMAP_VERSION[] = "MMAP_1.00";
  extern std::string startCoordsPath;

  void
  MoveMapContainer::saveGridCnx (unsigned int MapId, unsigned int mapx, unsigned int mapy)
  {
  std::string nName;
  for (unsigned int direction = 0; direction<4; ++direction)
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
    char filename[1024];
    sprintf (filename, "grid_cnx_%03u_%02u_%02u_%02u_%02u.tmp", MapId, mapx, mapy, x_val, y_val);
    printf("Processing %s", filename);
    nName = startCoordsPath + "/" + std::string(filename);
    ((MoveZoneContainerGenerator*)iMoveZoneContainer)->saveGridCnx(nName.c_str (),direction);
    }
  }
  
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
                sscanf (buffer, "%f,%f,%f", &x, &y, &z);
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
                sscanf (buffer, "%f,%f,%f", &x, &y, &z);
                pPosArray.push_back (Vector3 ((int)(1000*x), (int)(1000*y), (int)z));
                }
              fclose (f);
              }
              
            pPosTable.set(Vector2(x_val,y_val),pPosArray);
            }
          }

        for (unsigned int x = 0; x < maxX; ++x)
          {
            for (unsigned int y = 0; y < maxY; ++y)
              {
                unsigned int val = iTmp->get ((float) x, (float) y);
                if (GenCoords && val != MOVEMAP_VALUE_CANT_REACH)
                  {
                    float cx = basePos.x + x;
                    float cy = basePos.z + y;

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

    iMoveZoneContainer->save(output);

    fclose (output);

    printf ("Mmap saved (%u iNMoveMapBoxes)\n", iNMoveMapBoxes);
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

      iMoveZoneContainer = new MoveZoneContainer();
      iMoveZoneContainer->load(input);

      printf ("zoneContainer loaded : %i zones\n",iMoveZoneContainer->getNZones());
    
    fclose (input);
    printf ("load done.\n");
  }
}
