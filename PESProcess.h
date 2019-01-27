/******************************************************************************\
 Datei : PESProcess.h
 Inhalt: Deklaration von Auswertungssensoren fuer Prozesse:
         PES- Process, ProcessQueue, ProcessQLen, ProcessQLenFrequency
	 SignalWaitTime, SignalFrequency, GlobalSignalFrequency
	  StateFrequency, ProcessCount (ProcessCountDistribution?)
 Autor : Christian Rodemeyer, Marc Diefenbruch
 Datum : 02.10.95
 Status: 
\******************************************************************************/   

#ifndef __PESPROCESS_H
#define __PESPROCESS_H

#ifndef __PESENSOR_H
#include "PESensor.h"
#endif

/******************************************************************************\
 PESProcess: Basisklasse aller Prozessinidividuellen Auswertungen, ueber das
   proc Datenelement kann der Sensor feststellen, ob Ereignisse mit dem zu
   untersuchenden Prozess in Verbindung stehen.
\******************************************************************************/ 

class PESProcess: virtual public PESensor
{
  public:
    PESProcess(const SCProcessType * ProcessType);
    PESProcess(const char * ProcessName);
    ~PESProcess(void);
    
    SCBoolean NotifyOnEvent(SCTraceAction Event) const;
    void EvProcessCreate(const SCProcess * Process, const SCProcess *);
    
  protected:
    const SCProcessType * processType;
    const char *          processName;
};
  
/******************************************************************************\
 PESProcessQueue: Basisklasse aller Auswertungen über die Signal-Warteschlange 
   eines Prozesses.
\******************************************************************************/  

class PESProcessQueue: public PESProcess, protected virtual PESQueue
{
  public:
    PESProcessQueue(const SCProcessType * ProcessType);
    PESProcessQueue(const char * ProcessName);
    
    SCBoolean NotifyOnEvent(SCTraceAction Event) const;
    
    void EvSignalReceive(const SCProcess* Process, const SCSignal*);
    void EvSignalConsume(const SCProcess* Process, const SCSignal*);
    void EvSignalDrop(const SCProcess* Process, const SCSignal*);
};


/******************************************************************************\
 PESProcessQLen:
\******************************************************************************/  

class PESProcessQLen: public PESProcessQueue, public PESQueueLength
{
  public:
    PESProcessQLen(const SCProcessType * ProcessType,
                   SCDuration Interval);
    PESProcessQLen(const char * ProcessName,
                   SCDuration Interval);
    void Report(SCStream& Out) const;
    
  protected:  
    void UpdateQLen(int QLenDiff);
};


/******************************************************************************\
 PESProcessQLenFrequency:
\******************************************************************************/  

class PESProcessQLenFrequency: public PESProcessQueue,
                               public PESQueueLengthFrequency
{
  public:
    PESProcessQLenFrequency(const SCProcessType * ProcessType);
    PESProcessQLenFrequency(const char * ProcessName);

    void Report(SCStream& Out) const;
    
  protected:
    void UpdateQLen(int QLenDiff);
};


/******************************************************************************\
 PESSignalWaitTime: 
\******************************************************************************/  

class PESSignalWaitTime: public PESProcess, public PESTally
{
  public:
    PESSignalWaitTime(const SCProcessType * ProcessType, 
                      SCDuration            Interval,
                      const SCSignalType *  SignalType = NULL);
    PESSignalWaitTime(const char *          ProcessName, 
                      SCDuration            Interval,
                      const char *          SignalName = NULL);
    ~PESSignalWaitTime(void);

    SCBoolean NotifyOnEvent(SCTraceAction Event) const;
    void EvSignalConsume(const SCProcess* Process, const SCSignal* Message);
    void Report(SCStream& Out) const;
    
  private:
    const SCSignalType * signalType;
    const char         * signalName;
};    


/******************************************************************************\
 PESSignalFrequency: Zaehler fuer ankommende Signale
\******************************************************************************/ 

class PESSignalFrequency: public PESProcess, public PESFrequency
{
  public:
    PESSignalFrequency(const SCProcessType * ProcessType, 
                       SCBoolean             SignalIn);
    PESSignalFrequency(const char *          ProcessName, 
                       SCBoolean             SignalIn);
    
    SCBoolean NotifyOnEvent(SCTraceAction Event) const;
    void EvSignalReceive(const SCProcess* Process, const SCSignal* Message);
    void Report(SCStream& Out) const;
    
  private:
    SCBoolean sigIn;
};


/******************************************************************************\
 PESGlobalSignalFrequency: Globaler Zaehler fuer alle Signale im System. Als 
   wichtiger Nebeneffekt werden die Name der im System existierenden Signale 
   gesammelt und als NameTable bereit gestellt. Dieser Sensor ist zwar nicht 
   direkt prozessbezogen, da Signale aber nur zwischen Prozessen ausgetauscht 
   werden, passt er hier besser als in allen anderen Modulen noch schlechter.
\******************************************************************************/ 

class PESGlobalSignalFrequency: public PESFrequency //, public PDNameTable
{
  public:
    PESGlobalSignalFrequency(void);
    
    SCBoolean NotifyOnEvent(SCTraceAction Event) const; 
    void EvSignalReceive(const SCProcess*, const SCSignal* Message);
    void Report(SCStream& Out) const;    
};


/******************************************************************************\
 PESStateFrequency: Ermittelt Haeufigkeit und Namen der Zustaende eines Prozeßes.
\******************************************************************************/ 

class PESStateFrequency: virtual public PESProcess,
                         public PESFrequency
{
  public:
    PESStateFrequency(const SCProcessType * ProcessType);
    PESStateFrequency(const char *          ProcessName);
    ~PESStateFrequency(void);

    SCBoolean NotifyOnEvent(SCTraceAction Event) const;
    void      EvStateChange(const SCProcess* Process,
                            const SCStateType* NewState);
    void Reset(void);
    void Report(SCStream& Out) const;
    
    enum {ganttState = PESTally::__T * PESCounter::__C};
                                      // spezieller Wert der dem Updater
                                      // fuer Gantt-Diagramme uebergeben wird

    double GetValue(int Index) const; 
    const  PDStateTable & GetStateTable (void) const { return stateTable; }
    
  private:
    const SCStateType * currentState;
    SCTime              lastChange;
    PDStateTable        stateTable;
};


/******************************************************************************\
 PESProcessNumber: Statisktik ueber die Instanzen einer Prozessklasse.
\******************************************************************************/ 

class PESProcessNumber: public PESTally
{
  public:
    PESProcessNumber(const SCProcessType * ProcessType,
                     SCDuration Interval);
    PESProcessNumber(const char * ProcessName,
                     SCDuration Interval);
    ~PESProcessNumber(void);
    
    SCBoolean NotifyOnEvent(SCTraceAction Event) const;
    void EvProcessCreate(const SCProcess* Process, const SCProcess*);    
    void EvProcessDelete(const SCProcess* Process);
 
    void Reset(void);
    void Report(SCStream& Out) const;
    
  private:
    const SCProcessType * processType;
    const char *          processName;
    SCTime                lastChange;
    int                   count;
};


#endif
