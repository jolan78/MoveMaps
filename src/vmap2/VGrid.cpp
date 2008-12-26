
#include "VGrid.h"
#include <string>

using namespace G3D;

namespace VMAP2
{
  extern std::string VMAPDirPrefix;
  
  // Feel free to increase when versions change
  const char VGridMagic[] = { 'V','M','A','P','0','0','3','\0'} ;

  VGrid::VGrid (class G3D::BinaryInput& b) :
  iLoaded (false)
  {
    deserialize (b);
  }

  VGrid::~VGrid ()
  {
    unload ();
  }

  void
  VGrid::serialize (class G3D::BinaryOutput& b) const
  {
    iBounds.serialize (b);
    b.writeUInt32 (iFileName.length () + 1);
    b.writeString (iFileName);
  }

  void
  VGrid::deserialize (class G3D::BinaryInput& b)
  {
    iBounds.deserialize (b);
    int32 size = b.readUInt32 ();
    iFileName = b.readString (size);
  }

  bool VGrid::operator== (const VGrid& other) const
  {
    return this == &other;
  }

  size_t
  VGrid::hashCode () const
  {
    return iBounds.hashCode ();
  }

  void
  VGrid::getBounds (G3D::AABox& b) const
  {
    b = iBounds;
  }

  void
  VGrid::unload ()
  {
    // explictly unload
    if (iTree.size () > 0)
      {
        for (ModelTree::Iterator iter = iTree.begin (); iter != iTree.end (); ++iter)
          delete iter->value;

        iTree.clear ();
      }

    iLoaded = false;
  }

  bool
  VGrid::load ()
  {
    if (iLoaded)
      return true;

    std::string temp_name = VMAPDirPrefix + iFileName;

    //    if(!fileExists(temp_name.c_str ()))
    //      return false;
    
    // better abort ,because if filename doesnot exist ,
    // then vmaps dir is not valid and doesnt contain valid data

    BinaryInput b (temp_name.c_str (), G3D_LITTLE_ENDIAN, true);

    // check the magic header
    char magic[sizeof (VGridMagic)];

    b.readBytes (magic, sizeof (VGridMagic));

    if (::memcmp (magic, VGridMagic, sizeof (magic)) != 0)
      abort ();

    int32 size = b.readInt32 ();

    printf("position %d \n",b.getPosition ());
    
    iTree.deserializeStructure (b);
    
    printf("position %d \n",b.getPosition ());

    for (int32 i = 0; i < size; i++)
      {
//        printf("position %d \n",b.getPosition ());
        iTree.insert (ModelPtr (new Model (b)));
      }
      

    iLoaded = true;

    return true;
  }

  void
  VGrid::save () const
  {
    debugAssert (iLoaded);

    std::string temp_name = VMAPDirPrefix + iFileName;

    BinaryOutput b (temp_name, G3D_LITTLE_ENDIAN);
    
    b.writeBytes (VGridMagic,sizeof(VGridMagic));
    
    b.writeInt32 (iTree.size ());

    iTree.serializeStructure (b);

    for (ModelTree::Iterator it = iTree.begin (); it != iTree.end (); ++it)
      {
        it->value->serialize (b);
      }
      
    b.compress ();
    b.commit ();
  }

  VGrid::VGrid (const char* pDestFileName) :
  iFileName (pDestFileName),iLoaded(false) { }
  
  void 
  VGrid::addModel (Model* new_model)
  {
    iLoaded = true;
    iTree.insert (ModelPtr (new_model));
  }
  
  void VGrid::finish ()
  {
    //this is bug ,shouldnt create VGrid if it doesnt have Models in it
    if (iTree.size () == 0)
      abort (); 
    
    // BALANCE the tree
    iTree.balance (0, 0);

    // CALC bounds
    iBounds.set (Vector3::inf (),Vector3::zero () - Vector3::inf ());

    debugAssert (iTree.size () > 0);

    for (ModelTree::Iterator iter = iTree.begin (); iter != iTree.end (); ++iter)
      {
        AABox box;
        iter->value->getBounds (box);
        iBounds.set (iBounds.low ().min (box.low ()), iBounds.high ().max (box.high ()));
      }

    debugAssert (iBounds.low ().x < iBounds.high ().x);
    debugAssert (iBounds.low ().y < iBounds.high ().y);
    debugAssert (iBounds.low ().z < iBounds.high ().z);
  }
  
}

void
getBounds (const VMAP2::VGridPtr& m, G3D::AABox& b)
{
  m.value->getBounds (b);
}
