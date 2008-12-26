
#include <string>
#include <map>

#include <GLG3D/GApp.h>
#include <GLG3D/VARArea.h>
#include <G3D/Array.h>

namespace VMAP2
{
  class ViewApp : public G3D::GApp
  {
  private:
    typedef std::pair<G3D::Array<int>, G3D::VAR > IndexVarPair;
    typedef std::map<std::string, IndexVarPair> IndexVarMap;

    IndexVarMap iMap;
    G3D::VARAreaRef iVARAreaRef;
    
    void onGraphics (G3D::RenderDevice* rd, G3D::Array<G3D::PosedModelRef> &posed3D, G3D::Array<G3D::PosedModel2DRef> &posed2D);
  public:
    ViewApp (const G3D::GApp::Settings& settings);
    ~ViewApp ();
    
    void loadVGrid (const char* file);
  };
}

