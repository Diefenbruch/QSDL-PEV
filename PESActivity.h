/******************************************************************************\
 Datei : PESActivity.h
 Inhalt: Deklaration der Klassen PESEvent und PESActivity
 Autor : Christian Rodemeyer, Marc Diefenbruch
 Datum : 20.10.95
 Status: 
\******************************************************************************/   

#ifndef __PESACTIVITY_H
#define __PESACTIVITY_H

#ifndef __PESENSOR_H
#include "PESensor.h"
#endif


/******************************************************************************\
 PESEvent: Sensor zur Registration einfacher Events   
\******************************************************************************/  

class PESEvent: public PESTally, public PESCounter
{
  public:
    PESEvent(const char *        Name, 
             const PDEventType * EventType,
             SCDuration          Interval);

    SCBoolean NotifyOnEvent(SCTraceAction Event) const;
    void      Reset(void);
    void      Report(SCStream& Out) const;
    double    GetValue(int ValueIndex) const;
    
    void EvProcessCreate(const SCProcess* Process, const SCProcess*);
    void EvSignalReceive(const SCProcess* Process, const SCSignal* Message);
    void EvMachineCreate(const SCMachine* Machine); 
    void EvServiceRequest(const SCMachine* Machine, const SCRequest* Request);

  private:    
    const char *             name;     // Name des Sensors im Report
    const PDEventType* const evType;
    const SCRunnable*        runnable;
    SCTime                   lastEvent;
};


/******************************************************************************\
 PESActivity: Sensor zur Registration einfacher Aktivitäten
\******************************************************************************/  

class PESActivity: public PESTally, public PESCounter
{
  public:
    PESActivity(const char *      Name, 
		const PDEventType* Start, 
		const PDEventType* Stop,
		SCDuration         Interval);
    
    SCBoolean NotifyOnEvent(SCTraceAction Event) const;
    void      Reset(void);   
    void      Report(SCStream& Out) const;    
    double    GetValue(int ValueIndex) const;
    
    void EvProcessCreate(const SCProcess* Process, const SCProcess*);
    void EvSignalReceive(const SCProcess* Process, const SCSignal* Message);
    void EvMachineCreate(const SCMachine* Machine); 
    void EvServiceRequest(const SCMachine* Machine, const SCRequest* Request);
    
  protected:
    virtual void ActivityStart(void);
    virtual void ActivityStop(void);

  private:
    const char *             name;
    const PDEventType* const evStart;
    const PDEventType* const evStop;
    const SCRunnable*        runnableStart;
    const SCRunnable*        runnableStop;
    SCTime                   activityStart;
    SCBoolean                activityHasStopped;
};


#endif
