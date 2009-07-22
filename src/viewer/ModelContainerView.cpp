
#include "G3D/Array.h"


#include "G3D/Box.h"


#include "G3D/Array.h"

#include "ModelContainerView.h"
#include "Config/Config.h"


// only for "load *" console command :
#ifdef WIN32
  #include <strsafe.h>

/* std::wstring to std::string function by Andrew Revvo
   ref : http://social.msdn.microsoft.com/Forums/en-US/Vsexpressvc/thread/0f749fd8-8a43-4580-b54b-fbf964d68375/ */
  std::string ws2s(const std::wstring& s)
    {
    int len;
    int slength = (int)s.length() + 1;
    len = WideCharToMultiByte(CP_ACP, 0, s.c_str(), slength, 0, 0, 0, 0); 
    char* buf = new char[len];
    WideCharToMultiByte(CP_ACP, 0, s.c_str(), slength, buf, len, 0, 0); 
    std::string r(buf);
    delete[] buf;
    return r;
    }
#else
  #include <dirent.h>
#endif


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
    iShowSky = false;
    iShowMMaps = true;
    iShowVMaps = true;
    iShowBoxes = false;
    iShowZones = false;
    iShowPortals = false;
    iShowPath = false;
    
    iVARAreaRef = VARArea::create (settings.window.width * settings.window.height * 60 * 40); // thats huge !

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
    keys.sort();
    Array<std::string>::ConstIterator i = keys.begin ();
    while (i != keys.end ())
      {
        VAR* var = iTriVarTable.get (*i);
        Array<int> indexArray = iTriIndexTable.get (*i);
        const std::string& name = *i;
        if (name.find ("1_moveMaps",0) != std::string::npos)
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
        else if (name.find ("5_boundingBoxes",0) != std::string::npos)
          {
            if (iShowBoxes)
              {
              
                rd->setBlendFunc(RenderDevice::BLEND_SRC_ALPHA, RenderDevice::BLEND_ONE_MINUS_SRC_ALPHA);
                rd->setColor (Color4(0, 255, 0, 0.4f)/*Color3::green ()*/);
                rd->beginIndexedPrimitives ();
                rd->setVertexArray (*var);
                rd->sendIndices (RenderDevice::QUADS, indexArray);
                rd->endIndexedPrimitives ();
              }
          }
        else if (name.find ("4_zones",0) != std::string::npos)
          {
            if (iShowZones)
              {
              
                rd->setBlendFunc(RenderDevice::BLEND_SRC_ALPHA, RenderDevice::BLEND_ONE_MINUS_SRC_ALPHA);
                rd->setColor (Color4(255, 255, 0, 0.4f));

                rd->beginIndexedPrimitives ();
                rd->setVertexArray (*var);
                rd->sendIndices (RenderDevice::QUADS, indexArray);
                rd->endIndexedPrimitives ();
              }
          }
        else if (name.find ("4_pzones",0) != std::string::npos)
          {
            if (iShowPath)
              {
              
                rd->setBlendFunc(RenderDevice::BLEND_SRC_ALPHA, RenderDevice::BLEND_ONE_MINUS_SRC_ALPHA);
                rd->setColor (Color4(0, 0, 255, 0.4f));

                rd->beginIndexedPrimitives ();
                rd->setVertexArray (*var);
                rd->sendIndices (RenderDevice::QUADS, indexArray);
                rd->endIndexedPrimitives ();
              }
          }
        else if (name.find ("2_portals",0) != std::string::npos)
          {
            if (iShowPortals)
              {
              
                rd->setColor (Color3(0, 255, 0));

                rd->beginIndexedPrimitives ();
                rd->setVertexArray (*var);
                rd->sendIndices (RenderDevice::LINES, indexArray);
                rd->endIndexedPrimitives ();
              }
          }
        else if (name.find ("3_badportals",0) != std::string::npos)
          {
            if (iShowPortals)
              {
              
                rd->setColor (Color3(255, 0, 0));

                rd->beginIndexedPrimitives ();
                rd->setVertexArray (*var);
                rd->sendIndices (RenderDevice::LINES, indexArray);
                rd->endIndexedPrimitives ();
              }
          }
        else if (name.find ("2_path",0) != std::string::npos)
          {
            if (iShowPath)
              {
              
                rd->setColor (Color3(0, 0, 255));

                rd->beginIndexedPrimitives ();
                rd->setVertexArray (*var);
                rd->sendIndices (RenderDevice::LINES, indexArray);
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
  ModelContainerView::addGrid ( int iMap, int x, int y )
  {
    loadAndShowTile (iMap, x, y);
    
    
    ManagedModelContainer* mc = 0;
    std::string dirFileName = iVMapManager.getDirFileName (iMap, x, y);
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
    
    // show the mmaps
    addMoveMapToDisplay (iMap, x, y);
    
    consolePrintf("Loaded %d-%d",x,y);
  }

  //===================================================

  void
  ModelContainerView::addMoveMapToDisplay ( int mapId, int x, int y )
  {
    // Create a MoveMapContainer that represents the tile
    // Todo later maybe store the container
    MoveMapContainer* moveMapBoxContainer=new MoveMapContainer();
    // Show the bounding boxes of the MoveMapBoxes
    Array<Box> gBoxArray;
    Array<Box> gZoneArray;
    Array<Vector3> gPortalArray;
    Array<Vector3> gBadPortalArray;
    Array<Vector3> gPathArray;
    
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
    moveMapBoxContainer->load (gMMapDataDir.c_str(),buffer);

    for (unsigned int i = 0; i < moveMapBoxContainer->getNMoveMapBoxes (); ++i)
      {
        const MoveMapBox& moveMapBox = moveMapBoxContainer->getMoveMapBox (i);
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
    
    char name[30];
    sprintf(name,"1_moveMaps2_%d-%d",x,y);
    printf("Load %d MoveMaps Points vertices to %s\n",vArray.size(),name);

    iTriVarTable.set (name, new VAR (vArray, iVARAreaRef));
    iTriIndexTable.set (name, iArray);
    
    // Add the Bounding boxes
    Array<int> ibArray;
    Array<Vector3> vbArray;
    sprintf(name,"5_boundingBoxes_%d-%d",x,y);
    
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
    char maprefbuffer[6];
    sprintf (maprefbuffer, "%02u:%02u", x, y);
    std::string mapref=maprefbuffer;
    printf ("loading map %s\n",mapref.c_str());
    iMoveZoneContainers.set(mapref,moveMapBoxContainer->getMoveZoneContainer());
    Array<MoveZone*> iMoveZoneArray=iMoveZoneContainers[mapref]->getMoveZoneArray();
    for (unsigned int j = 0; j< iMoveZoneArray.size(); j++)
      {
      const MoveZone* iMoveZone=iMoveZoneArray[j];
      const AABox& b = iMoveZone->getBounds();
      AABox ab ;
      ab.set(Vector3(b.low().x, b.low().z, b.low().y), Vector3(b.high().x, b.high().z, b.high().y));
      
      gZoneArray.push_back(Box(ab));
      const Array<MovePortal*> portals = iMoveZone->getPortalArray();
      for (unsigned int p = 0; p< portals.size(); ++p)
        {
        Vector3 mylow=Vector3(portals[p]->getLow2(),iMoveZone->getBounds().high().z);
        Vector3 myhigh=Vector3(portals[p]->getHigh2(),iMoveZone->getBounds().high().z);
        if (portals[p]->getDirection() == EXTEND_N)
            {
            mylow.x-=0.3f;
            myhigh.x+=0.3f;
            mylow.y+=0.1f;
            myhigh.y+=0.1f;
            }
        else if (portals[p]->getDirection() == EXTEND_S)
            {
            mylow.x-=0.3f;
            myhigh.x+=0.3f;
            mylow.y-=0.1f;
            myhigh.y-=0.1f;
            }
        else if (portals[p]->getDirection() == EXTEND_E)
            {
            mylow.y-=0.3f;
            myhigh.y+=0.3f;
            mylow.x+=0.1f;
            myhigh.x+=0.1f;
            }
        else if (portals[p]->getDirection() == EXTEND_W)
            {
            mylow.y-=0.3f;
            myhigh.y+=0.3f;
            mylow.x-=0.1f;
            myhigh.x-=0.1f;
            }
        if (portals[p]->getDestinationID() == INT_MAX)
            {
            gBadPortalArray.push_back(Vector3(mylow.x,mylow.z,mylow.y));
            gBadPortalArray.push_back(Vector3(myhigh.x,myhigh.z,myhigh.y));
            }
        else
            {
            gPortalArray.push_back(Vector3(mylow.x,mylow.z,mylow.y));
            gPortalArray.push_back(Vector3(myhigh.x,myhigh.z,myhigh.y));
            }
        }
      }
    printf("added %i zones \n",gZoneArray.size());
    
    // Add the Zones
    Array<int> izArray;
    Array<Vector3> vzArray;

    sprintf(name,"4_zones_%d-%d",x,y);

    count = 0;
    
    for(Array<Box>::Iterator iter = gZoneArray.begin (); iter != gZoneArray.end (); ++iter)
      {
        const Box& b = *iter;
        for (int i = 0; i < 8; i++)
          vzArray.append (b.corner (i));

        // side 1
        izArray.append (count);
        izArray.append (count + 1);
        izArray.append (count + 2);
        izArray.append (count + 3);

        // side 1 back
        izArray.append (count + 3);
        izArray.append (count + 2);
        izArray.append (count + 1);
        izArray.append (count);
        
        // side 2
        izArray.append (count + 4);
        izArray.append (count + 5);
        izArray.append (count + 6);
        izArray.append (count + 7);
        
        // side 2 back
        izArray.append (count + 7);
        izArray.append (count + 6);
        izArray.append (count + 5);
        izArray.append (count + 4);
        
        // side 3
        izArray.append (count + 1);
        izArray.append (count + 2);
        izArray.append (count + 6);
        izArray.append (count + 5);
        
        // side 3 back
        izArray.append (count + 5);
        izArray.append (count + 6);
        izArray.append (count + 2);
        izArray.append (count + 1);
        
        // side 4
        izArray.append (count + 0);
        izArray.append (count + 3);
        izArray.append (count + 7);
        izArray.append (count + 4);
        
        // side 4 back
        izArray.append (count + 4);
        izArray.append (count + 7);
        izArray.append (count + 3);
        izArray.append (count + 0);
        
        // side 5
        izArray.append (count + 2);
        izArray.append (count + 3);
        izArray.append (count + 7);
        izArray.append (count + 6);
        
        // side 5 back
        izArray.append (count + 6);
        izArray.append (count + 7);
        izArray.append (count + 3);
        izArray.append (count + 2);
        
        // side 6
        izArray.append (count + 0);
        izArray.append (count + 1);
        izArray.append (count + 5);
        izArray.append (count + 4);
        
        // side 6 back
        izArray.append (count + 4);
        izArray.append (count + 5);
        izArray.append (count + 1);
        izArray.append (count + 0);

        count += 8;
      }
    
    printf("Load %d MapZones vertices\n",vzArray.size());
    iTriVarTable.set (name, new VAR (vzArray, iVARAreaRef));
    iTriIndexTable.set (name, izArray);
    
    // Add the Portals
    Array<int> ipArray;
    Array<Vector3> vpArray;
    
    sprintf(name,"2_portals_%d-%d",x,y);

    count = 0;
    
    for(Array<Vector3>::Iterator iter = gPortalArray.begin (); iter != gPortalArray.end (); ++iter)
      {
        const Vector3& v = *iter;
        vpArray.append (v);
        ipArray.append (count++);
      }
    printf("Load %d portals vertices\n",vpArray.size());
    iTriVarTable.set (name, new VAR (vpArray, iVARAreaRef));
    iTriIndexTable.set (name, ipArray);

    // Add empty path array
    Array<int> ipthArray;
    Array<Vector3> vpthArray;

    
    sprintf(name,"2_path_%d-%d",x,y);

    count = 0;
    
    char coordname[255];
    sprintf (coordname, "%s/path_%03u_%02u_%02u.txt",startCoordsPath.c_str(), mapId, x, y);
    FILE* f = fopen (coordname, "r");
    if (f)
      {
      int bufferSize = 500;
      char buffer[500];
      float px, py, pz;
      while (fgets (buffer, bufferSize - 1, f))
        {
        sscanf (buffer, "%f,%f,%f", &px, &py, &pz);
        if(gPathArray.size() > 0)
          gPathArray.push_back (Vector3 (px, pz, py));
        // for the next line
        gPathArray.push_back (Vector3 (px, pz, py));
        }
      fclose (f);
      gPathArray.popDiscard();
      }
    
    for(Array<Vector3>::Iterator iter = gPathArray.begin (); iter != gPathArray.end (); ++iter)
      {
        const Vector3& v = *iter;
        vpthArray.append (v);
        ipthArray.append (count++);
      }
    printf("Load %d path vertices\n",vpthArray.size());
    iTriVarTable.set (name, new VAR (vpthArray, iVARAreaRef));
    iTriIndexTable.set (name, ipthArray);
    
    // Add Portals with dest not found
    Array<int> ibpArray;
    Array<Vector3> vbpArray;


    sprintf(name,"3_badportals_%d-%d",x,y);

    count = 0;
    
    for(Array<Vector3>::Iterator iter = gBadPortalArray.begin (); iter != gBadPortalArray.end (); ++iter)
      {
        const Vector3& v = *iter;
        vbpArray.append (v);
        ibpArray.append (count++);
      }
    printf("Load %d badportals vertices\n",vbpArray.size());
    iTriVarTable.set (name, new VAR (vbpArray, iVARAreaRef));
    iTriIndexTable.set (name, ibpArray);
    
    
    printf("total used : %dB , %dB (%f%%) free\n",iVARAreaRef->allocatedSize(),iVARAreaRef->freeSize(), 100.0f*iVARAreaRef->freeSize()/iVARAreaRef->totalSize());
    if (100.0f*iVARAreaRef->freeSize()/iVARAreaRef->totalSize() < 10)
      printf("#### WARNING ####\nless than 10%% memory remaining, you mays crash if you load more grids, see ModelContainerView::ModelContainerView to ajust available VARArea memory\n############\n");
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

    if (ui->keyPressed (GKey::fromString ("z")))
      {
        iShowZones = !iShowZones;
      }

    if (ui->keyPressed (GKey::fromString ("p")))
      {
        iShowPortals = !iShowPortals;
      }
    
    if (ui->keyPressed (GKey::fromString ("i")))
      {
        iShowPath = !iShowPath;
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

  //===================================================

  void
  ModelContainerView::onConsoleCommand(const std::string& cmd)
  {
    Array<std::string> command = stringSplit(cmd,' ');
    
    if (command[0] == "exit")
      {
      setExitCode(0);
      return;
      }
    if (command[0] == "add")
      {
      unsigned int argnb=1;
      if (command.size()<argnb+1 || command[argnb] != "path")
        {
        consolePrintf("syntax error");
        return;
        }
        
      std::string mapref=command[++argnb];
      const MoveZoneContainer* iMZcontainer;
      if (!iMoveZoneContainers.get(mapref,iMZcontainer))
        {
        consolePrintf("map '%s' is not loaded",mapref.c_str());
        return;
        }
      
      if (command.size()<argnb+1)
        {
        consolePrintf("syntax error");
        return;
        }
      
      Vector3 orig,dest;
      
      std::string iszone=command[argnb+1];
      if(iszone == "zones")
        {
        // adding path by zone id
        argnb++;
        if (command.size()<argnb + 2)
          {
          consolePrintf("syntax error");
          return;
          }
        
        unsigned int origzoneid = atoi(command[++argnb].c_str());
        unsigned int destzoneid = atoi(command[++argnb].c_str());
        
        Array<MoveZone*> iMoveZoneArray=iMZcontainer->getMoveZoneArray();
        
        if (origzoneid<0 && origzoneid>=iMoveZoneArray.size())
          {
          consolePrintf("error : non-existent zone id %u",origzoneid);
          return;
          }
        else if (destzoneid<0 && destzoneid>=iMoveZoneArray.size())
          {
          consolePrintf("error : non-existent zone id %u",origzoneid);
          return;
          }
        else
          {
          const MoveZone* iMoveZone=iMoveZoneArray[origzoneid];
          const AABox& ob = iMoveZone->getBounds();
          orig = Vector3((ob.low().x+ob.high().x)/2, (ob.low().y+ob.high().y)/2, (ob.low().z+ob.high().z)/2);

          iMoveZone=iMoveZoneArray[destzoneid];
          const AABox& db = iMoveZone->getBounds();
          dest = Vector3((db.low().x+db.high().x)/2, (db.low().y+db.high().y)/2, (db.low().z+db.high().z)/2);
          }
        }
      else
        {
        // adding path by coords
        if (command.size()<argnb + 6)
          {
          consolePrintf("syntax error");
          return;
          }
        
        orig = Vector3(atof(command[argnb+1].c_str()), atof(command[argnb+2].c_str()), atof(command[argnb+3].c_str()));
        dest = Vector3(atof(command[argnb+4].c_str()), atof(command[argnb+5].c_str()), atof(command[argnb+6].c_str()));
        }
      printf("looking for path %f,%f,%f -> %f,%f,%f\n",orig.x,orig.y,orig.z,dest.x,dest.y,dest.z);
      
      PathGenerator* pathGen = new PathGenerator(orig,dest,iMZcontainer);
      pathGen->withStreching();
      unsigned int result = pathGen->GeneratePath();
      
      if (result == ERR_ORIG_NOT_FOUND)
        {
        consolePrintf("orig is not inside a zone");
        return;
        }
      else if (result == ERR_DEST_NOT_FOUND)
        {
        consolePrintf("dest is not inside a zone");
        return;
        }
      else if (result == PATH_FOUND)
        {
        char name[14];
        Array<std::string> gridCoords = stringSplit(mapref,':');
        if (gridCoords.size()<0)
          {
          consolePrintf("syntax error");
          return;
          }
        sprintf(name,"2_path_%s-%s",gridCoords[0].c_str(),gridCoords[1].c_str());
        // Add empty path array
        Array<Vector3> gPathArray;
        Array<Vector3> iPath = pathGen->getPathArray();
        
        for (unsigned int pathIdx=0; pathIdx < iPath.size(); ++pathIdx)
          {
          if(gPathArray.size() > 0)
            gPathArray.push_back (Vector3(iPath[pathIdx].x,iPath[pathIdx].z,iPath[pathIdx].y));
          // for the next line
          gPathArray.push_back (Vector3(iPath[pathIdx].x,iPath[pathIdx].z,iPath[pathIdx].y));
          }
        gPathArray.popDiscard();
        
        Array<int> ipthArray;
        Array<Vector3> vpthArray;
        
        unsigned int count = 0;
        for(Array<Vector3>::Iterator iter = gPathArray.begin (); iter != gPathArray.end (); ++iter)
          {
          const Vector3& v = *iter;
          vpthArray.append (v);
          ipthArray.append (count++);
          }
        printf("Loaded %d path vertices\n",vpthArray.size());
        iTriVarTable.set (name, new VAR (vpthArray, iVARAreaRef));
        iTriIndexTable.set (name, ipthArray);
        }
      else if (result == PATH_NOT_FOUND)
        {
        consolePrintf("path not found ; adding visited zones instead");
        Array<unsigned int> visitedMZ = pathGen->getVisitedCells();
        
        printf("%d visited cells, first:%u\n",visitedMZ.size(),visitedMZ[1]);
        

        // Add the Zones
        Array<int> izArray;
        Array<Vector3> vzArray;
        char name[20];
        Array<std::string> gridCoords = stringSplit(command[2],':');
        if (gridCoords.size()<0)
          {
          consolePrintf("syntax error");
          return;
          }
        sprintf(name,"4_pzones_%s-%s",gridCoords[0].c_str(),gridCoords[1].c_str());
        
        unsigned int count = 0;
        AABox b;

        for(unsigned int mzidx=0;mzidx<visitedMZ.size();++mzidx)
          {
          const MoveZone* iMoveZone=iMZcontainer->getZone(visitedMZ[mzidx]);
          const AABox ob = iMoveZone->getBounds();
          b.set(Vector3(ob.low().x, ob.low().z, ob.low().y), Vector3(ob.high().x, ob.high().z, ob.high().y));
          
          for (int i = 0; i < 8; i++)
            vzArray.append (Box(b).corner (i));

          // side 1
          izArray.append (count);
          izArray.append (count + 1);
          izArray.append (count + 2);
          izArray.append (count + 3);

          // side 1 back
          izArray.append (count + 3);
          izArray.append (count + 2);
          izArray.append (count + 1);
          izArray.append (count);
        
          // side 2
          izArray.append (count + 4);
          izArray.append (count + 5);
          izArray.append (count + 6);
          izArray.append (count + 7);
        
          // side 2 back
          izArray.append (count + 7);
          izArray.append (count + 6);
          izArray.append (count + 5);
          izArray.append (count + 4);
        
          // side 3
          izArray.append (count + 1);
          izArray.append (count + 2);
          izArray.append (count + 6);
          izArray.append (count + 5);
        
          // side 3 back
          izArray.append (count + 5);
          izArray.append (count + 6);
          izArray.append (count + 2);
          izArray.append (count + 1);
        
          // side 4
          izArray.append (count + 0);
          izArray.append (count + 3);
          izArray.append (count + 7);
          izArray.append (count + 4);
        
          // side 4 back
          izArray.append (count + 4);
          izArray.append (count + 7);
          izArray.append (count + 3);
          izArray.append (count + 0);
        
          // side 5
          izArray.append (count + 2);
          izArray.append (count + 3);
          izArray.append (count + 7);
          izArray.append (count + 6);
        
          // side 5 back
          izArray.append (count + 6);
          izArray.append (count + 7);
          izArray.append (count + 3);
          izArray.append (count + 2);
        
          // side 6
          izArray.append (count + 0);
          izArray.append (count + 1);
          izArray.append (count + 5);
          izArray.append (count + 4);
        
          // side 6 back
          izArray.append (count + 4);
          izArray.append (count + 5);
          izArray.append (count + 1);
          izArray.append (count + 0);

          count += 8;
        
          }
        
        printf("added %d visited cell vertices\n",vzArray.size());

        iTriVarTable.set (name, new VAR (vzArray, iVARAreaRef));
        iTriIndexTable.set (name, izArray);
        
        return;
        }

      return;
      }
    
    if (command[0] == "load")
      {
      if (command.size()==2 && command[1]=="*")
        {
        unsigned int n=0;
        #ifdef WIN32
          WIN32_FIND_DATA ffd;
          TCHAR szDir[MAX_PATH];

          size_t origsize = strlen(gMMapDataDir.c_str()) + 1;
          size_t convertedChars = 0;
          wchar_t wcstring[MAX_PATH];
          mbstowcs_s(&convertedChars, wcstring, origsize, gMMapDataDir.c_str(), _TRUNCATE);
          
          StringCchCopy(szDir, MAX_PATH, wcstring);
          StringCchCat(szDir, MAX_PATH, TEXT("\\*.mmap"));
          HANDLE hFind = FindFirstFile(szDir, &ffd);
          do
            {
            std::string fname = ws2s(ffd.cFileName);
        #else
          DIR* dirp=opendir(gMMapDataDir.c_str());
          dirent* de;
          while ((de = readdir(dirp)) != NULL)
            {
            std::string fname = de->d_name;
            if (fname.find(".mmap") != std::string::npos)
              {
        #endif
          
            int x = atoi(fname.substr(4,2).c_str());
            int y = atoi(fname.substr(7,2).c_str());
            addGrid(iMap, x, y);
            n++;
            // hardcoded width and height ...
            if (100.0f*iVARAreaRef->freeSize()/iVARAreaRef->totalSize() < 8) //1024 * 768 * 60 > iVARAreaRef->freeSize()) //
              {
              consolePrintf("Had to stop loading due to memory after adding %u grids (defined in ModelContainerView::ModelContainerView)",n);
              return;
              }
            
        #ifdef WIN32
          }while (FindNextFile(hFind, &ffd) != 0);
          FindClose(hFind);
        #else
            }
          }
          closedir(dirp);
        #endif
        consolePrintf("loaded %u grids",n);
        return;
        }
      if (command.size()<3)
        {
        consolePrintf("syntax error");
        return;
        }
      int x=atoi(command[1].c_str());
      int y=atoi(command[2].c_str());
      
      char buffer[255];
      sprintf (buffer, "%s/%03u_%02u_%02u.mmap", gMMapDataDir.c_str(), iMap, x, y);
      FILE* fp = fopen(buffer,"rb");
      if (!fp)
       {
       consolePrintf("can't open mmap file %s (non-existent?)",buffer);
       return;
       }
      fclose(fp);

      addGrid(iMap, x, y);

      return;
      }
    if (command[0] == "go")
      {
      if (command.size()<2)
        {
        consolePrintf("syntax error");
        return;
        }
      if (command[1] == "zone")
        {
        if (command.size() != 4)
          {
          consolePrintf("syntax error");
          return;
          }
        std::string mapref=command[2];
        unsigned int zoneid=atoi(command[3].c_str());
        printf("go zone %s %u\n",mapref.c_str(),zoneid);
        const MoveZoneContainer* iMZcontainer;
        if (!iMoveZoneContainers.get(mapref,iMZcontainer))
          {
          consolePrintf("map %s is not loaded",mapref.c_str());
          return;
          }
        Array<MoveZone*> iMoveZoneArray=iMZcontainer->getMoveZoneArray();
        
        if (zoneid>=0 && zoneid<iMoveZoneArray.size())
          {
          const MoveZone* iMoveZone=iMoveZoneArray[zoneid];
          const AABox& b = iMoveZone->getBounds();
          defaultController->setPosition(Vector3((b.low().x+b.high().x)/2, b.high().z + 20, (b.low().y+b.high().y)/2));
          defaultController->lookAt(Vector3((b.low().x+b.high().x)/2, b.high().z, (b.low().y+b.high().y)/2));
          consolePrintf("changed view to zone id %u",iMoveZone->getIndex());
          consolePrintf("box: low %f,%f high %f,%f",b.low().x,b.low().y,b.high().x,b.high().y);
          }
        else
          {
          consolePrintf("zone %u does not exist",zoneid);
          return;
          }        
        }
      else if (command[1] == "xyz")
        {
        if (command.size() != 5)
          {
          consolePrintf("syntax error");
          return;
          }
        float x=atoi(command[2].c_str());
        float y=atoi(command[3].c_str());
        float z=atoi(command[4].c_str());
        defaultController->setPosition(Vector3(x,z+5,y));
        defaultController->lookAt(Vector3(x,z,y));
        consolePrintf("changed view to coords [%f,%f,%f]",x,y,z);
        return;
        }
      else
        consolePrintf("syntax error");
      return;
      }
    
    consolePrintf("unknown command");
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

