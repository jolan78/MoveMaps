#include "MoveZone.h"
#include <time.h>
//#define DEBUG_COORDS
//#define DEBUG_PORTALS
//#define LAYER_CONNEXION
//#define DEBUG_GRID
//#define DEBUG_EXTEND
//#define DEBUG_ZONE_COMPLETE
// adds check in getLocalHeightAt() (and getLocalHeightAt) :
//#define CHECK_POINTINBOUNDS
//#define DBGVECTOR
#ifdef DBGVECTOR
Vector3 MZdbgVector=Vector3(15400.000000,15059.000000,92);
#endif

// max crossable height diff *4, 7=1.75
#define MAX_C_HEIGHT 7

namespace VMAP
{

  MovePortal::MovePortal (float pX1,float pY1,float pZ1,float tHeight,unsigned int pDirection, MoveZone* MZ)
    {
    X1=X2=pX1;
    Y1=Y2=pY1;
    Z1=Z2=pZ1; // debug
    destGridX=destGridY=0;
    tempHeight.append(tHeight);
    DestZoneID= (MZ!=NULL? MZ->getIndex() :INT_MAX);
    DestZone=MZ;

    direction=pDirection;
    }

  MovePortal::MovePortal (Vector3 low,Vector3 high/*debug*/,unsigned int destID,unsigned int pDirection)
    {
    X1=low.x;
    Y1=low.y;
    Z1=low.z;
    X2=high.x;
    Y2=high.y;
    Z2=high.z;
    DestZoneID=destID;
    direction=pDirection;
    }
  
  void
  MovePortal::extend()
    {
    switch (direction)
      {
      case EXTEND_N :
      case EXTEND_S :
        X2+=1;
        break;
      case EXTEND_E :
      case EXTEND_W :
        Y2+=1;
        break;
      }
    }

  void
  MovePortal::extend(float tHeight)
    {
    extend();
    tempHeight.append(tHeight);
    }
  
  void 
  MovePortal::connect(MoveZoneContainer* MZContainer)
    {
    unsigned int i=0;
    MoveZone* prevMZ=NULL;
    
    for (float x=X1;x<=X2;x++)
      {
      for (float y=Y1;y<=Y2;y++)
        {
        float destheight=tempHeight[i++];
        float dx=x;
        float dy=y;
        switch (direction)
          {
          case EXTEND_N :
            dy+=1;
            break;
          case EXTEND_S :
            dy-=1;
            break;
          case EXTEND_E :
            dx+=1;
            break;
          case EXTEND_W :
            dx-=1;
            break;
          }
        
        MoveZone* MZ=MZContainer->getMoveZoneByCoords(Vector3(dx,dy,destheight));
        if (MZ == NULL)
          {
          printf("Connexion not found in Portal %f,%f %f,%f (%d out of %d pts) at %f,%f,%f\n",X1,Y1,X2,Y2,i,tempHeight.size(),dx,dy,destheight);
          continue;
          }
        if (prevMZ == NULL)
          DestZoneID = MZ->getIndex();
        else if (prevMZ != MZ)
          {
          printf("Portal %f,%f %f,%f (%d out of %d pts) should be sptited at %f,%f,%f\n",X1,Y1,X2,Y2,i,tempHeight.size(),dx,dy,destheight);
          // split and returns new portal
          }
        prevMZ = MZ;
        }
      }
    }
    
  void
  MovePortal::save(FILE* fp)
  {
    fwrite (&DestZoneID,sizeof(unsigned int),1,fp);

    fwrite (&X1,sizeof(float),1,fp);
    fwrite (&Y1,sizeof(float),1,fp);
    fwrite (&Z1,sizeof(float),1,fp);
    fwrite (&X2,sizeof(float),1,fp);
    fwrite (&Y2,sizeof(float),1,fp);
    fwrite (&Z2,sizeof(float),1,fp);
    fwrite (&direction,sizeof(unsigned int),1,fp);
    fwrite (&destGridX,sizeof(unsigned int),1,fp);
    fwrite (&destGridY,sizeof(unsigned int),1,fp);
  }

  void
  MovePortal::load(FILE* fp)
  {
    fread (&DestZoneID,sizeof(unsigned int),1,fp);

    fread (&X1,sizeof(float),1,fp);
    fread (&Y1,sizeof(float),1,fp);
    fread (&Z1,sizeof(float),1,fp);
    fread (&X2,sizeof(float),1,fp);
    fread (&Y2,sizeof(float),1,fp);
    fread (&Z2,sizeof(float),1,fp);
    fread (&direction,sizeof(unsigned int),1,fp);
    fread (&destGridX,sizeof(unsigned int),1,fp);
    fread (&destGridY,sizeof(unsigned int),1,fp);
  }

  
  size_t hashCode (const MoveZone* pMZ)
    {
    return pMZ->getBounds ().hashCode ();
    }

