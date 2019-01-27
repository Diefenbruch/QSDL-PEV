/******************************************************************************\
 Datei : PVXEventDispatcher.h
 Inhalt: Implementierung eins XEventDispatcher für die Visualisierungsklassen 
 Autor : Christian Rodemeyer, Marc Diefenbruch
 Datum : 20.08.95
 Status: Prototyping
\******************************************************************************/   

#ifndef __PVXEVENTDISPATCHER_H
#define __PVXEVENTDISPATCHER_H

#ifndef __PVDisplay
#include "PVDisplay.h"
#endif

#include <SCL/SCList.h>

class PVXEventDispatcher
{
  public:
    PVXEventDispatcher(void);
    ~PVXEventDispatcher(void);
    
    void AddDisplay(PVDisplay* ToAdd);
    void RemoveDisplay(PVDisplay* ToRemove);
    void ArrangeDisplays(void); // ordnet Displays und xmapped sie
    void UpdateDisplays(void);  // aktualisiert Anzeige (keine Expose-Ereignisse!)
    void DoEvents(void);
    void WaitForEvent(void);
    
    Display* GetXDisplay(void) const   {return xDpy;}
    
  private:
    Display* xDpy;       // Verbindung zum Server
    
    typedef SCListIter<PVDisplay> DispIter;
      
    SCList<PVDisplay> displayList;  
};

#endif
