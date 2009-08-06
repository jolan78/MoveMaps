#include "MoveZone.h"

/* width / height ration < 1.5
 this increase nb of zones from 3000 to 8500 and doesnt give good results in my tests */
//#define PREFER_SQUARES

// max crossable height diff *4, 7=1.75
#define MAX_C_HEIGHT 7

namespace VMAP
{

  MovePortal::MovePortal (Vector2 pCenter,float tHeight,unsigned int pDirection, MoveZone* MZ)
    {
    center=pCenter;
    radius=0;
    destGridX=destGridY=0;
    tempHeight.append(tHeight);
    DestZoneID= (MZ!=NULL? MZ->getIndex() :INT_MAX);
    DestZone=MZ;

    direction=pDirection;
    }

  
  void
  MovePortal::extend()
    {
    radius+=0.5;
    switch (direction)
      {
      case EXTEND_N :
      case EXTEND_S :
        center.x+=0.5;
        break;
      case EXTEND_E :
      case EXTEND_W :
        center.y+=0.5;
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
    float X1=center.x;
    float X2=center.x;
    float Y1=center.y;
    float Y2=center.y;
    
    switch (direction)
      {
      case EXTEND_N :
      case EXTEND_S :
        X1-=radius;
        X2+=radius;
        break;
      case EXTEND_E :
      case EXTEND_W :
        Y1-=radius;
        Y2+=radius;
        break;
      }

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

    
    fwrite (&center.x,sizeof(float),1,fp);
    fwrite (&center.y,sizeof(float),1,fp);
    fwrite (&radius,sizeof(float),1,fp);
    fwrite (&direction,sizeof(unsigned int),1,fp);
    fwrite (&DestZoneID,sizeof(unsigned int),1,fp);
    fwrite (&destGridX,sizeof(unsigned int),1,fp);
    fwrite (&destGridY,sizeof(unsigned int),1,fp);
  }

  void
  MovePortal::load(FILE* fp)
  {
    fread (&DestZoneID,sizeof(unsigned int),1,fp);

    fread (&center.x,sizeof(float),1,fp);
    fread (&center.y,sizeof(float),1,fp);
    fread (&radius,sizeof(float),1,fp);
    fread (&direction,sizeof(unsigned int),1,fp);
    fread (&DestZoneID,sizeof(unsigned int),1,fp);
    fread (&destGridX,sizeof(unsigned int),1,fp);
    fread (&destGridY,sizeof(unsigned int),1,fp);
  }

  void
  MovePortal::reconnect(Array<MoveZone*>& moveZoneArray)
    {
    if (destGridX == 0 && destGridY == 0) // dest inside grid
      DestZone=moveZoneArray[DestZoneID];
    }

  Vector2
  MovePortal::getLow2()
    {
    switch (direction)
      {
      case EXTEND_N :
      case EXTEND_S :
        return Vector2(center.x-radius,center.y);
        break;
      case EXTEND_E :
      case EXTEND_W :
        return Vector2(center.x,center.y-radius);
        break;
      default:
        assert(false);
      }
    }
    
  Vector2
  MovePortal::getHigh2()
    {
    switch (direction)
      {
      case EXTEND_N :
      case EXTEND_S :
        return Vector2(center.x+radius,center.y);
        break;
      case EXTEND_E :
      case EXTEND_W :
        return Vector2(center.x,center.y+radius);
        break;
      default:
        assert(false);
      }
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
 MoveZoneContainerGenerator::generate(const MoveMapBox* iMoveMapBoxArray,int iNMoveMapBoxes,AABox gridBounds)
   {
    MZ_Tree = new RStarTree<MoveZone*>(2, 4);
    moveZoneIndex=0;
    unsigned int sizeX, sizeY;
    
    for (int i = 0; i < iNMoveMapBoxes; ++i)
      {
      const MoveMapBox* moveMapBox = &iMoveMapBoxArray[i];
      
      Set<Vector2> reachedPos;
      
      const Vector3 basePos = moveMapBox->getBasePosition ();

      sizeX = moveMapBox->getSizeX ();
      sizeY = moveMapBox->getSizeY ();

      /*printf("\n#moveMapBox (%i*%i) at %f,%f Bounds : %f,%f,%f %f,%f,%f\n",sizeX,sizeY,basePos.x,basePos.z,
             moveMapBox->getBounds().low().x,moveMapBox->getBounds().low().z,moveMapBox->getBounds().low().y,
             moveMapBox->getBounds().high().x,moveMapBox->getBounds().high().z,moveMapBox->getBounds().high().y);*/
      unsigned int BoxMZCount=0;

      for (int y = 0; y < sizeY; ++y)
        {
          for (int x = 0; x < sizeX; ++x)
            {
              unsigned short val = moveMapBox->get ((float) x, (float) y);
              if (val != MOVEMAP_VALUE_CANT_REACH && !reachedPos.contains(Vector2(x,y)) )
                {
                  // TODO check orthogonal dir too
                  // TODO : check connexion pts !!
                  outOfZonePoints.insert(Vector2(x,y));
                  while (outOfZonePoints.size() >0)
                    {
                    Set<Vector2>::Iterator itr=outOfZonePoints.begin();
                    Vector2 sPos=*itr;
                    outOfZonePoints.remove(sPos);

                    if (!reachedPos.contains(Vector2(sPos.x,sPos.y)) && !( sPos.x + basePos.x < gridBounds.low().x + 0.5 || sPos.y + basePos.z < gridBounds.low().z + 0.5 || sPos.x + basePos.x > gridBounds.high().x - 1.5 ||  sPos.y + basePos.z > gridBounds.high().z - 1.5  /* edge of the grid */ )) // todo : remove : we check at outOfZonePoints.push_back
                      {
                      MoveZoneGenerator* MZ = new MoveZoneGenerator (moveMapBox,moveZoneIndex,&sPos,&outOfZonePoints,&reachedPos,gridBounds);
  
                      AABox test= MZ->getBounds();
                      
                      MZ_Tree->Insert((MoveZone*)MZ);
                      pMoveZonesArray.append((MoveZone*)MZ);
                      BoxMZCount++;
                      moveZoneIndex++;
                      }
                    }
                }
            }
        }
      printf("generated %u zones.\n",BoxMZCount);
    }
  
  // connect portals
  for (unsigned int i=0; i < pMoveZonesArray.size();i++)
    {
    Array<MovePortal*> iPortals;
    ((MoveZoneGenerator*)pMoveZonesArray[i])->setPortalArray(&iPortals);
    
    for (unsigned short j=0; j<4; ++j)
      {
      connectLayerPortals((MoveZoneGenerator*)pMoveZonesArray[i],j);
      connectPortals((MoveZoneGenerator*)pMoveZonesArray[i],j);
      }
    }
  printf("####finished. %u zones\n",moveZoneIndex);

  }

  void
  MoveZoneContainerGenerator::connectLayerPortals(MoveZoneGenerator* iMoveZone,unsigned int direction)
    {
    Table<Vector2, Vector3> * layerPortals=iMoveZone->getLayerConnexions(direction);
    if (layerPortals->size()==0)
      {
      return;
      }
    AABox bounds=iMoveZone->getBounds();
    AABox fromBounds;
    iMoveZone->getBoxBounds(fromBounds);
    
    MoveZoneGenerator* prevMZ=NULL;
    Array<MoveZone*> MZArray;
    MoveZoneGenerator* curMZ=NULL;
    MovePortal* CurrentPortal;
    Array<MovePortal*> iPortals = iMoveZone->getPortalArray();
    Vector3 curpos,prevpos;
    bool previsportal=false;

    
    float x1,x2,y1,y2;
    switch (direction)
      {
      case EXTEND_N :
        x1=bounds.low().x +0.5f;
        x2=bounds.high().x -0.5f;
        y1=y2=bounds.high().y -0.5f;
      break;
      case EXTEND_S :
        x1=bounds.low().x +0.5f;
        x2=bounds.high().x -0.5f;
        y1=y2=bounds.low().y +0.5f;
      break;
      case EXTEND_E :
        x1=x2=bounds.high().x -0.5f;
        y1=bounds.low().y +0.5f;
        y2=bounds.high().y -0.5f;
      break;
      case EXTEND_W :
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
        if (layerPortals->get(Vector2(x,y),curpos))
          {
          lowZ=curpos.z-1.8;
          highZ=curpos.z+1.8;
           
          MZArray=getMoveZonesByZRange(curpos.x,curpos.y,lowZ,highZ);

          if (MZArray.size() == 0)
            {
            previsportal=false;
            }
          
          for (unsigned int i=0; i<MZArray.size(); ++i)
            {
            MoveZoneGenerator* tmpMZ=(MoveZoneGenerator*)MZArray[i];
            float h=tmpMZ->getGlobalHeightAt(curpos.x,curpos.y);
            if (h != FLOAT_HEIGHT_CANT_REACH && abs(h - curpos.z) < 1.8)
              {
              curMZ=tmpMZ;
              break;
              }
            }
          if (curMZ == NULL)
            {
            previsportal=false;
            }
          else
            {
            if (previsportal && curMZ == prevMZ)
              {
              CurrentPortal->extend(bounds.high().z); // TODO : better manage z if needed
              }
            else
              {
              CurrentPortal= new MovePortal(Vector2(x,y),curpos.z, direction ,curMZ);// TODO : better manage z if needed
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
  MoveZoneContainerGenerator::connectPortals(MoveZoneGenerator* iMoveZone,unsigned int direction)
    {
    AABox bounds=iMoveZone->getBounds();
    Table<Vector2, Vector3> * PortalCells=iMoveZone->getPortalCell(direction);
    MoveZoneGenerator* prevMZ=NULL;
    MoveZoneGenerator* curMZ=NULL;
    MovePortal* CurrentPortal;
    Array<MovePortal*> iPortals = iMoveZone->getPortalArray();
    Vector3 curpos,prevpos;
    bool previsportal=false;
    AABox pBoxBounds;

    iMoveZone->getBoxBounds (pBoxBounds);
    
    
    float x1,x2,y1,y2;
    switch (direction)
      {
      case EXTEND_N :
        x1=bounds.low().x +0.5f;// xy bounds are stretch by 0.5 in order to allow faster zone finding
        x2=bounds.high().x -0.5f;
        y1=y2=bounds.high().y -0.5f;
      break;
      case EXTEND_S :
        x1=bounds.low().x +0.5f;
        x2=bounds.high().x -0.5f;
        y1=y2=bounds.low().y +0.5f;
      break;
      case EXTEND_E :
        x1=x2=bounds.high().x -0.5f;
        y1=bounds.low().y +0.5f;
        y2=bounds.high().y -0.5f;
      break;
      case EXTEND_W :
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
        if (PortalCells->get(Vector2(x,y),curpos))
          {
          curMZ=(MoveZoneGenerator*)getMoveZoneByCoords(curpos);
          bool isGridPortal=false;
          if (curMZ == NULL)
            {
            // unfound portals should occur only at grid edge.
            if (curpos.x<0 || curpos.x> pBoxBounds.high().x - pBoxBounds.low().x -1 ||
               curpos.y<0 || curpos.y> pBoxBounds.high().z - pBoxBounds.low().z -1)
              {
              gridPortal gp;
              gp.MoveZoneId=iMoveZone->getIndex();
              gp.fromx=x;
              gp.fromy=y;
              gp.destz=curpos.z;
              
              gridPortals[direction].push_back(gp);
              
              isGridPortal=true;
              }
            else
              printf("Connexion not found in MZ %d PortalCell %f,%f -> %f,%f,%f\n",iMoveZone->getIndex(),x,y,curpos.x,curpos.y,curpos.z);
            
            previsportal=false;
            }
          
          if (previsportal && curMZ == prevMZ)
            {
            CurrentPortal->extend(bounds.high().z); // TODO : better manage z if needed
          
            }
          else
            {
            CurrentPortal= new MovePortal(Vector2(x,y),curpos.z, direction ,curMZ);// TODO : better manage z if needed
            if (isGridPortal)
              CurrentPortal->setGridPortal(INT_MAX,INT_MAX);
            iPortals.append(CurrentPortal);
            }
          prevMZ=curMZ;

          prevpos=curpos;
          if (prevMZ)
            previsportal=true;
          portalID++;
          }
        else
          previsportal=false;
        }
      }
    
    
    iMoveZone->setPortalArray(&iPortals);
    
    }
  
  void
  MoveZoneContainerGenerator::saveGridCnx(const char* filename, const unsigned int direction)
    {
    
    if (gridPortals[direction].size() > 0)
      {
      FILE *GridCnx = fopen (filename, "wb");
      for(Array<gridPortal>::ConstIterator itr= gridPortals[direction].begin();itr != gridPortals[direction].end();++itr)
        {
        fprintf (GridCnx, "%u,%f,%f,%f\n", itr->MoveZoneId, itr->fromx, itr->fromy, itr->destz);
        }
      fclose (GridCnx);
      }
    }
  
  void
  MoveZoneContainer::reconnectPortals()
    {
    for (unsigned int zoneID=0;zoneID<pMoveZonesArray.size();++zoneID)
      pMoveZonesArray[zoneID]->reconnectPortals(pMoveZonesArray);
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
  
  MoveZoneGenerator::MoveZoneGenerator (const MoveMapBox* iMoveMapBox,unsigned int ZoneIndex,Vector2* sPos,Set<Vector2>* outOfZonePoints,Set<Vector2>* reachedPos,AABox iGridBounds)
    {
    iIndex=ZoneIndex;
    myOutOfZonePoints=outOfZonePoints;
    myReachedPos=reachedPos;
    zMoveMapBox=iMoveMapBox;    
    gridBounds=iGridBounds;
    const Vector3 basePos = zMoveMapBox->getBasePosition ();
    
    unsigned int initZ=getLocalCompressedHeightAt(sPos->x, sPos->y);
    
    low=Vector3(sPos->x,sPos->y,initZ);
    high=Vector3(sPos->x,sPos->y,initZ);
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
    
    low.x += basePos.x - 0.5;
    low.y += basePos.z - 0.5;
    low.z = zMoveMapBox->getFloatHeight (low.z) -0.1;
    high.x += basePos.x + 0.5;
    high.y += basePos.z + 0.5;
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
    }
  
  bool
  MoveZoneGenerator::Extend(unsigned int direction,unsigned int extendermask)
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
      Set<Vector2> tmpOutOfZonePoints;
      Table<Vector2, Vector3> tempLayerConnexions;
      bool addPortalToHigh=false;
      bool addPortalToLow=false;
      unsigned int lowEdgeDirection,highEdgeDirection;
      
      AABox pBoxBounds = zMoveMapBox->getBounds ();

      const Vector3 basePos = zMoveMapBox->getBasePosition ();
      
      MovePortal* CurrentPortal = NULL;
      Array<MovePortal*> TempMovePortals;
      unsigned int prevTestHeight=0;
      
      
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
          break;
        }
       
       for (int x =lineX1; x<=lineX2; x++)
         {
         for ( int y =lineY1; y<=lineY2; y++)
           {
           bool isportal=false;
           
           /*
           (x,y) is the 'test' point, out of the current zone, his height is testval
           (x+diffBaseV.x, y+diffBaseV.z) is the 'from' point, inside the zone, leading to the test point, his height is fromval
           coords are relative to the layer base position
           */
           
           unsigned int fromval = getLocalCompressedHeightAt(x+deltatestX, y+deltatestY);
           assert (fromval != MOVEMAP_VALUE_CANT_REACH);
           unsigned int testval;
           if (x<0 || x> pBoxBounds.high().x - pBoxBounds.low().x -1 ||
               y<0 || y> pBoxBounds.high().z - pBoxBounds.low().z -1)
             testval = MOVEMAP_VALUE_CANT_REACH; // the testpoint is out of layer's box
           else
             {
             testval = getLocalCompressedHeightAt(x, y);
             }

           if (newhigh.z < testval)
             newhigh.z = testval;
           if (newlow.z > testval)
             newlow.z = testval;
             
           if (testval == MOVEMAP_VALUE_CANT_REACH ||
               myReachedPos->contains(Vector2(x, y)) ||
               abs( (int)testval - (int)fromval) > MAX_C_HEIGHT /* FIXME: getMaxZDiff ? getZDiff ? VectorMoveMap.h*/ ||
               (x+prevExtendCellX >= lineX1 && y+prevExtendCellY >= lineY1) && abs( (int)testval - (int)prevTestHeight) > MAX_C_HEIGHT /*TODO : check if thats necessary */
               || x + basePos.x < gridBounds.low().x + 0.5 || y + basePos.z < gridBounds.low().z + 0.5 || x + basePos.x > gridBounds.high().x - 1.5 ||  y + basePos.z > gridBounds.high().z - 1.5  /* edge of the grid */ )
             {
             
             myReachedPos->insert(Vector2(x+deltatestX, y+deltatestY));
             if ((testval != MOVEMAP_VALUE_CANT_REACH) && abs( (int)testval - (int)fromval) <= MAX_C_HEIGHT)
               {
               isportal=true;
               }
             else 
               {
               /* if the test point xy is out of layer or it is too far away then it could be a layer-to-layer (or grid) connexion
               we cant rely on from pt z is near the floor/ceil of the layer due to layer box inside layer box cases
               TODO: handle layerbox-inside-zone case : another point can be reachable in the current layer. perhaps remove zones with no portals to it could fix it.
               */
               tempLayerConnexions.set(Vector2(x+ pBoxBounds.low().x + deltatestX, y+ pBoxBounds.low().z + deltatestY),Vector3(x+ pBoxBounds.low().x, y+ pBoxBounds.low().z,fromval));
               }
             
             broken=true;
             }
           else
             {
             isportal=true;
             #ifdef PREFER_SQUARES
             if (addPortalToHigh && addPortalToLow)
               {
               if ((high.x-low.x)>(high.y-low.y))
                 {
                 if((high.x-low.x)/(high.y-low.y) >1.5)
                   broken = true;
                 }
               else if((high.y-low.y)/(high.x-low.x) >1.5)
                 broken = true;
               }
             #endif
             }
           if (isportal)
             {
             tempPortalCells.set(Vector2(x+ basePos.x + deltatestX, y+ basePos.z + deltatestY),
                                         Vector3(x+ basePos.x, y+ basePos.z, testval));
             }
           
           if (testval != MOVEMAP_VALUE_CANT_REACH)
             {
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
       if (testval != MOVEMAP_VALUE_CANT_REACH && abs( (int)testval - (int)fromval) <= MAX_C_HEIGHT)
         {
         PortalCells[lowEdgeDirection].set(Vector2(lineX1 + deltatestX + basePos.x,lineY1 + deltatestY + basePos.z), 
                         Vector3(lineX1 + deltatestX + prevExtendCellX + basePos.x,lineY1 + deltatestY + prevExtendCellY + basePos.z, testval));
         }
       else
         {
         LayerConnexions[lowEdgeDirection].set(Vector2(lineX1 + deltatestX + basePos.x , lineY1 + deltatestY + basePos.z),Vector3(lineX1 + deltatestX + prevExtendCellX + basePos.x,lineY1 + deltatestY + prevExtendCellY + basePos.z, zMoveMapBox->getFloatHeight (fromval)));
         }
       
       if (testval != MOVEMAP_VALUE_CANT_REACH)
         {
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

        if (testval != MOVEMAP_VALUE_CANT_REACH && abs( (int)testval - (int)fromval) <= MAX_C_HEIGHT)
          {
          PortalCells[highEdgeDirection].set(Vector2(lineX2 + deltatestX + basePos.x,lineY2 + deltatestY + basePos.z), 
                          Vector3(lineX2 + deltatestX -prevExtendCellX + basePos.x,lineY2 + deltatestY -prevExtendCellY + basePos.z, testval));
          }
        else
          {
          LayerConnexions[highEdgeDirection].set(Vector2(lineX2 + deltatestX + basePos.x , lineY2 + deltatestY + basePos.z),Vector3(lineX2 + deltatestX -prevExtendCellX + basePos.x,lineY2 + deltatestY -prevExtendCellY + basePos.z, zMoveMapBox->getFloatHeight (fromval)));
          
          }
        if (testval != MOVEMAP_VALUE_CANT_REACH)
          { 
          myOutOfZonePoints->insert(Vector2(lineX2 + deltatestX -prevExtendCellX,lineY2 + deltatestY -prevExtendCellY));
          }
        }
    
    if (broken)
      {
      for ( unsigned short i=0; i<3; i++)
        {
        Array<Vector2> LCorigs = tempLayerConnexions.getKeys();
        for (int o=0; o<LCorigs.length(); o++)
          {
          Vector3 val = tempLayerConnexions.get(LCorigs[o]);
          val.z=zMoveMapBox->getFloatHeight (val.z); // uncompress height for layers connexion
          LayerConnexions[direction].set(LCorigs[o], val);
          }
        }
      Array<Vector2> PCorigs = tempPortalCells.getKeys();
      for (int o=0; o<PCorigs.length(); o++)
        {
        PortalCells[direction].set(PCorigs[o], tempPortalCells.get(PCorigs[o]));
        }
      for(Set<Vector2>::Iterator itr= tmpOutOfZonePoints.begin(); itr != tmpOutOfZonePoints.end(); ++itr)
        {
        myOutOfZonePoints->insert(*itr);
        }

      }
    else
      {
      PortalCells[direction].clear();// delete cells added by orthogonals edges
      TempMovePortals.deleteAll();
// FIXME : build error :      tmpOutOfZonePoints.deleteAll();
      low=newlow;
      high=newhigh;
      
      for (int x =lineX1; x<=lineX2; x++)
        {
        for ( int y =lineY1; y<=lineY2; y++)
          {
          myReachedPos->insert(Vector2(x,y));
          if (myOutOfZonePoints->contains(Vector2(x, y)))
            {
            myOutOfZonePoints->remove(Vector2(x, y));
            }
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
    return elem1->getCenter2().x < elem2->getCenter2().x || elem1->getCenter2().y < elem2->getCenter2().y;
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
  MoveZone::reconnectPortals(Array<MoveZone*>& moveZoneArray)
    {
    for (unsigned int i=0;i<iPortals.size();++i)
      iPortals[i]->reconnect(moveZoneArray);
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