  void getBounds (const MoveZone* pMZ, G3D::AABox& pAABox)
    {
    pAABox = pMZ->getBounds();
    }

 
 void
 MoveZoneContainer::generate(const MoveMapBox* iMoveMapBoxArray,int iNMoveMapBoxes,AABox gridBounds) {
    
    pMoveZones = new AABSPTree<MoveZone*>();
    MZ_Tree = new RStarTree<MoveZone*>(2, 4);
    moveZoneIndex=0;
    time_t sec1;
    time (&sec1);
    unsigned int sizeX, sizeY;
    
    for (int i = 0; i < iNMoveMapBoxes; ++i)
      {
      
      time_t boxsec1;
      time (&boxsec1);
      
      const MoveMapBox* moveMapBox = &iMoveMapBoxArray[i];
      
      Set<Vector2> reachedPos;
      
      const Vector3 basePos = moveMapBox->getBasePosition ();
      
      #ifdef DEBUG_GRID
      printf("grid bounds : %f,%f %f,%f\n",gridBounds.low().x,gridBounds.low().z,gridBounds.high().x,gridBounds.high().z);
      #endif
      
      sizeX = moveMapBox->getSizeX ();
      sizeY = moveMapBox->getSizeY ();

      printf("\n#moveMapBox (%i*%i) at %f,%f Bounds : %f,%f,%f %f,%f,%f\n",sizeX,sizeY,basePos.x,basePos.z,
             moveMapBox->getBounds().low().x,moveMapBox->getBounds().low().z,moveMapBox->getBounds().low().y,
             moveMapBox->getBounds().high().x,moveMapBox->getBounds().high().z,moveMapBox->getBounds().high().y);
      unsigned int BoxMZCount=0;


      // TODO : scan inside grid only : not that time consuming
      for (int y = 0; y < sizeY; ++y) // peraps posConnTable (MoveMapContainer::fillMoveMapConnectionManagerArray)
        {
          for (int x = 0; x < sizeX; ++x)
            {
              
              unsigned short val = moveMapBox->get ((float) x, (float) y);
              //printf("val at %f,%f : %i\n",x + diffBaseV.x, y + diffBaseV.y, val);
              if (val != MOVEMAP_VALUE_CANT_REACH && !reachedPos.contains(Vector2(x,y)) )
                { // TODO check orthogonal dir too
    // TODO : check connexion pts !!
                  //unsigned short testval = pcArray->directGet (x, y, 0);
                  /*if (testval == MAP_VALUE_UNDEF || zMoveMapLayer->isConnectionPoint(Vector3(startX,startY,testval)) )
                      return;*/
                  
                  outOfZonePoints.insert(Vector2(x,y));
                  while (outOfZonePoints.size() >0)
                    {
                    Set<Vector2>::Iterator itr=outOfZonePoints.begin();
                    Vector2 sPos=*itr;
                    outOfZonePoints.remove(sPos);

                    if (!reachedPos.contains(Vector2(sPos.x,sPos.y)) && !( sPos.x + basePos.x < gridBounds.low().x + 0.5 || sPos.y + basePos.z < gridBounds.low().z + 0.5 || sPos.x + basePos.x > gridBounds.high().x - 1.5 ||  sPos.y + basePos.z > gridBounds.high().z - 1.5  /* edge of the grid */ )) // todo : remove : we check at outOfZonePoints.push_back
                      {
                      /*if(BoxMZCount==0)
                        {
                        time_t tmpboxsec2;
                        time (&tmpboxsec2);
                        long diff = tmpboxsec2 - boxsec1;
                        printf("took %d sec. to find a valid start point in box\n",diff);
                        }*/

                      #ifdef DBGVECTOR
                      if (MZdbgVector.x == sPos.x + basePos.x && MZdbgVector.y == sPos.y + basePos.z)
                        printf("starting new zone %u at DBGVECTOR\n",moveZoneIndex);
                      #endif

                      #ifdef DEBUG_ZONE_COMPLETE
                      printf("\n#new zone id %i starting at %f,%f : %i (%i to go)\n",moveZoneIndex,sPos.x,sPos.y,val,outOfZonePoints.size());
                      #endif
                      MoveZone* MZ = new MoveZone (moveMapBox,moveZoneIndex,&sPos,&outOfZonePoints,&reachedPos,gridBounds);
  
                      AABox test= MZ->getBounds();
                      
                      //pMoveZones.append(*MZ);
                      pMoveZones->insert(MZ);
                      MZ_Tree->Insert(MZ);
                      pMoveZonesArray.append(MZ);
                      #ifdef DEBUG_ZONE_COMPLETE
                      printf("%u %f,%f %f,%f , H:%f (%f,%f)  -%i\n",moveZoneIndex ,test.low().x ,test.low().y, test.high().x ,test.high().y,test.high().z - test.low().z , test.low().z,test.high().z, outOfZonePoints.size());
                      #endif
                      BoxMZCount++;
                      moveZoneIndex++;
                      }
                    #ifdef DEBUG_GRID
                    else
                      printf("out of grid edge : %f,%f\n",sPos.x + basePos.x,sPos.y + basePos.z);
                    #endif
                    }
                  
                  // FIXME ugly goto
                  // Not correct : some boxes are not continous
                  /*if (BoxMZCount>0)
                    goto debug;*/
                }
            }
        }
      debug:
      time_t boxsec2;
      time (&boxsec2);
      long diff = boxsec2 - boxsec1;
      printf("generated %u zones in %d sec.\n",BoxMZCount,diff);
    }
  
  time_t portalsec1;
  time (&portalsec1);
  // connect portals
  for (unsigned int i=0; i < pMoveZonesArray.size();i++)
    {
    Array<MovePortal*> iPortals;
    pMoveZonesArray[i]->setPortalArray(&iPortals);
    
    for (unsigned short j=0; j<4; ++j)
      {
      connectLayerPortals(pMoveZonesArray[i],j);
      #ifdef LAYER_CONNEXION
      printf("built %d layerportals\n",pMoveZonesArray[i]->getPortalArray().size());
      #endif
      
      connectPortals(pMoveZonesArray[i],j);
      
      #ifdef DEBUG_PORTALS
      printf("built %d portals (layer+zone)\n",pMoveZonesArray[i]->getPortalArray().size());
      #endif
      }
    }
  time_t portalsec2;
  time (&portalsec2);
  long diff = portalsec2 - portalsec1;
  printf("connected %u zones in %d sec.\n",moveZoneIndex,diff);
  
  time_t sec2;
  time (&sec2);
  diff = sec2 - sec1;
  
  printf("####finished in %d sec for %u zones\n",diff,moveZoneIndex);

  }

