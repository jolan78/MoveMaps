#include "VMapManager.h"
#include <G3D/G3DAll.h>


#include "PathGenerator.h"
#include "MoveMapBoxContainer.h"

//debug
#include <iostream>

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
  time_t begin;
  time_t end;
  
  Config conf;
  conf.SetSource ("mmap.conf");
  
  VMAP::gMMapDataDir = conf.GetStringDefault("mmap.outputdir",".");  
  
  VMAP::MoveMapContainer moveMapBoxContainer;
  char buffer[50];
  
  sprintf (buffer, "%03u_%02u_%02u", atoi (argv[1]), atoi (argv[2]), atoi (argv[3]));
  moveMapBoxContainer.load (VMAP::gMMapDataDir.c_str(),buffer);
  
  VMAP::MoveZoneContainer* MZcontainer =  moveMapBoxContainer.getMoveZoneContainer();
  printf("preparing\n");
  
  Array<Vector3> benchPoints;
  for (unsigned int zone=0;zone<MZcontainer->getNZones();zone++)
    benchPoints.push_back(MZcontainer->getZone(zone)->getBounds().center());
  /*VMAP::MoveZone* startMZ = MZcontainer->getZone(atoi(argv[4]));
  VMAP::MoveZone* destMZ = MZcontainer->getZone(atoi(argv[5]));

  Vector3 orig=startMZ->getBounds().center();
  Vector3 dest=destMZ->getBounds().center();*/
  printf("benching\n");
  
  time(&begin);
  unsigned int n=0;
  unsigned int maxPath=30000;
  unsigned int failedPaths=0;
  for (int dest=benchPoints.size()-1;dest>0;--dest)
    {
    int orig=benchPoints.size()-dest;
    //for (int orig=0;orig<benchPoints.size();++orig)
      {
      VMAP::PathGenerator* pathGen = new VMAP::PathGenerator(benchPoints[orig],benchPoints[dest],MZcontainer);
      unsigned int result = pathGen->GeneratePath();
      n++;
      delete pathGen;
      if (result != PATH_FOUND)
        {
        printf("path failed(%u) (%f,%f,%f) -> %f,%f,%f\n",result,benchPoints[orig].x,benchPoints[orig].y,benchPoints[orig].z,benchPoints[dest].x,benchPoints[dest].y,benchPoints[dest].z);
        VMAP::PathGenerator* pathGen = new VMAP::PathGenerator(benchPoints[dest],benchPoints[orig],MZcontainer);
        result = pathGen->GeneratePath();
        if (result == PATH_FOUND)
          {
          printf("backward path works !\n");
          //pathGen->PrintPath();
          //return 0;
          }
        n++;
        delete pathGen;
        
        failedPaths++;
        }
      if(!(n%500))
        {
        time(&end);

        printf("##%u paths (%u failed) in %u secs. : %f path/s\n",n,failedPaths,(end-begin),(float)n / (float)(end-begin));
        if (n>maxPath)
          return 0;
        }
      }
    }
  time(&end);

  printf("done : %u paths (%u failed) in %u secs. : %f path/s\n",n,failedPaths,(end-begin),(float)n / (float)(end-begin));
  
  time(&begin);
  printf("benchmark of R*Tree lookups :\n");
  unsigned int f=0;
  for (int i=0;i<3000000;++i)
    {
    int n=(i%(benchPoints.size()-1));
    VMAP::MoveZone* startMZ = MZcontainer->getMoveZoneByCoords(benchPoints[n]);
    if (startMZ == NULL)
      {
      printf("lookup failed, point %u : %f,%f,%f\n",n, benchPoints[n].x,benchPoints[n].y,benchPoints[n].z);
      f++;
      }
    }
  time(&end);
  printf("done : 3,000,000 lookups in %u secs. %u failed: %f lookup/s (2 lookups per path needed)\n",(end-begin),f,3000000 / (float)(end-begin));
  
  printf("path exemple :\n");
  VMAP::PathGenerator* pathGen = new VMAP::PathGenerator(benchPoints[0],benchPoints[benchPoints.size()-1],MZcontainer);

  unsigned int result = pathGen->GeneratePath();
  
  printf("Path %f,%f,%f -> %f,%f,%f\n",benchPoints[0].x,benchPoints[0].y,benchPoints[0].z,benchPoints[benchPoints.size()-1].x,benchPoints[benchPoints.size()-1].y,benchPoints[benchPoints.size()-1].z);
  printf ("PathGenerator returned %u\n",result);
  pathGen->PrintPath();
  
  std::string cmd;
  printf("pause\n");
  std::cin >>  cmd;

  
  return 0;
}


#include "Config/Config.cpp"

