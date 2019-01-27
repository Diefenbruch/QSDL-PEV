/******************************************************************************\
 Datei : PCController.h
 Inhalt: Interaktive Steuerung und ‹berwachung eins QSDL-Simulators.
         Deklaration der Klassen PCConntroller
 Autor : Christian Rodemeyer, Marc Diefenbruch
 Datum : 26.08.95
 Status: 
\******************************************************************************/   

#ifndef __PCCONTROLLER_H
#define __PCCONTROLLER_H

#ifndef __PESENSOR_H
#include "PESensor.h"
#endif
#ifndef __PVDISPLAY_H
#include "PVDisplay.h"
#endif

/******************************************************************************\
 PCController:
\******************************************************************************/   

class PEEventDispatcher;
class PVXEventDispatcher;

class PCController: public PESensor, public PVDisplay
{
  public:
    PCController(PEEventDispatcher*  Parent, 
                 PVXEventDispatcher& XParent,
                 const char *        SysName);
    ~PCController(void);
    
    void VisualUpdate(void);
    
    // Redefinierte PESensor Funktionen
    // --------------------------------
    SCBoolean NotifyOnEvent(SCTraceAction Event) const;
    
    void EvProcessCreate(const SCProcess*, const SCProcess*); 
    void EvProcessDelete(const SCProcess*);
    void EvMachineCreate(const SCMachine*); 
    void EvMachineDelete(const SCMachine*);
    void EvSignalConsume(const SCProcess*, const SCSignal*); 
    void EvSignalReceive(const SCProcess*, const SCSignal*);
    void EvSignalDrop(const SCProcess*, const SCSignal*);
    void EvServiceRequest(const SCMachine*, const SCRequest*);
    void EvServiceFinish(const SCMachine*, const SCRequest*);
     
    void   Reset(void)             {}          // Der Controller braucht 
    double GetValue(int) const     {return 0;} // diese Sensor- 
    void   Report(SCStream&) const {}          // Funktionalitaet nicht!
    
    // Redefinierte PVDisplay Funktionen
    // ---------------------------------
    void Default(XEvent& ev);
    void Paint();
    void Update();
     
    long GetXEventMask() const;
    void GetXSizeHints(XSizeHints& Hints) const;
    
  private:  
    PVXEventDispatcher& xEventDispatcher; // Link zur XEvent-Verwaltung
    PEEventDispatcher&  controlled;       // Link zum konrollierten Parent
    
    // Beschreibung der drei Buttons (Stop/Start, SlowMotion, Quit)
    // Fuer nur drei Buttons ist es akzeptabel, die Daten unstrukturiert
    // zu verwalten, d.h. keine Button-Klasse einzufuehren. Bei einem
    // komplexeren User-Interface ist es in der Tat guenstiger die Motif-
    // Bibliotheken einzusetzen (siehe Diplomarbeit). Dies kann aber
    // nur Teil einer spaeteren Arbeit sein.
    // -----------------------------------------------------------------
    
    // Anklickbare Buttons
    enum Button{bNone = -1, bStop, bSync, bSnap, bReset, bQuit, bNum}; 
    
    int    bWidth, bHeight;  // Hoehe und Breite fuer alle gleich
    XPoint bPos[bNum];       // Position Left und Top
    const char * bCaption[bNum];   //
    Button pressed;          // Gerade gedruecktes UI-Objekt (-1 fuer bNone)
        
    SCBoolean stopped;       // Simulation wurde angehalten
    SCBoolean asyncUpdate;   // Update laeuft asynchron zur Simulation 
   
    int    numProcesses;     // Anzahl aktiver Prozeﬂe
    int    numSignals;       // insgesamt wartende Signale
    int    numMachines;      // Anzahl existierender Maschinen
    int    numRequests;      // insgesamt wartende Requests
      
    void DrawButton(const XPoint& Pos, const char * Caption,
                    SCBoolean Lowered);
    void ClearButton(const XPoint& Button);
    SCBoolean PosInButton(int PosX, int PosY,
                          const XPoint& Button);
    void DrawSimTime();
    void DrawCounters();
};

#endif
