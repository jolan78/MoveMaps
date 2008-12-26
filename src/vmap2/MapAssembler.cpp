
#include "G3D/BinaryInput.h"


#include "VGrid.h"
#include "MapAssembler.h"
#include "G3D/BinaryInput.h"

#include <string>
#include <stdio.h>

using namespace G3D;

namespace VMAP2
{
  // we define it here
  std::string VMAPDirPrefix;

  std::string
  getModNameFromModPosName (const std::string& pModPosName)
  {
    size_t spos = pModPosName.find_first_of ('#');
    std::string modelFileName = pModPosName.substr (0, spos);
    return (modelFileName);
  }

  void
  getModelPosition (std::string& pPosString, ModelPosition& pModelPosition)
  {
    float vposarray[3];
    float vdirarray[3];
    float scale;

    size_t spos = pPosString.find_first_of ('#');
    std::string stripedPosString = pPosString.substr (spos + 1, pPosString.length ());

    sscanf (stripedPosString.c_str (), "%f,%f,%f_%f,%f,%f_%f",
            &vposarray[0], &vposarray[1], &vposarray[2],
            &vdirarray[0], &vdirarray[1], &vdirarray[2],
            &scale);

    pModelPosition.iPos = Vector3 (vposarray[0], vposarray[1], vposarray[2]);
    pModelPosition.iDir = Vector3 (vdirarray[0], vdirarray[1], vdirarray[2]);
    pModelPosition.iScale = scale;

  }

  
  
  
  
  
  
  
  
  
  
  
  
  

  MapAssembler::MapAssembler (const std::string& pSrcDirName, const std::string& pDestDirName)
  {
    iSrcDirName = pSrcDirName;
    iDestDirName = pDestDirName;
    VMAPDirPrefix = iDestDirName;
    VMAPDirPrefix += "/";
  }

  int
  MapAssembler::ConvertWorld ()
  {
    // read the dir file in src dir
    std::string fname = iSrcDirName;
    fname.append ("/dir");
    //    iCoordModelMapping.setModelNameFilterMethod (iFilterMethod);
    iCoordModelMapping.readCoordinateMapping (fname);

    Array<unsigned int> mapIds = iCoordModelMapping.getMaps ();

    // nothing to convert
    if (mapIds.size () == 0)
      return 1;

    int mapId = 0;
    int x = 28, y = 28;

    VMap m (mapId);

    ProcessGrid (mapId, x, y, m);

    m.commit ();



    //    for (int i = 0; i < mapIds.size () && result; ++i)
    //      {
    //        unsigned int mapId = mapIds[i];
    //
    //        for (int x = 0; x < 66 && result; ++x)
    //          for (int y = 0; y < 66 && result; ++y)
    //            {
    //
    //            }
    //      }

    return 0;
  }

  bool
  MapAssembler::ProcessGrid (int mapId, int x, int y, VMap& map)
  {
    // get the names of the models in the grid/map
    NameCollection nameCollection = iCoordModelMapping.getFilenamesForCoordinate (mapId, x, y);

    // no work here
    if (nameCollection.size () == 0)
      return true;

    // this are actually the grids, as we know them ;)
    if (nameCollection.iMainFiles.size () > 0)
      {
        std::string gridname = G3D::format ("%04u_%i_%i.vmap", mapId, x, y);
        
        // Check, this file is processed, for such grids its inpossible to happen ... 
        // but it was in old code so i add it here ;)
        if (!iCoordModelMapping.isAlreadyProcessedSingleFile (gridname))
          {
            iCoordModelMapping.addAlreadyProcessedSingleFile (gridname);

            VGrid* v = createVGrid (nameCollection.iMainFiles, gridname.c_str ());
            if (v)
              {
                map.addGrid (v);
                v->unload (); // save some memory
              }
          }
      }


    //TODO CONTINUE !!! SINGLE TARGET FILES ,BOUNDS , SAVE

    return true;
  }

