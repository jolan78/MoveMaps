

#include <stdlib.h>
#include <stdio.h>
#include <string>

#include "MapAssembler.h"

G3D_START_AT_MAIN ();  

int main (int argc, char* argv[])
{
  using namespace VMAP2;
  
  if (argc == 3 || argc == 4)
    {
      char *src = argv[1];
      char *dest = argv[2];

      MapAssembler* ma = new MapAssembler (std::string (src), std::string (dest));

      ma->addWorldAreaMapId(0);   //0 Azeroth
      ma->addWorldAreaMapId(1);   //1 Kalimdor
      ma->addWorldAreaMapId(530); //530 #Expansion 0
      ma->addWorldAreaMapId(509); //509 #AhnQiraj  
      ma->addWorldAreaMapId(469); //469 #BlackwingLair
      ma->addWorldAreaMapId(189); //189 #MonasteryInstances
      ma->addWorldAreaMapId(30);  //030 #PVPZone01
      ma->addWorldAreaMapId(37);  //037 #PVPZone02
      ma->addWorldAreaMapId(33);  //033 #Shadowfang
      ma->addWorldAreaMapId(533); //533 #Stratholme Raid
      ma->addWorldAreaMapId(209); //209 #TanarisInstance
      ma->addWorldAreaMapId(309); //309 #Zul'gurub
      ma->addWorldAreaMapId(560); //560 #HillsbradPast
      ma->addWorldAreaMapId(534); //534 #HyjalPast
      ma->addWorldAreaMapId(532); //532 #Karazahn
      ma->addWorldAreaMapId(543); //543 #HellfireRampart
      ma->addWorldAreaMapId(568); //568 #ZulAman
      ma->addWorldAreaMapId(564); //564 #BlackTemple
      
      return ma->ConvertWorld ();
    }
  
  return 1;
}
