#ifndef _CMD_INPUT_DFA_H_
#define _CMD_INPUT_DFA_H_
#include "in_kb.h"
#include "in_mouse.h"
#include "gfx_camera.h"
//#include "gfx_click_list.h"
class StarSystem;
class UnitCollection;
class ClickList;
class InputDFA {
  enum State {NONE, UNITS_SELECTED, LOCATION_SELECT, TARGET_SELECT};
  //wish the mouse-bound functions could be member functions
  static void ClickSelect (KBSTATE, int,int,int,int,int);
  static void TargetSelect (KBSTATE,int,int,int,int,int);
  static void NoneSelect (KBSTATE,int,int,int,int,int);
  void SetStateDragSelect();
  void SetStateSomeSelected();
  void SetStateNone();
  //bindorders:: UNITS_SELCTED  ->  TARGET_SELECT or LocationSelect
  void SetState(State st);
  State state;
  int curorder;
  StarSystem *parentSystem;
  ClickList *clickList;
  UnitCollection * selected;
  UnitCollection * targetted;
  void ContextSwitch();
  bool Selecting;//is the selection box being drawn
  int prevx;
  int prevy; 
  //  enum State startOver ();
  //  enum State someSelected();
  //  enum State dragSelect();
  UnitCollection *getCollection();
  void replaceCollection (UnitCollection * newcol);
  void appendCollection (UnitCollection *newcol);
public:
  InputDFA (StarSystem * par);
  ~InputDFA();
  void Draw();
  void ContextAcquire();
};
#endif
