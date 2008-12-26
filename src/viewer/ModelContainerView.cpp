
#include "G3D/Array.h"


#include "G3D/Box.h"


#include "G3D/Array.h"

#include "ModelContainerView.h"
#include "Config/Config.h"

using namespace G3D;

namespace VMAP
{
  extern std::string startCoordsPath;
  extern std::string gDataDir;
  extern std::string gMMapDataDir;
  extern std::string gvMapDataDir;
  extern std::string g3dDataDir;


  //==========================================

  ModelContainerView::ModelContainerView (const G3D::GApp::Settings& settings, int mapId, int x, int y) :
  GApp (settings),
  iVMapManager ()
  {
    i_App = this;
    iShowSky = true;
    iShowMMaps = true;
    iShowVMaps = true;
    iShowBoxes = false;

    iVARAreaRef = VARArea::create (settings.window.width * settings.window.height * 60);

    iMap = mapId;
    ix = x;
    iy = y;

  }
  //===================================================

  ModelContainerView::~ModelContainerView (void)
  {
    Array<std::string > keys = iTriVarTable.getKeys ();
    Array<std::string>::ConstIterator i = keys.begin ();
    while (i != keys.end ())
      {
        VAR* var = iTriVarTable.get (*i);
        delete var;
        ++i;
      }
  }

  //===================================================

  void
  ModelContainerView::cleanup ()
  {
  }

  //===================================================
  
  void
  ModelContainerView::init ()
  {
    // Show the vmaps
    loadAndShowTile (iMap, ix, iy);
    
    // Set the camera near the scene
    ManagedModelContainer* mc = 0;
    std::string dirFileName = iVMapManager.getDirFileName (iMap, ix, iy);
    MapTree* mt = iVMapManager.getInstanceMapTree (iMap);
    if (!mt->hasDirFile (dirFileName))
      dirFileName = iVMapManager.getDirFileName (iMap);
    if (mt->hasDirFile (dirFileName))
      {
        Array<std::string> fileNames = mt->getDirFiles (dirFileName).getFiles ();
        for (int i = 0; i < fileNames.size (); ++i)
          {
            std::string name = fileNames[i];
            mc = mt->getModelContainer (name);
            break;
          }
      }
    if (mc != 0)
      {
        const SubModel& sm = mc->getSubModel (0);
        Vector3 vpos = (sm.getAABoxBounds ().low ());
        setViewPosition (vpos);
      }

    // show the mmaps
    addMoveMapToDisplay (iMap, ix, iy);
  }

  //==========================================

  void
  fillSubModelArary (const ModelContainer* pModelContainer, const TreeNode *root, Array<SubModel>& array, Vector3& pLo, Vector3& pHi)
  {
    Vector3 lo = Vector3 (inf (), inf (), inf ());
    Vector3 hi = Vector3 (-inf (), -inf (), -inf ());

    for (int i = 0; i < root->getNValues (); i++)
      {
        SubModel sm = pModelContainer->getSubModel (root->getStartPosition () + i);
        lo = lo.min (sm.getAABoxBounds ().low ());
        hi = hi.max (sm.getAABoxBounds ().high ());
        array.append (sm);
      }

    if (root->getChild ((TreeNode *) & pModelContainer->getTreeNode (0), 0))
      {
        fillSubModelArary (pModelContainer, root->getChild ((TreeNode *) & pModelContainer->getTreeNode (0), 0), array, lo, hi);
      }
    if (root->getChild ((TreeNode *) & pModelContainer->getTreeNode (0), 1))
      {
        fillSubModelArary (pModelContainer, root->getChild ((TreeNode *) & pModelContainer->getTreeNode (0), 1), array, lo, hi);
      }

    float dist1 = (hi - lo).magnitude ();
    AABox b;
    root->getBounds (b);
    float dist2 = (b.high () - b.low ()).magnitude ();
    if (dist1 > dist2)
      {
        // error
        int xxx = 0;
      }
  }

  //==========================================

  void
  ModelContainerView::addModelContainer (const std::string& pName, const ModelContainer* pModelContainer )
  {
    // VARArea::UsageHint::WRITE_EVERY_FEW_FRAMES

    int offset = 0;

    Array<int> iIndexArray;
    Array<Vector3> iGlobArray;

    Array<SubModel> SMArray;
    Vector3 lo, hi;
    fillSubModelArary (pModelContainer, &pModelContainer->getTreeNode (0), SMArray, lo, hi);


    for (int i = 0; i < SMArray.size (); ++i)
      {
        SubModel sm = SMArray[i];
        Array<Vector3> vArray;
        Array<int> iArray;
        fillVertexAndIndexArrays (sm, vArray, iArray);

        for (int j = 0; j < iArray.size (); ++j)
          {
            iIndexArray.append (offset + iArray[j]);

          }
        for (int j = 0; j < vArray.size (); ++j)
          {
            iGlobArray.append (vArray[j]);
          }
        offset += vArray.size ();
        //break;
      }
    iTriVarTable.set (pName, new VAR (iGlobArray, iVARAreaRef));
    iTriIndexTable.set (pName, iIndexArray);
  }

