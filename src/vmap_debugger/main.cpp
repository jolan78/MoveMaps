
#include <stdio.h>

#include "ViewApp.h"
namespace VMAP2
{
  extern std::string VMAPDirPrefix;
}

int main(int argc,const char* argv[])
{
  VMAP2::VMAPDirPrefix = argv[1];
  VMAP2::VMAPDirPrefix += "/";
  
  G3D::GApp::Settings settings;
  
  //settings.window.
  settings.window.height = 768;
  settings.window.width = 1024;
          
  VMAP2::ViewApp app(settings);
  
  app.loadVGrid (argv[2]);
  
//  return app.run ();
}


