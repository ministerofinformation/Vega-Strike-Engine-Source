#include "cmd_input_dfa.h"
#include "gfx_click_list.h"

//needed as functions bound to keys may not be class member functions--Context switch handles it
InputDFA *CurDFA=NULL;

#define ACTIVE_SHIFT               1
#define ACTIVE_CTRL                2
#define ACTIVE_ALT                 4

//Equiv of nonselect, but for Targets instead of selected ships
void InputDFA::TargetSelect (KBSTATE k,int x,int y, int delx, int dely, int mod) {
  if (k==RESET)
    return;///little hack to prevent the function from being 'primed' with reset and continuing on an infinite loop again and again and again
  CurDFA->state=TARGET_SELECT;//to fool the Noneselect function into using targets
  //don't bind keys above...."quiet state update"
  if (mod&ACTIVE_CTRL) {
    mod-=ACTIVE_CTRL;
  }
  NoneSelect (k,x,y,delx,dely,mod);
  if (CurDFA->state==TARGET_SELECT) {
    //executeOrders from selected->target;
    if (k==RELEASE&&CurDFA->targetted!=NULL) {
      UnitCollection::UnitIterator * tmp = CurDFA->targetted->createIterator();
      Unit * un;
      while (un = tmp->current()) {
	fprintf (stderr,"Execute orders on target! %x\n",un);
	tmp->advance();
      }
      delete tmp;
      delete CurDFA->targetted;
      CurDFA->targetted=NULL;
      
    }
    CurDFA->SetStateNone(); //go back up the heirarchy;
  }
}

void InputDFA::ClickSelect (KBSTATE k, int x, int y, int delx, int dely, int mod) {
  static int kmod;
  if (k==RESET)
    return;///little hack to prevent the function from being 'primed' with reset and continuing on an infinite loop again and again and again

  if (k==PRESS) {

    CurDFA->Selecting=false;
    kmod = mod;
    CurDFA->prevx=x;
    CurDFA->prevy=y;
  }
  if (kmod&ACTIVE_CTRL/*||condition of bound orders matchedFIXME*/) {
    TargetSelect(k,x,y,delx,dely,kmod);//add some provision for binding keys to orders
    return;
  }
  if (k==PRESS){
    Unit * sel = CurDFA->clickList->requestShip(x,y);
    if (sel!=NULL) {
      UnitCollection *tmpcollection=new UnitCollection;
      tmpcollection->append(sel);
      if (mod&ACTIVE_SHIFT) {      
	CurDFA->appendCollection(tmpcollection);
	fprintf (stderr,"Select:appendingselected\n");
      } else {
	CurDFA->replaceCollection (tmpcollection);
	fprintf (stderr,"Select:replacingselected\n");
      }
      CurDFA->SetStateSomeSelected();
    }else {
      if (!(mod&ACTIVE_SHIFT)){
	CurDFA->SetStateNone();
	fprintf (stderr,"Select:missedReplaceselected\n");
      }
    }
  }
  if (k==DOWN) {
    if (delx||dely) {
      CurDFA->Selecting=true;
      UnitCollection * tmpcol = CurDFA->clickList->requestIterator(CurDFA->prevx,CurDFA->prevy,x,y);
      if (mod&ACTIVE_SHIFT) {
	//do clickb0x0rz on both CurDFA->selection && tmpcol FIXME
	delete tmpcol;

	/*CurDFA->appendCollection(tmpcol)*/;
      }else{
	//do clickb0x0rz on tmpcolFIXME
	//	CurDFA->replaceCollection(tmpcol);//so you can see click boxes around stuff
      }
    }
  }
  if (k==RELEASE&&CurDFA->Selecting) {
    CurDFA->Selecting=false;
    UnitCollection *tmpcol = CurDFA->clickList->requestIterator(CurDFA->prevx,CurDFA->prevy,x,y);
    if (!(kmod&ACTIVE_SHIFT)){
      CurDFA->replaceCollection(tmpcol);
      UnitCollection::UnitIterator * tmp2 = tmpcol->createIterator();
      if (!tmp2->current()) {
	CurDFA->SetStateNone();
	fprintf (stderr,"SelectBoxMissed\n");
      } else 
	fprintf (stderr,"SelectBoxReplace\n");//      SetStateSomeSelected(); already there
      delete tmp2;
    }else {
      fprintf (stderr,"Select:SelectBoxAppending\n");
      CurDFA->appendCollection(tmpcol);
    }
  }
}


//this function is bound in the NONE state...
void InputDFA::NoneSelect (KBSTATE k,int x, int y, int delx, int dely, int mod) {
  static int kmod;
  if (k==RESET)
    return;///little hack to prevent the function from being 'primed' with reset and continuing on an infinite loop again and again and again
  if (mod&ACTIVE_CTRL)
    return; //you don't want control pressed
  if (k==PRESS) {
    CurDFA->Selecting=false;
    kmod = mod;
    CurDFA->prevx=x;
    CurDFA->prevy=y;
    Unit * sel = CurDFA->clickList->requestShip(x,y);
    if (sel!=NULL) {
      UnitCollection *tmpcollection=new UnitCollection;
      tmpcollection->append(sel);
      fprintf (stderr,"None::replacing Single Unit");if (CurDFA->state==TARGET_SELECT) fprintf (stderr," to target\n");else fprintf (stderr," to select\n");
      
      CurDFA->replaceCollection (tmpcollection);
      CurDFA->SetStateSomeSelected();
    }else {
      fprintf (stderr,"None::missed\n");if (CurDFA->state==TARGET_SELECT) fprintf (stderr," to target");else fprintf (stderr," to select");
    }
  }
  if (k==DOWN) {
    if (delx||dely) {
      CurDFA->Selecting=true;
      UnitCollection * tmpcol = CurDFA->clickList->requestIterator(CurDFA->prevx,CurDFA->prevy,x,y);
      //FIXMEdo select boxes
      delete tmpcol;
    }
  }
  if (k==RELEASE&&CurDFA->Selecting) {
    CurDFA->Selecting=false;
    UnitCollection *tmpcol = CurDFA->clickList->requestIterator(CurDFA->prevx,CurDFA->prevy,x,y);
    CurDFA->replaceCollection(tmpcol);
    UnitCollection::UnitIterator * tmp2 = tmpcol->createIterator();
    if (tmp2->current()) {
      fprintf (stderr,"None::replacing SelectBox Units");if (CurDFA->state==TARGET_SELECT) fprintf (stderr," to target");else fprintf (stderr," to select");
      CurDFA->SetStateSomeSelected();
    }else {
      fprintf (stderr,"None::select box missed");
    }
    delete tmp2;
  }
}

