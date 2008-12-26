#include <time.h>
#include "ModelContainer.h"
#include "VMapManager.h"
#include "MoveMapGenerator.h"
#include "PositionControlArray.h"

#include "TileAssembler.h"

#include "Config/Config.h"

namespace VMAP 
{
  std::string startCoordsPath;
}

int
GenerateMMAPS (int mapId, int x, int y, bool GenCoords)
{
  time_t sec1;
  time (&sec1);
  
  std::string gDataDir;
  std::string gMMapsDir;
  Config conf;
  conf.SetSource("mmap.conf");
  
  gDataDir = conf.GetStringDefault("mmap.datadir",".");
  gMMapsDir = conf.GetStringDefault("mmap.outputdir",".");
  VMAP::startCoordsPath = conf.GetStringDefault("mmap.coordsdir",".");
  
  VMAP::MoveMapGenerator iMoveMapGenerator(gDataDir,gMMapsDir);
  if (!iMoveMapGenerator.generateMoveMaps (mapId, x, y, GenCoords))
    return 1;
  
  time_t sec2;
  time (&sec2);
  long diff = sec2 - sec1;
  printf ("Time = %d\n", diff);
  
  return 0;
}

int
main (int argc, char** argv)
{
  if (argc < 3)
    printf ("Need 3 arguments: MapId, Map X coord, Map y coord, [generate coords (0 or 1)]");
  else
    {
      bool GenCoords;
      if (argc > 4 && atoi (argv[4]) > 0)
        GenCoords = true;
      else
        GenCoords = false;

      return GenerateMMAPS (atoi (argv[1]), atoi (argv[2]), atoi (argv[3]), GenCoords);
    }
  return 0;
}

#include "Config/Config.cpp"