  void
  MoveZoneContainer::connectLayerPortals(MoveZone* iMoveZone,unsigned int direction)
    {
    #ifdef LAYER_CONNEXION
    printf ("############################ connectLayerPortals for MZ %d ###########\n",iMoveZone->getIndex());
    #endif
    Table<Vector2, Vector3> * layerPortals=iMoveZone->getLayerConnexions(direction);
    if (layerPortals->size()==0)
      {
      #ifdef LAYER_CONNEXION
      printf ("no LayerPortals\n");
      #endif
      return;
      }
    AABox bounds=iMoveZone->getBounds();
    AABox fromBounds;
    iMoveZone->getBoxBounds(fromBounds);
    
    MoveZone* prevMZ=NULL;
    Array<MoveZone*> MZArray;
    MoveZone* curMZ=NULL;
    MovePortal* CurrentPortal;
    Array<MovePortal*> iPortals = iMoveZone->getPortalArray();
    Vector3 curpos,prevpos;
    bool previsportal=false;

    #ifdef LAYER_CONNEXION
    printf ("bounds %f,%f %f,%f\n%d layer portals\n",bounds.low().x,bounds.low().y,bounds.high().x,bounds.high().y,layerPortals->size());
    #endif
    
    float x1,x2,y1,y2;
    switch (direction)
      {
      case EXTEND_N :
        #ifdef LAYER_CONNEXION
        printf("check N:\n");
        #endif
        x1=bounds.low().x +0.5f;
        x2=bounds.high().x -0.5f;
        y1=y2=bounds.high().y -0.5f;
      break;
      case EXTEND_S :
        #ifdef LAYER_CONNEXION
        printf("check S:\n");
        #endif
        x1=bounds.low().x +0.5f;
        x2=bounds.high().x -0.5f;
        y1=y2=bounds.low().y +0.5f;
      break;
      case EXTEND_E :
        #ifdef LAYER_CONNEXION
        printf("check E:\n");
        #endif
        x1=x2=bounds.high().x -0.5f;
        y1=bounds.low().y +0.5f;
        y2=bounds.high().y -0.5f;
      break;
      case EXTEND_W :
        #ifdef LAYER_CONNEXION
        printf("check W:\n");
        #endif
        x1=x2=bounds.low().x +0.5f;
        y1=bounds.low().y +0.5f;
        y2=bounds.high().y -0.5f;
      break;
      
      }
    
    
    float lowZ,highZ;
    for (float x=x1 ; x<=x2 ;++x) 
      {
      for (float y=y1 ; y<=y2 ;++y)
        {
        curMZ=NULL;
        #ifdef LAYER_CONNEXION
        printf("checking %f,%f\n",x,y);
        #endif
        if (layerPortals->get(Vector2(x,y),curpos))
          {
          #ifdef LAYER_CONNEXION
          printf("LayerPortal at %f,%f -> %f,%f,%f\n",x,y,curpos.x,curpos.y,curpos.z);
          #endif
          lowZ=curpos.z-1.8;
          highZ=curpos.z+1.8;
           
          MZArray=getMoveZonesByZRange(curpos.x,curpos.y,lowZ,highZ);

          if (MZArray.size() == 0)
            {
            #ifdef LAYER_CONNEXION
            printf("Layer Connexion not found in MZ %d layerPortal %f,%f -> %f,%f,%f\n",iMoveZone->getIndex(),x,y,curpos.x,curpos.y,curpos.z);
            #endif
            previsportal=false;
            }
          
          for (unsigned int i=0; i<MZArray.size(); ++i)
            {
            MoveZone* tmpMZ=MZArray[i];
            float h=tmpMZ->getGlobalHeightAt(curpos.x,curpos.y);
            if (h != FLOAT_HEIGHT_CANT_REACH && abs(h - curpos.z) < 1.8)
              {
              curMZ=tmpMZ;
              #ifdef LAYER_CONNEXION
              printf("Layer Connexion found : MZ %d -> %d layerPortal %f,%f -> %f,%f,%f : %f\n",iMoveZone->getIndex(),curMZ->getIndex(),x,y,curpos.x,curpos.y,curpos.z,h);
              #endif
              break;
              }
            else
              {
              #ifdef LAYER_CONNEXION
              printf("mz %u too far away %f,%f (%f) %s\n",tmpMZ->getIndex(),h,curpos.z,abs(h - curpos.z),(h == FLOAT_HEIGHT_CANT_REACH?"UNDEF":""));
              #endif
              }
            }
          if (curMZ == NULL)
            {
            #ifdef LAYER_CONNEXION
            printf("Layer Connexion not found in MZ %d layerPortal %f,%f -> %f,%f,%f\n",iMoveZone->getIndex(),x,y,curpos.x,curpos.y,curpos.z);
            #endif
            previsportal=false;
            }
          else
            {
            #ifdef LAYER_CONNEXION
            if (previsportal)
              printf("prev MZ is %s %u, %u\n",(curMZ == prevMZ?"identical":"different"),curMZ->getIndex(),(prevMZ != NULL ? prevMZ->getIndex() : INT_MAX));
            #endif
            if (previsportal && curMZ == prevMZ)
              {
              CurrentPortal->extend(bounds.high().z); // TODO : better manage z if needed
            
              }
            else
              {
              //printf("new portal\n");
              CurrentPortal= new MovePortal(x,y,bounds.high().z,curpos.z, direction ,curMZ);// TODO : better manage z if needed
              iPortals.append(CurrentPortal);
              }
            }
          prevMZ=curMZ;

          prevpos=curpos;
          if (prevMZ)
            previsportal=true;
          }
        else
          previsportal=false;
        }
      }
    
    iMoveZone->setPortalArray(&iPortals);
    }

  void
  MoveZoneContainer::connectPortals(MoveZone* iMoveZone,unsigned int direction)
    {
    #ifdef DEBUG_PORTALS
    printf ("############################ connectPortals for MZ %d ###########\n",iMoveZone->getIndex());
    #endif
    AABox bounds=iMoveZone->getBounds();
    Table<Vector2, Vector3> * PortalCells=iMoveZone->getPortalCell(direction);
    MoveZone* prevMZ=NULL;
    MoveZone* curMZ=NULL;
    MovePortal* CurrentPortal;
    Array<MovePortal*> iPortals = iMoveZone->getPortalArray();
    Vector3 curpos,prevpos;
    bool previsportal=false;
    AABox pBoxBounds;

    iMoveZone->getBoxBounds (pBoxBounds);
    
    #ifdef DEBUG_PORTALS
    printf ("bounds %f,%f %f,%f\n",bounds.low().x,bounds.low().y,bounds.high().x,bounds.high().y);
    #endif
    
    float x1,x2,y1,y2;
    switch (direction)
      {
      case EXTEND_N :
        #ifdef DEBUG_PORTALS
        printf("check N\n");
        #endif
        x1=bounds.low().x +0.5f;// xy bounds are stretch by 0.5 in order to allow faster zone finding
        x2=bounds.high().x -0.5f;
        y1=y2=bounds.high().y -0.5f;
      break;
      case EXTEND_S :
        #ifdef DEBUG_PORTALS
        printf("check S\n");
        #endif
        x1=bounds.low().x +0.5f;
        x2=bounds.high().x -0.5f;
        y1=y2=bounds.low().y +0.5f;
      break;
      case EXTEND_E :
        #ifdef DEBUG_PORTALS
        printf("check E\n");
        #endif
        x1=x2=bounds.high().x -0.5f;
        y1=bounds.low().y +0.5f;
        y2=bounds.high().y -0.5f;
      break;
      case EXTEND_W :
        #ifdef DEBUG_PORTALS
        printf("check W\n");
        #endif
        x1=x2=bounds.low().x +0.5f;
        y1=bounds.low().y +0.5f;
        y2=bounds.high().y -0.5f;
      break;
      
      }
    
    unsigned int portalID=iPortals.size(); // ID of portalcell = id of portal if at grid edge 
    for (float x=x1 ; x<=x2 ;++x) 
      {
      for (float y=y1 ; y<=y2 ;++y)
        {
        #ifdef DEBUG_PORTALS
        printf("checking %f,%f\n",x,y);
        #endif
        if (PortalCells->get(Vector2(x,y),curpos))
          {
          curMZ=getMoveZoneByCoords(curpos);

          if (curMZ == NULL)
            {
            // unfound portals should occur only at grid edge.
            if (curpos.x<0 || curpos.x> pBoxBounds.high().x - pBoxBounds.low().x -1 ||
               curpos.y<0 || curpos.y> pBoxBounds.high().z - pBoxBounds.low().z -1)
              {
              unsigned int* tmp=(unsigned int*)malloc(2*sizeof(unsigned int));
              tmp[0]=iMoveZone->getIndex();
              tmp[1]=portalID;
              
              
printf("gridportal : %u,%u %f,%f,%f\n",tmp[0],tmp[1],curpos.x,curpos.y,curpos.z);
              gridPortals[direction].set(tmp,curpos.z);
              }
            else
              printf("Connexion not found in MZ %d PortalCell %f,%f -> %f,%f,%f\n",iMoveZone->getIndex(),x,y,curpos.x,curpos.y,curpos.z);
            
            #ifdef DEBUG_PORTALS
            Array<MoveZone*> MZArray=getMoveZonesByZRange(curpos.x,curpos.y,curpos.z - 1, curpos.z + 1);
            for (unsigned int i=0; i<MZArray.size(); ++i)
              {
              MoveZone* tmpMZ=MZArray[i];
              float h=tmpMZ->getGlobalHeightAt(curpos.x,curpos.y);
              printf ("candidate : MZ %d h:%f\n",tmpMZ->getIndex(),h);
              if (h != FLOAT_HEIGHT_CANT_REACH && abs(h - curpos.z) < 1.8)
                printf("match\n");
              else
                printf("dont match\n");
              }
            #endif
            previsportal=false;
            }
          #ifdef DEBUG_PORTALS
          if (previsportal)
            printf("prev MZ is %s %u, %u\n",(curMZ == prevMZ?"identical":"different"),curMZ->getIndex(),(prevMZ != NULL ? prevMZ->getIndex() : INT_MAX));
          #endif
          
          if (previsportal && curMZ == prevMZ)
            {
            CurrentPortal->extend(bounds.high().z); // TODO : better manage z if needed
          
            }
          else
            {
            CurrentPortal= new MovePortal(x,y,bounds.high().z,curpos.z, direction ,curMZ);// TODO : better manage z if needed
            iPortals.append(CurrentPortal);
            }
          prevMZ=curMZ;

          prevpos=curpos;
          if (prevMZ)
            previsportal=true;
          }
        else
          previsportal=false;
        
        portalID++;
        }
      }
    
    #ifdef DEBUG_PORTALS
    Array<Vector2> origs = PortalCells->getKeys();
    
    for (int o=0; o<origs.length(); o++)
      printf("with PC %f,%f -> %f,%f,%f\n",origs[o].x,origs[o].y,PortalCells->get(origs[o]).x,PortalCells->get(origs[o]).y,PortalCells->get(origs[o]).z);
    
    for (unsigned int i=0; i< iPortals.size(); i++)
      printf("Portal from %f,%f,%f to: %f,%f,%f\n",iPortals[i]->getLow().x,iPortals[i]->getLow().y,iPortals[i]->getLow().z,iPortals[i]->getHigh().x,iPortals[i]->getHigh().y,iPortals[i]->getHigh().z );
    
    printf("MZ %d has %d portals\n",iMoveZone->getIndex(),iPortals.size());
    #endif
    
    iMoveZone->setPortalArray(&iPortals);
    
    }
  
  
  MoveZone*
  MoveZoneContainer::getMoveZoneByCoords (const Vector3& pPos) const
    {
    return MZ_Tree->FindOneLeafContaining(&pPos);
    }