  // here we create one .vmap file
  VGrid*
  MapAssembler::createVGrid (const G3D::Array<std::string>& pPosFileNames, const char* pDestFileName)
  {
    // No models here
    if (pPosFileNames.size () == 0)
      return NULL;

    VGrid* vg = new VGrid (pDestFileName);

    for (int pos = 0; pos < pPosFileNames.size (); ++pos)
      {
        std::string modelPosString = pPosFileNames[pos];
        std::string modelFileName = getModNameFromModPosName (modelPosString);

        ModelPosition modelPosition;
        
        getModelPosition (modelPosString, modelPosition);
        
        // all should be relative to object base position
        modelPosition.moveToBasePos (Vector3(0,0,0));

        modelPosition.init ();

        createModelsFromFile (modelFileName, modelPosition, *vg);
        
        // printf ("modelFileName = %s pDestFileName = %s \n", modelFileName.c_str (), pDestFileName);
      }

    if (vg->size () > 0)
      {
        vg->finish ();
        vg->save ();
        return vg;
      }
    else
      {
        delete vg;
        return NULL;
      }
        
    // NOT REACHED
    return vg;
  }

  void
  MapAssembler::createModelsFromFile (std::string& pModelFilename, ModelPosition& pModelPosition, VGrid& grid)
  {
    std::string filename = iSrcDirName + "/" + pModelFilename;

    BinaryInput rf (filename.c_str (), G3D_LITTLE_ENDIAN);

    char ident[8];

    rf.readBytes (ident, 8);

    if (strcmp (ident, "VMAP002") == 0)
      {
        // for VMAP002 need to read one int
        rf.readInt32 ();
      }
    else if (strcmp (ident, "VMAP001") != 0)
      {
        // if its not VMAP002 or VMAP001 format we cant handle it
        printf ("Wrong version of file %s \n", filename.c_str ());
        abort ();
      }

    // how many groups of models we have in the file
    uint32 groups = rf.readUInt32 ();

    for (uint32 g = 0; g < groups; g++)
      {
        // 3d data for the model
        Array<int32> tempIndexArray;
        Array<Vector3> tempVertexArray;

        uint32 flags = rf.readUInt32 ();

        char blockId[5] = { 0,0,0,0,0 };
        // READ GRP
        rf.readBytes (blockId, 4);

        if (strcmp (blockId, "GRP ") != 0)
          {
            printf ("File format doesnt match GRP %s g = %d \n", filename.c_str (), g);
            abort ();
          }

        int32 blocksize = rf.readInt32 ();

        uint32 branches = rf.readUInt32 ();
        // branches * uint32 ,indexes for each branch (not used yet)
        rf.skip (4 * branches);

        // READ INDEXES
        blockId[4] = 0;
        rf.readBytes (blockId, 4);

        if (strcmp (blockId, "INDX") != 0)
          {
            printf ("File format doesnt match INDX %s g = %d \n", filename.c_str (), g);
            abort ();
          }

        blocksize = rf.readInt32 ();
        uint32 nindexes = rf.readUInt32 ();

        if (nindexes > 0)
          {
            G3D::Array<uint16> indexarray;
            rf.readUInt16 (indexarray, nindexes);

            for (int i = 0; i < (int) nindexes; i++)
              tempIndexArray.append (indexarray[i]);
          }

        // READ VECTORS
        rf.readBytes (blockId, 4);

        if (strcmp (blockId, "VERT") != 0)
          {
            printf ("File format doesnt match VERT %s g = %d \n", filename.c_str (), g);
            abort ();
          }

        blocksize = rf.readInt32 ();
        uint32 nvectors = rf.readUInt32 ();

        if (nvectors > 0)
          {
            // do it explicitly ;)
            // note that it could be just one 4x4 matrix
            for (uint32 i = 0; i < nvectors; i++)
              {
                Vector3 v;
                v.z = rf.readFloat32 ();
                v.y = rf.readFloat32 ();
                v.x = rf.readFloat32 ();

                // Rotate the vertex around its base position
                v = pModelPosition.transform (v);

                const float swapy = v.y;
                v.y = v.x;
                v.x = swapy;

                // transform it to world space
                tempVertexArray.append (v + pModelPosition.iPos);
              }
          }

        // READ LIQUID
        if (flags & 1)
          {
            // we have liquit -> not handled yet ... skip
            rf.readBytes (blockId, 4);

            if (strcmp (blockId, "LIQU") != 0)
              {
                printf ("File format doesnt match LIQU %s g = %d \n", filename.c_str (), g);
                abort ();
              }

            blocksize = rf.readInt32 ();

            rf.skip (blocksize);
          }

        // CALCULATE Model
        const uint32 rest = nindexes % 3;
        if (rest != 0)
          nindexes -= rest;

        if (nindexes >= 3)
          // create a model from the index and vertex arrays ,and add it to the VGrid
          grid.addModel (new Model (tempVertexArray, tempIndexArray));
      }
    return;
  }
  

} // VMAP2



