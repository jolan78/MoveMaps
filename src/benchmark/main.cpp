#include "VMapManager.h"
#include <G3D/G3DAll.h>


#include "PathGenerator.h"
#include "MoveMapBoxContainer.h"


#include "Config/Config.h"

namespace VMAP
{
  std::string startCoordsPath;
  std::string gDataDir;
  //std::string gvMapDataDir;
  std::string gMMapDataDir;
  std::string g3dDataDir;
}

G3D_START_AT_MAIN ();

int
main (int argc, char** argv)
{
  if (argc < 3)
    {

      printf ("Need 8 arguments: MapId, Map X coord, Map y coord, start zoneID, dest zoneID");
      return 1;
    }

  Config conf;
  conf.SetSource ("mmap.conf");
  
  VMAP::gMMapDataDir = conf.GetStringDefault("mmap.outputdir",".");  
  
  VMAP::MoveMapContainer moveMapBoxContainer;
  char buffer[50];
  
  sprintf (buffer, "%03u_%02u_%02u", atoi (argv[1]), atoi (argv[2]), atoi (argv[3]));
  moveMapBoxContainer.load (VMAP::gMMapDataDir.c_str(),buffer);
  
  VMAP::MoveZoneContainer* MZcontainer =  moveMapBoxContainer.getMoveZoneContainer();
  VMAP::MoveZone* startMZ = MZcontainer->getZone(atoi(argv[4]));
  VMAP::MoveZone* destMZ = MZcontainer->getZone(atoi(argv[5]));

  Vector3 orig=startMZ->getBounds().center();
  Vector3 dest=destMZ->getBounds().center();
  VMAP::PathGenerator* pathGen = new VMAP::PathGenerator(orig,dest,MZcontainer);
  
  unsigned int result = pathGen->GeneratePath();
  
  printf("Path %u:%f,%f,%f -> %u:%f,%f,%f\n",atoi(argv[4]),orig.x,orig.y,orig.z,atoi(argv[5]),dest.x,dest.y,dest.z);
  printf ("PathGenerator returned %u\n",result);
  pathGen->PrintPath();
  
  
  return 0;
}


#include "Config/Config.cpp"