  //===================================================

  void
  ModelContainerView::onInit ()
  {
    // Called before the application loop beings.  Load data here and
    // not in the constructor so that common exceptions will be
    // automatically caught.
    
    //SKY CODE
    iSky = Sky::fromFile(g3dDataDir + "/sky/", "plainsky/null_plainsky512_*.jpg");
    iSkyParameters = SkyParameters (toSeconds(9, 00, 00, AM));
    
    debugWindow->setVisible (true);

    toneMap->setEnabled (false);
    init ();
  }

  void
  ModelContainerView::onGraphics (RenderDevice* rd, Array<PosedModelRef> &posed3D, Array<PosedModel2DRef> &posed2D)
  {
    rd->setProjectionAndCameraMatrix (defaultCamera);

    rd->setAmbientLightColor (Color4 (Color3::white ()));

    rd->setColorClearValue (Color3::black ());
    rd->clear ();
    
    //SKY CODE
    if (iShowSky)
      iSky->render (rd, iSkyParameters);

    // Setup lighting
    rd->enableLighting ();
    
    GLight light = GLight::directional (defaultController.pointer ()->position () + defaultController.pointer ()->lookVector ()*2, Color3::white ());
    rd->setLight (0, light);

    Array<std::string > keys = iTriVarTable.getKeys ();
    Array<std::string>::ConstIterator i = keys.begin ();
    while (i != keys.end ())
      {
        VAR* var = iTriVarTable.get (*i);
        Array<int> indexArray = iTriIndexTable.get (*i);
        const std::string& name = *i;
        if ((name.compare ("moveMaps") == 0) || (name.compare ("moveMaps2") == 0))
          {
            if (iShowMMaps)
              {
                rd->setColor (Color3::red ());
                rd->beginIndexedPrimitives ();
                rd->setVertexArray (*var);
                rd->sendIndices (RenderDevice::QUADS, indexArray);
                rd->endIndexedPrimitives ();
              }
          }
        else if (name.compare ("boundingBoxes") == 0)
          {
            if (iShowBoxes)
              {
                rd->setColor (Color3::green ());
                rd->beginIndexedPrimitives ();
                rd->setVertexArray (*var);
                rd->sendIndices (RenderDevice::QUADS, indexArray);
                rd->endIndexedPrimitives ();
              }
          }
        else
          {
            if (iShowVMaps)
              {
                rd->setColor (Color3::blue ());
                rd->beginIndexedPrimitives ();
                rd->setVertexArray (*var);
                rd->sendIndices (RenderDevice::LINES, indexArray);
                rd->endIndexedPrimitives ();
              }
          }
        ++i;
      }
    
    //SKY CODE
    if (iShowSky)
      iSky->renderLensFlare(rd, iSkyParameters);
    
    rd->disableLighting ();
    
    PosedModel2D::sortAndRender (rd, posed2D);
  }

  //===================================================

  void
  ModelContainerView::fillRenderArray (const SubModel& pSm, Array<TriangleBox> &pArray, const TreeNode* pTreeNode)
  {
    for (int i = 0; i < pTreeNode->getNValues (); i++)
      {
        pArray.append (pSm.getTriangles ()[i + pTreeNode->getStartPosition ()]);
      }

    if (pTreeNode->getChild (pSm.getTreeNodes (), 0) != 0)
      {
        fillRenderArray (pSm, pArray, pTreeNode->getChild (pSm.getTreeNodes (), 0));
      }

    if (pTreeNode->getChild (pSm.getTreeNodes (), 1) != 0)
      {
        fillRenderArray (pSm, pArray, pTreeNode->getChild (pSm.getTreeNodes (), 1));
      }
  }

  
  //===================================================