  Array<MoveZone*>
  MoveZoneContainer::getMoveZonesByZRange (const float x, const float y, const float lowZ, const float highZ) const
    {
    return MZ_Tree->FindLeavesByZRange(x, y, lowZ, highZ);
    }
  
  MoveZone::MoveZone (const MoveMapBox* iMoveMapBox,unsigned int ZoneIndex,Vector2* sPos,Set<Vector2>* outOfZonePoints,Set<Vector2>* reachedPos,AABox iGridBounds) {
    iIndex=ZoneIndex;
    myOutOfZonePoints=outOfZonePoints;
    myReachedPos=reachedPos;
    zMoveMapBox=iMoveMapBox;    
    gridBounds=iGridBounds;
    const Vector3 basePos = zMoveMapBox->getBasePosition ();
    
    unsigned int initZ=getLocalCompressedHeightAt(sPos->x, sPos->y);
    
    low=Vector3(sPos->x,sPos->y,initZ);
    high=Vector3(sPos->x,sPos->y,initZ);
    #ifdef DEBUG_COORDS
    printf ("start val : %d\n",initZ);
    printf("reached %f,%f (%f,%f)\n",basePos.x+sPos->x,basePos.z+sPos->y,sPos->x,sPos->y);
    #endif
    myReachedPos->insert(Vector2(sPos->x,sPos->y));
    
    unsigned int extender=0xF;
    
    unsigned int debugextiter=0;
    for (unsigned int i=0; extender;i++)
      {
      unsigned int mask=1<<(i%4);
      if (extender&mask)
          {
          debugextiter++;
          if (!Extend(i%4,extender))
              {
              extender &= ~mask;
              }
          }
      assert (i < 4096);
      }
    
//    Vector3 bp = layerBounds.low();
    // TODO : remove grid edge check
    low.x += basePos.x;
    //if (low.x > gridBounds.low().x +0.5)
      low.x -= 0.5;
    low.y += basePos.z;
    //if (low.y > gridBounds.low().z +0.5)
      low.y -= 0.5;
    low.z = zMoveMapBox->getFloatHeight (low.z) -0.1;
    high.x += basePos.x;
    //if (high.x < gridBounds.high().x -1.5)
      high.x += 0.5;
    high.y += basePos.z;
    //if (high.y < gridBounds.high().z -1.5)
      high.y += 0.5;
    high.z = zMoveMapBox->getFloatHeight (high.z) +0.1;
    
    
    
    iBounds.set(low,high);
    
    // uncompress portalcells

    for (unsigned short i=0; i<4; i++)
      {
      Array<Vector2> PCorigs = PortalCells[i].getKeys();
      for (int o=0; o<PCorigs.length(); o++)
        {
        PortalCells[i][PCorigs[o]].z=zMoveMapBox->getFloatHeight (PortalCells[i][PCorigs[o]].z);
        }
      }

    #ifdef DEBUG_ZONE_COMPLETE
    printf("finished zone after %i iterations. portalscells N:%d, S:%d E:%d W:%d:\n",debugextiter,PortalCells[EXTEND_N].size(),PortalCells[EXTEND_S].size(),PortalCells[EXTEND_E].size(),PortalCells[EXTEND_W].size());
    /*for (unsigned int i=0; i< iPortals.size(); i++)
      printf("Portal from %f,%f,%f to: %f,%f,%f : %d\n",iPortals[i]->getLow().x,iPortals[i]->getLow().y,iPortals[i]->getLow().z,iPortals[i]->getHigh().x,iPortals[i]->getHigh().y,iPortals[i]->getHigh().z );*/
    #endif
      }
  