InputDFA::InputDFA (StarSystem * par) {
  parentSystem= par;
  clickList = parentSystem->getClickList();
  state = NONE;
  ContextAcquire();//binds keys, etc
  curorder=0;
  Selecting=false;
  selected = NULL;
  targetted = NULL;
}

InputDFA::~InputDFA() {
  delete clickList;
}
/**
enum State InputDFA::startOver() {
  switch (state) {
  case NONE:
  case UNITS_SELECTED:
    return NONE;
  case LOCATION_SELECT:
  case TARGET_SELECT:
    return UNITS_SELECTED;
  }
  return NONE;
}

enum State InputDFA::someSelected() {
  switch (state) {
  case NONE:
  case UNITS_SELECTED:
    return UNITS_SELECTED;
  case LOCATION_SELECT:
    return NONE;//invalid
  case TARGET_SELECT:
    return TARGET_SELECT;
  }
  return UNITS_SELECTED;
}
*/
UnitCollection * InputDFA::getCollection () {
  switch (state) {
  case LOCATION_SELECT:
  case UNITS_SELECTED:
  case NONE:
    return selected;
  case TARGET_SELECT:
    return targetted;
  }
  return selected;
}
void InputDFA::replaceCollection (UnitCollection *newcol) {
  switch (state) {
  case LOCATION_SELECT:
  case UNITS_SELECTED:
  case NONE:
    if (selected)
      delete selected;
    selected = newcol;
    break;
  case TARGET_SELECT:
    if (targetted)
      delete targetted;
    targetted = newcol;
    break;
  }
}

void InputDFA::appendCollection (UnitCollection *newcol) {
  switch (state) {
  case LOCATION_SELECT:
  case UNITS_SELECTED:
  case NONE:
    if (selected) {
      UnitCollection::UnitIterator *tmpit =newcol->createIterator();
      selected->append (tmpit);
      delete tmpit;
      delete newcol;
      //remove duplicates FIXME
    } else {
      selected=newcol;
    }
    break;
  case TARGET_SELECT:
    if (targetted) {
      UnitCollection::UnitIterator * tmpit = newcol->createIterator();
      targetted->append (tmpit);
      delete tmpit;
      delete newcol;
      //remove duplicates FIXME
    }else {
      targetted = newcol;
    }
    break;	
  }

}
void InputDFA::SetStateNone() {
  switch (state) {
  case NONE:
  case UNITS_SELECTED:
    SetState (NONE);
    break;
  case LOCATION_SELECT:
  case TARGET_SELECT:
    SetState (UNITS_SELECTED);
    break;

  }
}

/*
void InputDFA::SetStateDragSelect() {
  switch (state) {
  case NONE:
  case UNITS_SELECTED:
    SetState (CLICK_DRAG);
    break;
  case LOCATION_SELECT:
  case TARGET_SELECT:
    SetState (TARGET_DRAG);
    break;
  }
}
*/
void InputDFA::SetStateSomeSelected() {
  switch (state){
  case NONE:
  case UNITS_SELECTED:
    SetState (UNITS_SELECTED);
    break;
  case LOCATION_SELECT:
  case TARGET_SELECT:
    SetState (TARGET_SELECT);
    break;
  }
}

void InputDFA::SetState (State st) {
  state = st;
  ContextAcquire();
}

void InputDFA::Draw () {
  switch (state) {
  case NONE:	//draw arrow
  break;
  case UNITS_SELECTED: //draw arrow, boxes
    break;
  case LOCATION_SELECT:
    break;
  case TARGET_SELECT:
    break;
  }
}

void InputDFA::ContextAcquire() {
  if (CurDFA!=NULL){
    CurDFA->ContextSwitch();
  }
  CurDFA=this;
  switch (state) {
  case NONE:	BindKey (0,NoneSelect);
  break;
  case UNITS_SELECTED:BindKey (0,ClickSelect);
    break;
  case LOCATION_SELECT://BindKey (0,ClickSelect);
    break;
  case TARGET_SELECT:BindKey (0,ClickSelect);
    break;
  }
}

/** ContextSwitch unbinds the respective keys and appropriately to the given state*/
void InputDFA::ContextSwitch (){
  switch (state) {
  case NONE:UnbindMouse(0);
      break;
  case UNITS_SELECTED:UnbindMouse(0);
    break;
  case LOCATION_SELECT://UnbindMouse(0);
    break;
  case TARGET_SELECT:UnbindMouse(0);
    break;
  }
}