  void
  ModelContainerView::addMoveMapToDisplay ( int mapId, int x, int y )
  {
    // Create a MoveMapContainer that represents the tile
    // Todo later maybe store the container
    MoveMapContainer moveMapBoxContainer;
    // Show the bounding boxes of the MoveMapBoxes
    Array<Box> gBoxArray;
    
    // for the file name
    char buffer[50];
    
    // index and vertex arrays to fill
    Array<int> iArray;
    Array<Vector3> vArray;
    
    // actually the points are a little squares :)
    float diff = 0.1f;
    Vector3 diff1 (-diff, 0.0f, -diff);
    Vector3 diff2 (-diff, 0.0f, diff);
    Vector3 diff3 (diff, 0.0f, diff);
    Vector3 diff4 (diff, 0.0f, -diff);
    int count = 0;
    
    sprintf (buffer, "%03u_%02u_%02u", mapId, x, y);
    moveMapBoxContainer.load (gMMapDataDir.c_str(),buffer);

    for (unsigned int i = 0; i < moveMapBoxContainer.getNMoveMapBoxes (); ++i)
      {
        const MoveMapBox& moveMapBox = moveMapBoxContainer.getMoveMapBox (i);
        const Vector3 basePos = moveMapBox.getBasePosition ();

        // draw the move map box for debugging
        const AABox& b = moveMapBox.getAABoxBounds();
        gBoxArray.push_back(Box(b));

        for (unsigned int x = 0; x < moveMapBox.getSizeX (); ++x)
          {
            for (unsigned int z = 0; z < moveMapBox.getSizeY (); ++z)
              {
                unsigned int val = moveMapBox.get ((float) x, (float) z);
                if (val != MOVEMAP_VALUE_CANT_REACH)
                  {
                    float height = moveMapBox.getFloatHeight (val);
                    Vector3 v (basePos.x + x, height, basePos.z + z);
                    vArray.append (v + diff1);
                    vArray.append (v + diff2);
                    vArray.append (v + diff3);
                    vArray.append (v + diff4);

                    iArray.append (count + 0);
                    iArray.append (count + 1);
                    iArray.append (count + 2);
                    iArray.append (count + 3);
                    count += 4;
                  }
              }
          }
      }

    std::string name ("moveMaps2");
    
    iTriVarTable.set (name, new VAR (vArray, iVARAreaRef));
    iTriIndexTable.set (name, iArray);
    
    // Add the Bounding boxes
    Array<int> ibArray;
    Array<Vector3> vbArray;
    name = "boundingBoxes";
    count = 0;
    
    for(Array<Box>::Iterator iter = gBoxArray.begin (); iter != gBoxArray.end (); ++iter)
      {
        const Box& b = *iter;
        for (int i = 0; i < 8; i++)
          vbArray.append (b.corner (i));

        // side 1
        ibArray.append (count);
        ibArray.append (count + 1);
        ibArray.append (count + 2);
        ibArray.append (count + 3);

        // side 1 back
        ibArray.append (count + 3);
        ibArray.append (count + 2);
        ibArray.append (count + 1);
        ibArray.append (count);
        
        // side 2
        ibArray.append (count + 4);
        ibArray.append (count + 5);
        ibArray.append (count + 6);
        ibArray.append (count + 7);
        
        // side 2 back
        ibArray.append (count + 7);
        ibArray.append (count + 6);
        ibArray.append (count + 5);
        ibArray.append (count + 4);
        
        // side 3
        ibArray.append (count + 1);
        ibArray.append (count + 2);
        ibArray.append (count + 6);
        ibArray.append (count + 5);
        
        // side 3 back
        ibArray.append (count + 5);
        ibArray.append (count + 6);
        ibArray.append (count + 2);
        ibArray.append (count + 1);
        
        // side 4
        ibArray.append (count + 0);
        ibArray.append (count + 3);
        ibArray.append (count + 7);
        ibArray.append (count + 4);
        
        // side 4 back
        ibArray.append (count + 4);
        ibArray.append (count + 7);
        ibArray.append (count + 3);
        ibArray.append (count + 0);
        
        // side 5
        ibArray.append (count + 2);
        ibArray.append (count + 3);
        ibArray.append (count + 7);
        ibArray.append (count + 6);
        
        // side 5 back
        ibArray.append (count + 6);
        ibArray.append (count + 7);
        ibArray.append (count + 3);
        ibArray.append (count + 2);
        
        // side 6
        ibArray.append (count + 0);
        ibArray.append (count + 1);
        ibArray.append (count + 5);
        ibArray.append (count + 4);
        
        // side 6 back
        ibArray.append (count + 4);
        ibArray.append (count + 5);
        ibArray.append (count + 1);
        ibArray.append (count + 0);

        count += 8;
      }
    
    iTriVarTable.set (name, new VAR (vbArray, iVARAreaRef));
    iTriIndexTable.set (name, ibArray);
  }
  
  //===================================================