  bool
  MoveZone::Extend(unsigned int direction,unsigned int extendermask)
      {
      int lineX1,lineX2,lineY1,lineY2;
      short int deltatestX=0;
      short int deltatestY=0;
      short int prevExtendCellX=0;
      short int prevExtendCellY=0;
      short int sizeX=0 ,sizeY=0;
      bool broken=false;
      Vector3 newhigh=high;
      Vector3 newlow=low;
      Table<Vector2, Vector3> tempPortalCells;
      Set<Vector2> tmpOutOfZonePoints;// todo : move to G3D::Set
      Table<Vector2, Vector3> tempLayerConnexions;
      bool addPortalToHigh=false;
      bool addPortalToLow=false;
      unsigned int lowEdgeDirection,highEdgeDirection;
      
      AABox pBoxBounds = zMoveMapBox->getBounds ();

      const Vector3 basePos = zMoveMapBox->getBasePosition ();
      
      MovePortal* CurrentPortal = NULL;
      Array<MovePortal*> TempMovePortals;
      unsigned int prevTestHeight=0;
      
#ifdef DEBUG_COORDS
printf("box : %f,%f,%f,%f\n",low.x+basePos.x,low.y+basePos.z,high.x+basePos.x,high.y+basePos.z);
#endif
      
      switch (direction)
        {
        case EXTEND_N :
          lineY1=lineY2=high.y+1;
          lineX1=low.x;
          lineX2=high.x;
          sizeY=1;
          sizeX=high.x-low.x;
          deltatestY=-1;
          prevExtendCellX=-1;
          newhigh.y += 1;
          if (!(extendermask & 1<<EXTEND_W))
            {
            addPortalToLow=true;
            lowEdgeDirection=EXTEND_W;
            }
          if (!(extendermask& 1<<EXTEND_E))
            {
            addPortalToHigh=true;
            highEdgeDirection=EXTEND_E;
            }

#ifdef DEBUG_EXTEND
printf ("N:%d,%d y1=%f \n",direction,extendermask,lineY1+basePos.z);
#endif
          break;
        case EXTEND_S :
          lineY1=lineY2=low.y-1;
          lineX1=low.x;
          lineX2=high.x;
          sizeY=1;
          sizeX=high.x-low.x;
          deltatestY=1;
          prevExtendCellX=-1;
          newlow.y -= 1;
          if (!(extendermask& 1<<EXTEND_W))
            {
            addPortalToLow=true;
            lowEdgeDirection=EXTEND_W;
            }
          if (!(extendermask& 1<<EXTEND_E))
            {
            addPortalToHigh=true;
            highEdgeDirection=EXTEND_E;
            }
#ifdef DEBUG_EXTEND
printf ("S:%d,%d y1=%f \n",direction,extendermask,lineY1+basePos.z);
#endif
          break;
        case EXTEND_E :
          lineX1=lineX2=high.x+1;
          lineY1=low.y;
          lineY2=high.y;
          sizeX=1;
          sizeY=high.y-low.y;
          deltatestX=-1;
          prevExtendCellY=-1;
          newhigh.x += 1;
          if (!(extendermask& 1<<EXTEND_S))
            {
            addPortalToLow=true;
            lowEdgeDirection=EXTEND_S;
            }
          if (!(extendermask& 1<<EXTEND_N))
            {
            addPortalToHigh=true;
            highEdgeDirection=EXTEND_N;
            }
#ifdef DEBUG_EXTEND
printf ("E:%d,%d x1=%f \n",direction,extendermask,lineX1+basePos.x);
#endif
          break;
        case EXTEND_W :
          lineX1=lineX2=low.x-1;
          lineY1=low.y;
          lineY2=high.y;
          sizeX=1;
          sizeY=high.y-low.y;
          deltatestX=1;
          prevExtendCellY=-1;
          newlow.x -= 1;
          if (!(extendermask& 1<<EXTEND_S))
            {
            addPortalToLow=true;
            lowEdgeDirection=EXTEND_S;
            }
          if (!(extendermask& 1<<EXTEND_N))
            {
            addPortalToHigh=true;
            highEdgeDirection=EXTEND_N;
            }
#ifdef DEBUG_EXTEND
printf ("W:%d,%d x1=%f \n",direction,extendermask,lineX1+basePos.x);
#endif
          break;
        }

/*
old check layer bounds : (lineX1>=0 && lineX2<(layerBounds.high ().x -layerBounds.low ().x) && lineY1>=0 && lineY2<(layerBounds.high ().z -layerBounds.low ().z) )
(lineX1+layerBounds.low().x >= pcArray->getBounds().low().x && lineX2+layerBounds.low().x < pcArray->getBounds().high ().x && lineY1+layerBounds.low().z >= pcArray->getBounds().low().z && lineY2+layerBounds.low().z < pcArray->getBounds().high().z)
*/

     // is this usefull ?
//     if (lineX1>=0 && lineX2+layerBounds.low().x < layerBounds.high().x && lineY1>=0  && lineY2+layerBounds.low().z < layerBounds.high().z) // todo check if Layer::containsXZ is apropriate
       
       for (int x =lineX1; x<=lineX2; x++) // = lineX1 - 1 ?
         {
         for ( int y =lineY1; y<=lineY2; y++)
           {

//           outcell[x-lineX1][y-lineY1]=false;
  
           bool isportal=false;
           
           /*
           (x,y) is the 'test' point, out of the current zone, his height is testval
           (x+diffBaseV.x, y+diffBaseV.z) is the 'from' point, inside the zone, leading to the test point, his height is fromval
           coords are relative to the layer base position
           */
           
           unsigned int fromval = getLocalCompressedHeightAt(x+deltatestX, y+deltatestY);
           assert (fromval != MOVEMAP_VALUE_CANT_REACH);
           //float fromval = SHORTHEIGHT2FLOAT (pcArray->directGet (x+deltatestX+diffBaseV.x, y+deltatestY+diffBaseV.z, 0));
           unsigned int testval;
           if (x<0 || x> pBoxBounds.high().x - pBoxBounds.low().x -1 ||
               y<0 || y> pBoxBounds.high().z - pBoxBounds.low().z -1)
             testval = MOVEMAP_VALUE_CANT_REACH; // the testpoint is out of layer's box
           else
             {
             testval = getLocalCompressedHeightAt(x, y);
             }
            
            // DEBUG
            //assert(fromval < 2000);
#ifdef DBGVECTOR
if ((MZdbgVector.x == x+ basePos.x+deltatestX && MZdbgVector.y == y+ basePos.z+deltatestY) ||
    (MZdbgVector.x == x+ basePos.x && MZdbgVector.y == y+ basePos.z))
      {
      printf ("FOUND DBGVECTOR in MZ %u ",iIndex);
      if (MZdbgVector.x == x+ basePos.x+deltatestX && MZdbgVector.y == y+ basePos.z+deltatestY)
        printf ("in orig%s\n",(zMoveMapBox->getFloatHeight (fromval) == MZdbgVector.z ? " EXACT":""));
      else
        printf("in dest%s\n",(zMoveMapBox->getFloatHeight (testval) == MZdbgVector.z ? " EXACT":""));
      printf("orig = %f,%f %f getGlobalHeightAt:%f%s\n",x+ basePos.x+deltatestX, y+ basePos.z+deltatestY, zMoveMapBox->getFloatHeight (fromval),getGlobalHeightAt(x+ basePos.x+deltatestX, y+ basePos.z+deltatestY),(fromval==MOVEMAP_VALUE_CANT_REACH?" NA":""));
      printf("dest = %f,%f %f getGlobalHeightAt:%f%s\n",x+ basePos.x, y+ basePos.z, zMoveMapBox->getFloatHeight (testval),getGlobalHeightAt(x+ basePos.x, y+ basePos.z),(testval==MOVEMAP_VALUE_CANT_REACH?" NA":""));
      
      }
#endif  
           // in MoveMapBox::getCharHeight : -0.5f ??
           
           if (newhigh.z < testval) // TODO compress
             newhigh.z = testval;
           if (newlow.z > testval)
             newlow.z = testval;
             
           if (testval == MOVEMAP_VALUE_CANT_REACH ||
               myReachedPos->contains(Vector2(x, y)) ||
               abs( (int)testval - (int)fromval) > MAX_C_HEIGHT /* FIXME: getMaxZDiff ? getZDiff ? VectorMoveMap.h*/ ||
               (x+prevExtendCellX >= lineX1 && y+prevExtendCellY >= lineY1) && abs( (int)testval - (int)prevTestHeight) > MAX_C_HEIGHT /*TODO : check if thats necessary */
               || x + basePos.x < gridBounds.low().x + 0.5 || y + basePos.z < gridBounds.low().z + 0.5 || x + basePos.x > gridBounds.high().x - 1.5 ||  y + basePos.z > gridBounds.high().z - 1.5  /* edge of the grid */ )
             {
             
#ifdef DEBUG_COORDS
printf("at %f:%f (%i,%i)",x+diffBaseV.x,y+diffBaseV.z,x,y);
if (myReachedPos->contains(Vector2(x, y)))
  printf("already reached");
if (testval == MOVEMAP_VALUE_CANT_REACH)
  printf(" testval: Undefined");
else
  printf(" testval:%f", zMoveMapBox->getFloatHeight (testval));
if (fromval == MOVEMAP_VALUE_CANT_REACH)
  printf(" fromval: Undefined");
else
  printf(" fromval:%f", zMoveMapBox->getFloatHeight (fromval));
if ((x+prevExtendCellX >= lineX1 && y+prevExtendCellY >= lineY1) && abs( (int)testval - (int)prevTestHeight) > MAX_C_HEIGHT)
  printf(" prevTestHeight too different : %f cur[%d,%d] prev[%d,%d]",zMoveMapBox->getFloatHeight (prevTestHeight),x,y,x+prevExtendCellX,y+prevExtendCellY);
if ((testval != MOVEMAP_VALUE_CANT_REACH) && abs( (int)testval - (int)fromval) > MAX_C_HEIGHT )
  printf(" tooHigh");
#endif
             myReachedPos->insert(Vector2(x+deltatestX, y+deltatestY));
             if ((testval != MOVEMAP_VALUE_CANT_REACH) && abs( (int)testval - (int)fromval) <= MAX_C_HEIGHT)
               {
               isportal=true;
               }
             
             // if the test point xy is out of layer or from pt z is near the floor/ceil of the layer then it could be a layer-to-layer (or grid) connexion
             if (testval == MOVEMAP_VALUE_CANT_REACH) // FIXME  : maybe a FLOAT_HEIGHT_CANT_REACH val exists at a far away height
               {
               #ifdef DBGVECTOR
               if (MZdbgVector.x == x+ basePos.x+deltatestX && MZdbgVector.y == y+ basePos.z+deltatestY)
                 printf("adding tempLayerConnexions at DBGVECTOR X,Y : %f,%f fromval : %f dist to floor : %f dist to ceil : %f\n",x+ basePos.x,y+ basePos.z,zMoveMapBox->getFloatHeight (fromval),fromval/4,((pBoxBounds.high().y-pBoxBounds.low().y)-fromval)/4);
               #endif

               tempLayerConnexions.set(Vector2(x+ pBoxBounds.low().x + deltatestX, y+ pBoxBounds.low().z + deltatestY),Vector3(x+ pBoxBounds.low().x, y+ pBoxBounds.low().z,fromval));
               
               }
             
#ifdef DEBUG_COORDS
printf("\n");
#endif
             broken=true;
             }
           else
             {
             isportal=true;

//             outcell[x-lineX1][y-lineY1]=true;// will be added to myOutOfZonePoints
#ifdef DEBUG_COORDS
printf("at %f:%f (%i,%i) OK\n",x+basePos.x,y+basePos.z,x, y);
#endif
             }
           if (isportal)
             {
             tempPortalCells.set(Vector2(x+ basePos.x + deltatestX, y+ basePos.z + deltatestY),
                                         Vector3(x+ basePos.x, y+ basePos.z, testval));
#ifdef DEBUG_PORTALS
printf("isportal at %f,%f dz:%f\n",x+ basePos.x + deltatestX, y+ basePos.z + deltatestY,zMoveMapBox->getFloatHeight (testval));
#endif
             }
           else
             {
#ifdef DEBUG_PORTALS
printf("not a portal at %f,%f\n",x+ basePos.x + deltatestX, y+ basePos.z + deltatestY);
#endif
             }
           
           if (testval != MOVEMAP_VALUE_CANT_REACH/* && !myOutOfZonePoints->contains(Vector2(x,y))*/)
             {
             #ifdef DBGVECTOR
             if ((MZdbgVector.x == x+ basePos.x && MZdbgVector.y == y+ basePos.z))
               printf ("Adding DBGVECTOR to tmpOutOfZonePoints\n");
             #endif
             tmpOutOfZonePoints.insert(Vector2(x,y));
             }

           prevTestHeight= testval;
           }
         }
      
     // now we check if cells at edges of extended line are portals
     Vector3 dummy;
     if (addPortalToLow && !PortalCells[lowEdgeDirection].get(Vector2(lineX1 + deltatestX + prevExtendCellX,lineY1 + deltatestY + prevExtendCellY),dummy) )
       {
       unsigned int testval;
       if (lineX1+deltatestX+prevExtendCellX >=0 && lineY1+deltatestY+prevExtendCellY >=0)
         testval = getLocalCompressedHeightAt(lineX1+deltatestX+prevExtendCellX, lineY1+deltatestY+prevExtendCellY);
       else
         testval=MOVEMAP_VALUE_CANT_REACH;
       unsigned int fromval = getLocalCompressedHeightAt(lineX1+deltatestX, lineY1+deltatestY);
#ifdef DEBUG_PORTALS
printf("while testing lowedge at %f,%f : Hdiff: %f testval: %f (%f,%f) : %f,%hd,%hd\n",lineX1 + deltatestX + basePos.x,lineY1 + deltatestY + basePos.z,zMoveMapBox->getFloatHeight (abs( (int)testval - (int)fromval)), zMoveMapBox->getFloatHeight (testval),lineX1+deltatestX+prevExtendCellX , lineY1+deltatestY+prevExtendCellY ,lineY1,deltatestY ,prevExtendCellY);
#endif
       // if the test point xy is out of layer or from pt z is near the floor/ceil of the layer then it could be a layer-to-layer (or grid) connexion
       if (testval == MOVEMAP_VALUE_CANT_REACH)
         {
         
         #ifdef DBGVECTOR
         if (MZdbgVector.x == lineX1+ basePos.x+deltatestX && MZdbgVector.y == lineY1+ basePos.z+deltatestY)
           printf("lowedge : adding LayerConnexions[%d] at DBGVECTOR X,Y : %f,%f fromval : %f dist to floor : %f dist to ceil : %f\n",lowEdgeDirection,lineX1+ basePos.x,lineY1+ basePos.z,zMoveMapBox->getFloatHeight (fromval),fromval/4,((pBoxBounds.high().y-pBoxBounds.low().y)-fromval)/4);
         #endif
         LayerConnexions[lowEdgeDirection].set(Vector2(lineX1 + deltatestX + basePos.x , lineY1 + deltatestY + basePos.z),Vector3(lineX1 + deltatestX + prevExtendCellX + basePos.x,lineY1 + deltatestY + prevExtendCellY + basePos.z, zMoveMapBox->getFloatHeight (fromval)));
         }
       else if (abs( (int)testval - (int)fromval) <= MAX_C_HEIGHT)
         {
#ifdef DEBUG_PORTALS
printf("cell - %f,%f -> %f,%f is a portal (%d,%d)\n",lineX1 + deltatestX + basePos.x,lineY1 + deltatestY + basePos.z,lineX1 + deltatestX + prevExtendCellX + basePos.x,lineY1 + deltatestY + prevExtendCellY + basePos.z,direction,extendermask);
#endif
         #ifdef DBGVECTOR
         if ((MZdbgVector.x == lineX1 + deltatestX + basePos.x && MZdbgVector.y == lineY1 + deltatestY + basePos.z))
           printf ("Adding DBGVECTOR LE to PortalCells orig\n");
         if ((MZdbgVector.x == lineX1 + deltatestX + prevExtendCellX + basePos.x && MZdbgVector.y == lineY1 + deltatestY + prevExtendCellY + basePos.z))
           printf ("Adding DBGVECTOR LE to PortalCells dest\n");
         #endif
         PortalCells[lowEdgeDirection].set(Vector2(lineX1 + deltatestX + basePos.x,lineY1 + deltatestY + basePos.z), 
                         Vector3(lineX1 + deltatestX + prevExtendCellX + basePos.x,lineY1 + deltatestY + prevExtendCellY + basePos.z, testval));
         }
       if (testval != MOVEMAP_VALUE_CANT_REACH /*&& !myOutOfZonePoints->contains(Vector2(lineX1 + deltatestX + prevExtendCellX,lineY1 + deltatestY + prevExtendCellY))*/)
         {
         #ifdef DBGVECTOR
         if ((MZdbgVector.x == lineX1 + deltatestX + prevExtendCellX + basePos.x && MZdbgVector.y == lineY1 + deltatestY + prevExtendCellY + basePos.z))
           printf ("Adding DBGVECTOR LE to myOutOfZonePoints\n");
         #endif

         myOutOfZonePoints->insert(Vector2(lineX1 + deltatestX + prevExtendCellX,lineY1 + deltatestY + prevExtendCellY));
         }
       }
      
      if (addPortalToHigh && !PortalCells[highEdgeDirection].get(Vector2(lineX2 + deltatestX -prevExtendCellX,lineY2 + deltatestY -prevExtendCellY),dummy))
        {
        unsigned int testval;
        if (lineX2 + deltatestX -prevExtendCellX<(pBoxBounds.high ().x -pBoxBounds.low ().x) && lineY2 + deltatestY -prevExtendCellY<(pBoxBounds.high ().z -pBoxBounds.low ().z) )
          testval = getLocalCompressedHeightAt(lineX2 + deltatestX -prevExtendCellX, lineY2 + deltatestY -prevExtendCellY);
        else
          testval=MOVEMAP_VALUE_CANT_REACH;
        unsigned fromval = getLocalCompressedHeightAt(lineX2 + deltatestX, lineY2 + deltatestY);

#ifdef DEBUG_PORTALS
printf("while testing highedge at %f,%f : Hdiff: %f testval: %f\n",lineX2 + deltatestX + basePos.x,lineY2 + deltatestY + basePos.z,zMoveMapBox->getFloatHeight(abs( (int)testval - (int)fromval)), zMoveMapBox->getFloatHeight (testval));
#endif
       // if the test point xy is out of layer or from pt z is near the floor/ceil of the layer then it could be a layer-to-layer (or grid) connexion
       if (testval == MOVEMAP_VALUE_CANT_REACH)
         {
         #ifdef DBGVECTOR
         if (MZdbgVector.x == lineX2+ basePos.x+deltatestX && MZdbgVector.y == lineY2+ basePos.z+deltatestY)
           printf("highedge : adding LayerConnexions[%d] at DBGVECTOR X,Y : %f,%f fromval : %f dist to floor : %f dist to ceil : %f\n",highEdgeDirection,lineX2+ basePos.x,lineY2+ basePos.z,zMoveMapBox->getFloatHeight (fromval),fromval/4,((pBoxBounds.high().y-pBoxBounds.low().y)-fromval)/4);
         #endif
         
         LayerConnexions[highEdgeDirection].set(Vector2(lineX2 + deltatestX + basePos.x , lineY2 + deltatestY + basePos.z),Vector3(lineX2 + deltatestX -prevExtendCellX + basePos.x,lineY2 + deltatestY -prevExtendCellY + basePos.z, zMoveMapBox->getFloatHeight (fromval)));
         
         }
        else if (abs( (int)testval - (int)fromval) <= MAX_C_HEIGHT)
          {
#ifdef DEBUG_PORTALS
printf("cell + %f,%f -> %f,%f is a portal (%d,%d)\n",lineX2 + deltatestX + basePos.x,lineY2 + deltatestY + basePos.z,lineX2 + deltatestX -prevExtendCellX + basePos.x,lineY2 + deltatestY -prevExtendCellY + basePos.z,direction,extendermask);
#endif
         #ifdef DBGVECTOR
         if ((MZdbgVector.x == lineX2 + deltatestX + basePos.x && MZdbgVector.y == lineY2 + deltatestY + basePos.z))
           printf ("Adding DBGVECTOR HE to PortalCells orig\n");
         if ((MZdbgVector.x == lineX2 + deltatestX -prevExtendCellX + basePos.x && MZdbgVector.y == lineY2 + deltatestY -prevExtendCellY + basePos.z))
           printf ("Adding DBGVECTOR HE to PortalCells dest\n");
         #endif
          PortalCells[highEdgeDirection].set(Vector2(lineX2 + deltatestX + basePos.x,lineY2 + deltatestY + basePos.z), 
                          Vector3(lineX2 + deltatestX -prevExtendCellX + basePos.x,lineY2 + deltatestY -prevExtendCellY + basePos.z, testval));
          }
       if (testval != MOVEMAP_VALUE_CANT_REACH/* && !myOutOfZonePoints->contains(Vector2(lineX2 + deltatestX -prevExtendCellX,lineY2 + deltatestY -prevExtendCellY))*/)
         {
         #ifdef DBGVECTOR
         if ((MZdbgVector.x == lineX2 + deltatestX -prevExtendCellX + basePos.x && MZdbgVector.y == lineY2 + deltatestY -prevExtendCellY + basePos.z))
           printf ("Adding DBGVECTOR HE to myOutOfZonePoints\n");
         #endif

         myOutOfZonePoints->insert(Vector2(lineX2 + deltatestX -prevExtendCellX,lineY2 + deltatestY -prevExtendCellY));
         }
       }
    
    
    if (broken)
      {
#ifdef DEBUG_EXTEND
printf ("Bloqued, box : %f,%f,%f,%f\n",low.x+basePos.x,low.y+basePos.z,high.x+basePos.x,high.y+basePos.z);
#endif
      for ( unsigned short i=0; i<3; i++)
        {
        Array<Vector2> LCorigs = tempLayerConnexions.getKeys();
        for (int o=0; o<LCorigs.length(); o++)
          {
          Vector3 val = tempLayerConnexions.get(LCorigs[o]);
          val.z=zMoveMapBox->getFloatHeight (val.z); // uncompress height for layers connexion
          LayerConnexions[direction].set(LCorigs[o], val);
          #ifdef LAYER_CONNEXION
          printf("LayerConnexions [%d] : %f,%f\n",direction,LCorigs[o].x,LCorigs[o].y);
          #endif
          }
        }
      Array<Vector2> PCorigs = tempPortalCells.getKeys();
 #ifdef DEBUG_PORTALS
if (PCorigs.length()>0);
    printf("adding %i portal cells\n",PCorigs.length());
#endif
      for (int o=0; o<PCorigs.length(); o++)
        {
        PortalCells[direction].set(PCorigs[o], tempPortalCells.get(PCorigs[o]));
        }
      #ifdef DBGVECTOR
      if (tmpOutOfZonePoints.contains(Vector2(MZdbgVector.x -basePos.x, MZdbgVector.y -basePos.z)))
        printf ("should add DBGVECTOR from tmp to myOutOfZonePoints\n");
      for(Set<Vector2>::Iterator itr= tmpOutOfZonePoints.begin(); itr != tmpOutOfZonePoints.end(); ++itr)
        printf ("myOutOfZonePoints %f,%f\n",(*itr).x +basePos.x, (*itr).y +basePos.z);
      #endif
      for(Set<Vector2>::Iterator itr= tmpOutOfZonePoints.begin(); itr != tmpOutOfZonePoints.end(); ++itr)
        {
        #ifdef DBGVECTOR
        if ((*itr).x == MZdbgVector.x -basePos.x && (*itr).y == MZdbgVector.y -basePos.z)
          printf ("Adding DBGVECTOR from tmp to myOutOfZonePoints\n");
        #endif
        myOutOfZonePoints->insert(*itr);
        }

      }
    else
      {
#ifdef DEBUG_PORTALS
printf("delete portals\n");
#endif

      PortalCells[direction].clear();// delete cells added by orthogonals edges
      TempMovePortals.deleteAll();
      #ifdef DBGVECTOR
      if (tmpOutOfZonePoints.contains(Vector2(MZdbgVector.x -basePos.x, MZdbgVector.y -basePos.z)))
        printf ("NOT Adding DBGVECTOR from tmp to myOutOfZonePoints\n");
      #endif
// FIXME : build error :      tmpOutOfZonePoints.deleteAll();
      low=newlow;
      high=newhigh;
      
      for (int x =lineX1; x<=lineX2; x++)
        {
        for ( int y =lineY1; y<=lineY2; y++)
          {
          //if (!myReachedPos->contains(Vector2(x,y)))
          //  {
#ifdef DEBUG_COORDS
if (!myReachedPos->contains(Vector2(x,y)))
  printf("reached %f,%f (%i,%i)\n",x+diffBaseV.x,y+diffBaseV.z,x,y);
#endif
//printf("myReachedPos at %f,%f\n",x+ basePos.x , y+ basePos.z );
#ifdef DBGVECTOR
if (myOutOfZonePoints->contains(Vector2(x, y)) && MZdbgVector.x == x+ basePos.x && MZdbgVector.y == y+ basePos.z)
  printf("DBGVECTOR: delete myOutOfZonePoints\n");
if (!myReachedPos->contains(Vector2(x,y)) && MZdbgVector.x == x+ basePos.x && MZdbgVector.y == y+ basePos.z)
  printf("adding DBGVECTOR to myReachedPos\n");

#endif

            myReachedPos->insert(Vector2(x,y));
            if (myOutOfZonePoints->contains(Vector2(x, y)))
              {
              myOutOfZonePoints->remove(Vector2(x, y));
#ifdef DEBUG_COORDS
printf("removing myOutOfZonePoints %f,%f : reached\n",x,y);
#endif
              }
            //}
          }
        }
      }

    
    tempPortalCells.clear();
    for ( unsigned short i=0; i<3; i++)
      tempLayerConnexions.clear();
    return !broken;
    }
  
