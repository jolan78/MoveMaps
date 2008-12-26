
#include "G3D/Array.h"


#include "G3D/Array.h"


#include <G3D/G3DAll.h>

#include "ViewApp.h"
#include "VGrid.h"

namespace VMAP2
{
  extern std::string VMAPDirPrefix;
  
  ViewApp::ViewApp (const G3D::GApp::Settings& settings) :
  GApp (settings),
  iVARAreaRef (VARArea::create (1024 * 1024 * 60))
 { }

  ViewApp::~ViewApp () 
  {
    // 
  }
          
  void ViewApp::loadVGrid (const char* file)
  {
    VGrid grid(file);
    
    // note that this doesnt load the bounds od the grid
    // because they are stored in another file
    grid.load();
    
    Array<Vector3> vectorArray;
    Array<int> indexArray;
    Vector3 tempCameraPos;
    int count = 0;

    ModelTree::Iterator iter = grid.getTree ().begin ();
    ModelTree::Iterator end = grid.getTree ().end ();

    while (iter != end)
      {
        Model& m = *(iter->value);

        Model::TriangleTree::Iterator miter = m.getTree ().begin ();
        Model::TriangleTree::Iterator mend = m.getTree ().end ();

        while (miter != mend)
          {
            vectorArray.append (miter->vertex (0));
            vectorArray.append (miter->vertex (1));
            vectorArray.append (miter->vertex (2));

            tempCameraPos = miter->vertex (2);
            
            indexArray.append (count + 0);
            indexArray.append (count + 1);

            indexArray.append (count + 1);
            indexArray.append (count + 2);

            indexArray.append (count + 2);
            indexArray.append (count + 0);

            count += 3;

            ++miter;
          }

        ++iter;
      }
    
    // Set camera position in some random place in the scene
    defaultCamera.setPosition (tempCameraPos);

    VAR v (vectorArray, iVARAreaRef);
    iMap[std::string (file)] = IndexVarPair (indexArray, v);
  }
    
  void
  ViewApp::onGraphics (RenderDevice* rd, Array<PosedModelRef> &posed3D, Array<PosedModel2DRef> &posed2D)
  {
    //das das
    rd->setProjectionAndCameraMatrix (defaultCamera);

    rd->setAmbientLightColor (Color4 (Color3::white ()));

    rd->setColorClearValue (Color3::black ());
    rd->clear ();

    rd->enableLighting ();

    GLight light = GLight::directional (defaultController.pointer ()->position () + defaultController.pointer ()->lookVector ()*2, Color3::white ());
    rd->setLight (0, light);

    /// dasdas da
    for(IndexVarMap::iterator iter = iMap.begin (); iter != iMap.end (); ++iter)
      {
        G3D::Array<int>& index = iter->second.first;
        VAR& var = iter->second.second;

        rd->setColor (Color3::red ());
        rd->beginIndexedPrimitives ();
        rd->setVertexArray (var);
        rd->sendIndices (RenderDevice::LINES, index);
        rd->endIndexedPrimitives ();
      }

    rd->disableLighting ();

    PosedModel2D::sortAndRender (rd, posed2D);
  }
  
  
}










