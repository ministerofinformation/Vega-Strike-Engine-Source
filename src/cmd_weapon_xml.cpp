#include "cmd_beam.h"
#include "cmd_weapon_xml.h"
#include "xml_support.h"
#include "physics.h"
#include <vector>

#include <expat.h>


using std::vector;
using XMLSupport::EnumMap;
using XMLSupport::Attribute;
using XMLSupport::AttributeList;

namespace BeamXML {
  enum Names {
    UNKNOWN,
    BEAM,
    BALL,
    BOLT,
    MISSILE,
    APPEARANCE,
    //    MANEUVER,
    ENERGY,
    DAMAGE,
    DISTANCE,
    //attributes
    NAME,
    XFILE,
    RED,
    GREEN,
    BLUE,
    ALPHA,
    SPEED,
    PULSESPEED,
    RADIALSPEED,
    RANGE,
    RADIUS,
    RATE,
    STABILITY,
    LONGRANGE,
    CONSUMPTION,
    REFIRE,
    LENGTH//,
    //YAW,
    //PITCH,
    //ROLL
  };
  const EnumMap::Pair element_names[] = {
    EnumMap::Pair ("UNKNOWN",UNKNOWN),
    EnumMap::Pair ("Beam",BEAM),
    EnumMap::Pair ("Ball",BALL),
    EnumMap::Pair ("Bolt",BOLT),
    EnumMap::Pair ("Missile", MISSILE),
    EnumMap::Pair ("Appearance", APPEARANCE),
    //    EnumMap::Pair ("Maneuver",MANEUVER),
    EnumMap::Pair ("Energy",ENERGY),
    EnumMap::Pair ("Damage",DAMAGE),
    EnumMap::Pair ("Distance",DISTANCE)
  };
  const EnumMap::Pair attribute_names [] = {
    EnumMap::Pair ("UNKNOWN",UNKNOWN),
    EnumMap::Pair ("Name",NAME),
    EnumMap::Pair ("file",XFILE),
    EnumMap::Pair ("r",RED),
    EnumMap::Pair ("g",GREEN),
    EnumMap::Pair ("b",BLUE),
    EnumMap::Pair ("a",ALPHA),
    EnumMap::Pair ("Speed",SPEED),
    EnumMap::Pair ("Pulsespeed",PULSESPEED),
    EnumMap::Pair ("Radialspeed",RADIALSPEED),
    EnumMap::Pair ("Range",RANGE),
    EnumMap::Pair ("Radius",RADIUS),
    EnumMap::Pair ("Rate",RATE),
    EnumMap::Pair ("Damage",DAMAGE),
    EnumMap::Pair ("Stability",STABILITY),
    EnumMap::Pair ("Longrange",LONGRANGE),
    EnumMap::Pair ("Consumption",CONSUMPTION),
    EnumMap::Pair ("Refire",REFIRE),
    EnumMap::Pair ("Length", LENGTH)//,
    //EnumMap::Pair ("Yaw",YAW),
    // EnumMap::Pair ("Pitch",PITCH),
    // EnumMap::Pair ("Roll",ROLL)
  };
  const EnumMap element_map(element_names, 9);
  const EnumMap attribute_map(attribute_names, 18);
  Hashtable <string, weapon_info,char[257]> lookuptable;
  string curname;
  weapon_info tmpweapon;
  int level=0;
  void beginElement (void *userData, const XML_Char *name, const XML_Char **atts) {
    AttributeList attributes (atts);
    Names elem = (Names) element_map.lookup(string (name));
    AttributeList::const_iterator iter;
    switch (elem) {
    case UNKNOWN:
      break;
    case BOLT:
    case BEAM:
    case BALL:
    case MISSILE:
      assert (level==0);
      level++;
      tmpweapon.type=elem;
      for (iter= attributes.begin(); iter!=attributes.end();iter++) {
	switch (attribute_map.lookup ((*iter).name)) {
	case UNKNOWN:
	  fprintf (stderr,"Unknown Weapon Element %s",(*iter).name.c_str());
	  break;
	case NAME:
	  curname = (*iter).value;
	  break;
	default:
	  assert (0);
	  break;
	}
      }
      break;
    case APPEARANCE:
      assert (level==1);
      level++;
      for (iter= attributes.begin(); iter!=attributes.end();iter++) {
	switch (attribute_map.lookup ((*iter).name)) {
	case UNKNOWN:
	  fprintf (stderr,"Unknown Weapon Element %s",(*iter).name.c_str());
	  break;
	case XFILE:
	   tmpweapon.file = (*iter).value;
	  break;
	case RED:
	  tmpweapon.r = XMLSupport::parse_float ((*iter).value);
	  break;
	case GREEN:
	  tmpweapon.g = XMLSupport::parse_float ((*iter).value);
	  break;
	case BLUE:
	  tmpweapon.b = XMLSupport::parse_float ((*iter).value);
	  break;
	case ALPHA:
	  tmpweapon.a = XMLSupport::parse_float ((*iter).value);
	  break;
 	default:
	  assert (0);
	  break;
	}
      }      
      break;
    case ENERGY:
      assert (level==1);
      level++;
      for (iter= attributes.begin(); iter!=attributes.end();iter++) {
	switch (attribute_map.lookup ((*iter).name)) {
	case UNKNOWN:
	  fprintf (stderr,"Unknown Weapon Element %s",(*iter).name.c_str());
	  break;
	case CONSUMPTION:
	  tmpweapon.EnergyConsumption = XMLSupport::parse_float ((*iter).value);
	  break;
	case RATE:
	  tmpweapon.EnergyRate = XMLSupport::parse_float ((*iter).value);
	  break;
	case STABILITY:
	  tmpweapon.Stability = XMLSupport::parse_float ((*iter).value);
	  break;
	case REFIRE:
	  tmpweapon.Refire = XMLSupport::parse_float ((*iter).value);
	  break;
	default:
	  assert (0);
	  break;
	}
      }      
      break;
    case DAMAGE:
      assert (level==1);
      level++;
      for (iter= attributes.begin(); iter!=attributes.end();iter++) {
	switch (attribute_map.lookup ((*iter).name)) {
	case UNKNOWN:
	  fprintf (stderr,"Unknown Weapon Element %s",(*iter).name.c_str());
	  break;
	case DAMAGE:
	  tmpweapon.Damage = XMLSupport::parse_float((*iter).value);
	  break;
	case RATE:
	  tmpweapon.Damage = XMLSupport::parse_float ((*iter).value);
	  break;
	case LONGRANGE:
	  tmpweapon.Longrange = XMLSupport::parse_float ((*iter).value);
	  break;
	default:
	  assert (0);
	  break;
	}
      }
      break;
    case DISTANCE:
      assert (level==1);
      level++;
      for (iter= attributes.begin(); iter!=attributes.end();iter++) {
	switch (attribute_map.lookup ((*iter).name)) {
	case UNKNOWN:
	  fprintf (stderr,"Unknown Weapon Element %s",(*iter).name.c_str());
	  break;
	case SPEED:
	  tmpweapon.Speed = XMLSupport::parse_float ((*iter).value);
	  break;
	case PULSESPEED:
	  tmpweapon.PulseSpeed = XMLSupport::parse_float ((*iter).value);
	  break;
	case RADIALSPEED:
	  tmpweapon.RadialSpeed = XMLSupport::parse_float ((*iter).value);
	  break;
	case RANGE:
	  tmpweapon.Range= XMLSupport::parse_float ((*iter).value);
	  break;
	case RADIUS:
	  tmpweapon.Radius = XMLSupport::parse_float ((*iter).value);
	  break;
	case LENGTH:
	  tmpweapon.Length = XMLSupport::parse_float ((*iter).value);
	  break;
	default:
	  assert (0);
	  break;
	}
      }
      break;
    default:
      assert (0);
      break;
    }
  }