  // to sort Array<MovePortal*>
  static bool MovePortalLT(MovePortal*const& elem1, MovePortal*const& elem2)
    {
    return elem1->getLow2().x < elem2->getLow2().x || elem1->getLow2().y < elem2->getLow2().y;
    }

  void
  MoveZone::save(FILE* fp)
  {
    fwrite (&iIndex,sizeof(unsigned int),1,fp);

    fwrite (&iBounds.low().x,sizeof(float),1,fp);
    fwrite (&iBounds.low().y,sizeof(float),1,fp);
    fwrite (&iBounds.low().z,sizeof(float),1,fp);
    fwrite (&iBounds.high().x,sizeof(float),1,fp);
    fwrite (&iBounds.high().y,sizeof(float),1,fp);
    fwrite (&iBounds.high().z,sizeof(float),1,fp);
    
    size_t nPortal=iPortals.size();
    fwrite (&nPortal,sizeof(size_t),1,fp);
    for (unsigned int i=0; i < nPortal;i++)
      iPortals[i]->save(fp);
  }

  void
  MoveZone::load(FILE* fp)
  {
    fread (&iIndex,sizeof(unsigned int),1,fp);
    float x1,y1,z1,x2,y2,z2;
    fread (&x1,sizeof(float),1,fp);
    fread (&y1,sizeof(float),1,fp);
    fread (&z1,sizeof(float),1,fp);
    fread (&x2,sizeof(float),1,fp);
    fread (&y2,sizeof(float),1,fp);
    fread (&z2,sizeof(float),1,fp);
    iBounds.set(Vector3(x1,y1,z1),Vector3(x2,y2,z2));
    
    size_t nPortal;
    fread (&nPortal,sizeof(size_t),1,fp);
    for (unsigned int i=0; i < nPortal;i++)
      {
      MovePortal* MP= new MovePortal();
      MP->load(fp);

      iPortals.push_back(MP);
      }
  }

  void
  MoveZoneContainer::setZone(AABox* pBox,unsigned int i, Array<MovePortal*> * PArray)
  {
    MoveZone *iTmp = new MoveZone();
    iTmp->setBounds(pBox);
    iTmp->setIndex(i);
    iTmp->setPortalArray(PArray);
    pMoveZonesArray.append(iTmp);
    //pMoveZones->insert(&iTmp);
  }
    
  bool MoveZone::operator== (const MoveZone& pMZ) const
    {
    return iIndex == pMZ.getIndex();
    }
  size_t MoveZone::hashCode ()
    {
      return (getBounds ().hashCode ());
    }

}

