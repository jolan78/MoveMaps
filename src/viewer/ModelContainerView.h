#ifndef _MODELCONTAINERVIEW_H
#define _MODELCONTAINERVIEW_H

#include "VMapManager.h"
#include <G3D/G3DAll.h>
#include "ModelContainer.h"
#include "MoveMapBoxContainer.h"
#include "MoveZone.h"

namespace VMAP
{
  //==========================================
  class ModelContainerView :
  public G3D::GApp
  {
  private:
    //SKY CODE
    SkyRef iSky;
    SkyParameters iSkyParameters;
    
    bool iShowSky;
    bool iShowMMaps;
    bool iShowVMaps;
    bool iShowBoxes;
    bool iShowZones;
    bool iShowPortals;
    
    VARAreaRef iVARAreaRef;
    
    Table<std::string, VAR*> iTriVarTable;
    Table<std::string, Array<int> > iTriIndexTable;
    
    GApp* i_App;

    VMapManager iVMapManager;
    
    Table<std::string, const MoveZoneContainer*> iMoveZoneContainers;
    
    int iMap;
    int ix;
    int iy;
  private:
    void ModelContainerView::addGrid ( int iMap, int x, int y );
    void addMoveMapToDisplay ( int mapId, int x, int y );
    
    void fillVertexAndIndexArraysWithMoveMapBoxContainer (Array<Vector3>& vArray, Array<int>& iArray, MoveMapContainer* iMoveMapBoxContainer);

  public:
    //ModelContainerView(GApp* pApp, int mapId, int x, int y);
    ModelContainerView (const G3D::GApp::Settings& settings, int mapId, int x, int y);

    ~ModelContainerView (void);

    void addModelContainer (const std::string& pName, const ModelContainer* pModelContainer);
    void removeModelContainer (const std::string& pName, const ModelContainer* pModelContainer);
    void setViewPosition (const Vector3& pPosition);

    void onGraphics (RenderDevice* rd, Array<PosedModelRef> &posed3D, Array<PosedModel2DRef> &posed2D);
    virtual void onInit ();
    void init ();
    void cleanup ();
    void onUserInput (UserInput* ui);
    void onConsoleCommand(const std::string& cmd);

    void fillRenderArray (const SubModel& pSm, Array<TriangleBox> &pArray, const TreeNode* pTreeNode);
    void fillVertexAndIndexArrays (const SubModel& pSm, Array<Vector3>& vArray, Array<int>& iArray );

    bool loadAndShowTile (int pMapId, int x, int y);
    void showMap (int pMapId, int x, int y);

    void showMap (MapTree* mt, std::string dirFileName);
    bool loadAndShowTile (char *pName, int pMapId);
  };

  //==========================================
}

#endif
