/******************************************************************************\
 Datei : PESMachine.h
 Inhalt: Deklaration von Auswertungssensoren für Maschinen:
         PES- Machine, MachineQueue, MachineQLen, MachineQLenFrequency,
	 (MachineUtilization), RequestWaitTime, RequestFrequency, 
	 GlobalRequestFrequency
 Autor : Christian Rodemeyer, Marc Diefenbruch
 Datum : 03.10.95
 Status: Fertig, aber noch ungetestet
\******************************************************************************/   

#ifndef __PESMACHINE_H
#define __PESMACHINE_H

#ifndef __PESENSOR_H
#include "PESensor.h"
#endif
#ifndef __PESPROCESS_H
#include "PESProcess.h"
#endif

/******************************************************************************\
 PESMachine: Basisklasse aller Auswertungen über Maschinen
\******************************************************************************/  

class PESMachine: virtual public PESensor
{
  public:
    PESMachine(const SCMachine * Machine);
    PESMachine(const char * MachineName);
    ~PESMachine(void);
  
    SCBoolean NotifyOnEvent(SCTraceAction Event) const;
    void EvMachineCreate(const SCMachine * Machine);

  protected:   
    const SCMachine * machine;
    const char *      machineName;
};

/******************************************************************************\
 PESMachineQueue: Basisklasse aller Auswertungen über die Request-Warteschlange 
   einer Maschine.
\******************************************************************************/  

class PESMachineQueue: public PESMachine, protected virtual PESQueue
{
  public:
    PESMachineQueue(const SCMachine * Machine);
    PESMachineQueue(const char * MachineName);
    
    SCBoolean NotifyOnEvent(SCTraceAction Event) const;
    
    void EvServiceRequest(const SCMachine * Machine,
                          const SCRequest* Request); 
    void EvServiceStart(const SCMachine * Machine,
                        const SCRequest* Request);
    void EvServiceInterrupt(const SCMachine * Machine,
                            const SCRequest* Request);
};


/******************************************************************************\
 PESMachineQLen: Auswertungen über die Request-Warteschlangenlänge 
   einer Maschine.
\******************************************************************************/  

class PESMachineQLen: public PESQueueLength, public PESMachineQueue
{
  public:
    PESMachineQLen(const SCMachine * Machine,
                   SCDuration Interval);
    PESMachineQLen(const char * MachineName,
                   SCDuration Interval);

    void Report(SCStream& Out) const;
      
  protected:
    void UpdateQLen(int QLenDiff);
};


/******************************************************************************\
 PESMachineQLenFrequency: Verteilung der Warteschlangenlänge
\******************************************************************************/  

class PESMachineQLenFrequency: public PESQueueLengthFrequency,
                               public PESMachineQueue
{
  public:
    PESMachineQLenFrequency(const SCMachine * Machine); 
    PESMachineQLenFrequency(const char * MachineName); 

    void Report(SCStream& Out) const;
    
  protected:  
    void UpdateQLen(int QLenDiff);
};

/******************************************************************************\
 PESMachineQLenFrequency: Verteilung der Warteschlangenlänge
\******************************************************************************/  

class PESMachineUtilization: public PESMachine, public PESTally
{   
  public:
    PESMachineUtilization(const SCMachine * Machine,
                          SCDuration Interval);
    PESMachineUtilization(const char * MachineName,
                          SCDuration Interval);

    void Reset(void);
    void Report(SCStream& Out) const;
    
    SCBoolean NotifyOnEvent(SCTraceAction Event) const;
    void EvServiceStart(const SCMachine* Machine, const SCRequest*);
    void EvServiceFinish(const SCMachine* Machine, const SCRequest*);
    void EvServiceInterrupt(const SCMachine* Machine, const SCRequest*);
    
  private:  
    SCTime lastChange;  // Zeitpunkt der letzte Aenderung der  
};    

/******************************************************************************\
 PESRequestWaitTime: Wartezeit von Requests in der Warteschlange. Wenn als
   RequestType ein leerer String ("") angegeben wird, werden alle Requesttypen
   zur Auswertung herangezogen, ansonsten werden alle ungleichen Requests
   gefiltert.
   
 Definition Wartezeit: Zeit, die zwischen der Ankunft eines Requests in
   der Warteschlange (durch SCL vergeben) bis zum Beginn der Bearbeitung
   (RequestStart) vergeht.
   
 PROBLEM: Gilt die obige Defintion auch für Preemptive arbeitende Maschinen?  
\******************************************************************************/  

class PESRequestWaitTime: public PESMachine, public PESTally
{
  public:
    PESRequestWaitTime(const SCMachine *     Machine, 
                       SCDuration            Interval, 
                       const SCRequestType * RequestType = NULL);
    PESRequestWaitTime(const char *          MachineName, 
                       SCDuration            Interval, 
                       const char *          RequestName = NULL);
    ~PESRequestWaitTime(void);
    
    SCBoolean NotifyOnEvent(SCTraceAction Event) const;
    void EvServiceStart(const SCMachine* Machine,
                        const SCRequest* Request);
    void Report(SCStream& Out) const;    
    
  private:
    const SCRequestType * requestType;
    const char *          requestName;
};    


/******************************************************************************\
 PESRequestThruTime: Durchlaufzeit, mit WaitTime abstimmen
\******************************************************************************/  

class PESRequestThruTime: public PESMachine, public PESTally
{
  public:
    PESRequestThruTime(const SCMachine *     Machine,
                       SCDuration            Interval,
                       const SCRequestType * RequestType = NULL);
    PESRequestThruTime(const char * MachineName,
                       SCDuration   Interval,
                       const char * RequestName = NULL);
    ~PESRequestThruTime(void);
    
    void Report(SCStream& Out) const;
    SCBoolean NotifyOnEvent(SCTraceAction Event) const;
    void EvServiceFinish(const SCMachine* Machine,
                         const SCRequest* Request);
    
  private:
    const SCRequestType * requestType;
    const char *          requestName;
};

/******************************************************************************\
 PESRequestFrequency: 
\******************************************************************************/  

class PESRequestFrequency: public PESMachine, public PESProcess,
                           public PESFrequency 
{
  public:
    PESRequestFrequency(const SCMachine *     Machine,
                        const SCProcessType * ProcessType,
                        SCBoolean             RequestIn);
    PESRequestFrequency(const char *          MachineName,
                        const char *          ProcessName,
                        SCBoolean             RequestIn);

    void Report(SCStream& Out) const;
    SCBoolean NotifyOnEvent(SCTraceAction Event) const; 
    void EvServiceRequest(const SCMachine * Machine,
                          const SCRequest* Request);
    
  private:
    SCBoolean reqIn;
};


/******************************************************************************\
 PESGlobalRequestFrequency: 
\******************************************************************************/  

class PESGlobalRequestFrequency: public PESFrequency //, public PDNameTable 
{
  public:
    PESGlobalRequestFrequency(void);
    
    SCBoolean NotifyOnEvent(SCTraceAction Event) const; 
    void EvServiceRequest(const SCMachine*,
                          const SCRequest* Request);  
    void Report(SCStream& Out) const;    
};

#endif
