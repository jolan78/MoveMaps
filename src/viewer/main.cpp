
#include "ModelContainerView.h"

#include "Config/Config.h"

namespace VMAP
{
  std::string startCoordsPath;
  std::string gDataDir;
  std::string gvMapDataDir;
  std::string gMMapDataDir;
  std::string g3dDataDir;
}

G3D_START_AT_MAIN ();

int
main (int argc, char** argv)
{
  if (argc < 3)
    {

      printf ("Need 3 arguments: MapId, Map X coord, Map y coord");
      return 1;
    }

  Config conf;
  conf.SetSource ("mmap.conf");
  
  VMAP::gDataDir = conf.GetStringDefault("mmap.datadir",".");
  VMAP::gMMapDataDir = conf.GetStringDefault("mmap.outputdir",".");
  VMAP::startCoordsPath = conf.GetStringDefault("mmap.coordsdir",".");
  VMAP::g3dDataDir = conf.GetStringDefault("mmap.g3ddatadir",".");

  G3D::GApp::Settings settings;
  settings.window.width = conf.GetIntDefault("mmap.winwidth",1024);
  settings.window.height = conf.GetIntDefault("mmap.winheight",768);
  
  VMAP::gvMapDataDir = VMAP::gDataDir + "/vmaps";
  

  VMAP::ModelContainerView modelContainerView (settings, atoi (argv[1]), atoi (argv[2]), atoi (argv[3]));
  modelContainerView.run ();

  return 0;
}


#include "Config/Config.cpp"