  void endElement (void *userData, const XML_Char *name) {
    Names elem = (Names)element_map.lookup(name);
    switch (elem) {
    case UNKNOWN:
      break;
    case BEAM:
    case BOLT:
    case BALL:
    case MISSILE:
      assert (level==1);
      level--;
      lookuptable.Put (curname,new weapon_info (tmpweapon));
      tmpweapon.init();
      break;
    case ENERGY:
    case DAMAGE:
    case DISTANCE:
    case APPEARANCE:
      assert (level==2);
      level--;
      break;
    default:
      break;
    }
  }

}

using namespace BeamXML;

weapon_info* getTemplate(const string &key) {
  return lookuptable.Get(key);
}

void LoadWeapons(const char *filename) {
  const int chunk_size = 16384;
  FILE * inFile= fopen (filename,"r+b");
  if (!inFile) {
    assert (0);
    return;
  }
  XML_Parser parser = XML_ParserCreate (NULL);
  XML_SetElementHandler (parser, &beginElement, &endElement);
 do {
    char *buf = (XML_Char*)XML_GetBuffer(parser, chunk_size);
    int length;
    
    length = fread (buf,1, chunk_size,inFile);
    //length = inFile.gcount();
    XML_ParseBuffer(parser, length, feof(inFile));
  } while(!feof(inFile));
 fclose (inFile);
}