  void
  ModelContainerView::fillVertexAndIndexArrays (const SubModel& pSm, Array<Vector3>& vArray, Array<int>& iArray )
  {
    Array<TriangleBox> tbarray;

    fillRenderArray (pSm, tbarray, &pSm.getTreeNode (0));
    MeshBuilder builder;
    int len = tbarray.size ();
    int count = 0;
    for (int i = 0; i < len; ++i)
      {
        Triangle t = Triangle (tbarray[i].vertex (0).getVector3 () + pSm.getBasePosition (),
                               tbarray[i].vertex (1).getVector3 () + pSm.getBasePosition (),
                               tbarray[i].vertex (2).getVector3 () + pSm.getBasePosition ());

        vArray.append (t.vertex (0));
        vArray.append (t.vertex (1));
        vArray.append (t.vertex (2));

        iArray.append (count + 0);
        iArray.append (count + 1);
        iArray.append (count + 1);
        iArray.append (count + 2);
        iArray.append (count + 2);
        iArray.append (count + 0);
        count += 3;
      }
  }

  //====================================================================

  void
  ModelContainerView::showMap (int pMapId, int x, int y)
  {
    MapTree* mt = iVMapManager.getInstanceMapTree (pMapId);
    std::string dirFileName = iVMapManager.getDirFileName (pMapId, x, y);
    
    if (!mt->hasDirFile (dirFileName))
      dirFileName = iVMapManager.getDirFileName (pMapId);
    
    showMap (mt, dirFileName);
  }

  //====================================================================

  bool
  ModelContainerView::loadAndShowTile (int pMapId, int x, int y)
  {
    std::string vmapdir = gDataDir + "/vmaps";
    bool result = iVMapManager.loadMap (vmapdir.c_str (), pMapId, x, y) == VMAP_LOAD_RESULT_OK;
    if (result == VMAP_LOAD_RESULT_OK)
      {
        showMap (pMapId, x, y);
      }
    return (result);
  }

  //=======================================================================

  void
  ModelContainerView::showMap (MapTree* mt, std::string dirFileName)
  {
    if (mt->hasDirFile (dirFileName))
      {
        FilesInDir filesInDir = mt->getDirFiles (dirFileName);
        if (filesInDir.getRefCount () == 1)
          {
            Array<std::string> fileNames = filesInDir.getFiles ();
            for (int i = 0; i < fileNames.size (); ++i)
              {
                std::string name = fileNames[i];

                ManagedModelContainer* mc = mt->getModelContainer (name);
                //if(mc->getNSubModel() == 791) {
                addModelContainer (name, mc );
                //}
              }
          }
      }
  }

  //=======================================================================

  bool
  ModelContainerView::loadAndShowTile (char *pName, int pMapId)
  {
    std::string vmapdir = gDataDir + "/vmaps";
    bool result = iVMapManager.loadMap (vmapdir.c_str (), pMapId, -1, -1) == VMAP_LOAD_RESULT_OK;
    if (result)
      {
        MapTree* mt = iVMapManager.getInstanceMapTree (pMapId);
        std::string dirFileName = iVMapManager.getDirFileName (pMapId);
        iTriVarTable = Table<std::string, VAR*>();
        iTriIndexTable = Table<std::string, Array<int> >(); // reset table
        showMap (mt, dirFileName);
      }
    return (result);
  }
  
  //====================================================================

  void
  ModelContainerView::onUserInput (UserInput* ui)
  {
    GApp::onUserInput (ui);
    
    // SKY CODE
    if (ui->keyPressed (GKey::fromString ("y")))
      {
        iShowSky = !iShowSky;
      }

    if (ui->keyPressed (GKey::fromString ("m")))
      {
        iShowMMaps = !iShowMMaps;
      }

    if (ui->keyPressed (GKey::fromString ("v")))
      {
        iShowVMaps = !iShowVMaps;
      }

    if (ui->keyPressed (GKey::fromString ("b")))
      {
        iShowBoxes = !iShowBoxes;
      }

    if (ui->keyPressed (GKey::fromString ("h")))
      { //inc count1
#if 0
        i_App->defaultController.getPosition ();
        Vector3 pos = i_App->defaultController.getPosition ();
        Vector3 pos2 = convertPositionToMangosRep (pos.x, pos.y, pos.z);
        //Vector3 pos3 = iVMapManager.convertPositionToInternalRep(pos2.x, pos2.y, pos2.z);
        //pos3 = iVMapManager.convertPositionToInternalRep(pos2.x, pos2.y, pos2.z);

        float hight = iVMapManager.getHeight (iMap, pos2.x, pos2.y, pos2.z);
        printf ("Hight = %f\n", hight);
#endif
      }
  }
  //==========================================

  void
  ModelContainerView::setViewPosition (const Vector3& pPosition)
  {
    //i_App->defaultController->setPosition(pPosition);
    printf ("Setting position to %g %g %g \n",pPosition.x,pPosition.y,pPosition.z);
    i_App->defaultCamera.setPosition (pPosition);
  }

  //==========================================
  //==========================================
}

