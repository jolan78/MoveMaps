/* 
 * Copyright (C) 2005-2008 MaNGOS <http://www.mangosproject.org/>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef _COORDMODELMAPPING_H_
#define _COORDMODELMAPPING_H_

#include <G3D/Table.h>
#include <G3D/Array.h>
#include <G3D/Vector3.h>
#include <G3D/Matrix3.h>

/**
This Class is a helper Class to convert the raw vector data into BSP-Trees.
We read the directory file of the raw data output and build logical groups.
Models with a lot of vectors are not merged into a resulting model, but separated into an additional file.
 */

namespace VMAP2
{

#define MIN_VERTICES_FOR_OWN_CONTAINER_FILE 65000

  // if we are in an instance
#define MIN_INST_VERTICES_FOR_OWN_CONTAINER_FILE 40000

  //=====================================================

  class ModelPosition
  {
  private:
    G3D::Matrix3 ixMatrix;
    G3D::Matrix3 iyMatrix;
    G3D::Matrix3 izMatrix;
  public:
    G3D::Vector3 iPos;
    G3D::Vector3 iDir;
    float iScale;

    void
    init ()
    {

      // Swap x and y the raw data uses the axis differently
      ixMatrix = G3D::Matrix3::fromAxisAngle (G3D::Vector3::unitY (), -(G3D::pi () * iDir.x / 180.0));
      iyMatrix = G3D::Matrix3::fromAxisAngle (G3D::Vector3::unitX (), -(G3D::pi () * iDir.y / 180.0));
      izMatrix = G3D::Matrix3::fromAxisAngle (G3D::Vector3::unitZ (), -(G3D::pi () * iDir.z / 180.0));

    }
    G3D::Vector3 transform (const G3D::Vector3& pIn) const;

    void
    moveToBasePos (const G3D::Vector3& pBasePos)
    {
      iPos -= pBasePos;
    }
  };

  class NameCollection
  {
  public:
    G3D::Array<std::string> iMainFiles;
    G3D::Array<std::string> iSingeFiles;

    void
    appendToMain (std::string pStr)
    {
      iMainFiles.append (pStr);
    }

    void
    appendToSingle (std::string pStr)
    {
      iSingeFiles.append (pStr);
    }

    size_t
    size ()
    {
      return (iMainFiles.size () + iSingeFiles.size ());
    }
  };

  //=====================================================

  class CMappingEntry
  {
  private:
    int xPos;
    int yPos;
    unsigned int iMapId;
    G3D::Array<std::string> iFilenames;

  public:

    CMappingEntry () { };

    CMappingEntry (unsigned int pMapId, const int pXPos, const int pYPos)
    {
      iMapId = pMapId;
      xPos = pXPos;
      yPos = pYPos;
    };

    ~CMappingEntry () { };

    void addFilename (char *pName);
    const std::string getKeyString () const;

    inline const G3D::Array<std::string>&
    getFilenames () const
    {
      return (iFilenames);
    }

    static const std::string
    getKeyString (unsigned int pMapId, int pXPos, int pYPos)
    {
      char b[100];
      sprintf (b, "%03u_%d_%d", pMapId, pXPos, pYPos);
      return (std::string (b));
    }

  };

  //=====================================================

  class CoordModelMapping
  {
  private:
    G3D::Table<std::string, CMappingEntry *> iMapObjectFiles;
    G3D::Table<std::string, std::string> iProcesseSingleFiles;
    G3D::Array<unsigned int> iMapIds;
    G3D::Array<unsigned int> iWorldAreaGroups;
    bool (*iFilterMethod)(char *pName);

    inline void
    addCMappingEntry (CMappingEntry* pCMappingEntry)
    {
      iMapObjectFiles.set (pCMappingEntry->getKeyString (), pCMappingEntry);
    }

    inline CMappingEntry*
    getCMappingEntry (const std::string& pKey)
    {
      if (iMapObjectFiles.containsKey (pKey))
        return (iMapObjectFiles.get (pKey));
      else
        return 0;
    }

  public:

    CoordModelMapping ()
    {
      iFilterMethod = NULL;
    }
    virtual ~CoordModelMapping ();

    bool readCoordinateMapping (const std::string& pDirectoryFileName);

    const NameCollection getFilenamesForCoordinate (unsigned int pMapId, int xPos, int yPos);

    static unsigned int
    getMapIdFromFilename (std::string pName)
    {
      size_t spos;

      spos = pName.find_last_of ('/');
      std::string basename = pName.substr (0, spos);
      spos = basename.find_last_of ('/');
      std::string groupname = basename.substr (spos + 1, basename.length ());
      unsigned int mapId = atoi (groupname.c_str ());
      return (mapId);
    }

    const G3D::Array<unsigned int>&
    getMaps () const
    {
      return iMapIds;
    }

    inline bool
    isAlreadyProcessedSingleFile (std::string pName)
    {
      return (iProcesseSingleFiles.containsKey (pName));
    }

    inline void
    addAlreadyProcessedSingleFile (std::string pName)
    {
      iProcesseSingleFiles.set (pName, pName);
    }

    inline void
    addWorldAreaMap (unsigned int pMapId)
    {
      if (!iWorldAreaGroups.contains (pMapId))
        {
          iWorldAreaGroups.append (pMapId);
        }
    }

    inline bool
    isWorldAreaMap (unsigned int pMapId)
    {
      return (iWorldAreaGroups.contains (pMapId));
    }

    void
    setModelNameFilterMethod (bool (*pFilterMethod)(char *pName))
    {
      iFilterMethod = pFilterMethod;
    }

  };
}
#endif                                                      /*_COORDMODELMAPPING_H_*/
