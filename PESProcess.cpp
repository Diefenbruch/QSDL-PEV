/******************************************************************************\
 Datei : PESProcess.cpp
 Inhalt: Deklaration von Auswertungssensoren für Prozeße
         
 Autor : Christian Rodemeyer, Marc Diefenbruch
 Datum : 10.10.98
 Status: Umgeschrieben von MD
\******************************************************************************/   

#include <string.h>

#include <SCL/SCProcess.h>
#include <SCL/SCProcessType.h>
#include <SCL/SCSignalType.h>

#include "PESProcess.h"

#if _SC_DMALLOC
  #include <dmalloc.h>
#endif

/******************************************************************************\
 PESProcess: 
\******************************************************************************/ 

PESProcess::PESProcess(const SCProcessType * ProcessType) :
  processType (ProcessType),
  processName (NULL)
{
}    


PESProcess::PESProcess(const char * ProcessName) :
  processType (NULL)
{
  if (ProcessName)
    processName = strdup(ProcessName);
  else
    processName = NULL;
}    


PESProcess::~PESProcess(void)
{
  if (processName)
  {
    delete processName;
    processName = NULL;
  }
}


SCBoolean PESProcess::NotifyOnEvent(SCTraceAction Event) const
{
  return (Event == scTraceProcessCreate);
}
    
void PESProcess::EvProcessCreate(const SCProcess* Process, const SCProcess*)
{
  if (!processType && processName)
  {
    if (!strcmp(Process->GetType()->GetName(), processName))
      processType = Process->GetType();
  }  
}


/******************************************************************************\
 PESProcessQueue: 
\******************************************************************************/  

PESProcessQueue::PESProcessQueue(const SCProcessType * ProcessType) :
  PESProcess (ProcessType)
{
}    

    
PESProcessQueue::PESProcessQueue(const char * ProcessName) :
  PESProcess (ProcessName)
{
}    


SCBoolean PESProcessQueue::NotifyOnEvent(SCTraceAction Event) const
{
  return (Event == scTraceSignalReceive) ||
         (Event == scTraceSignalConsume) ||
         (Event == scTraceSignalDrop) ||
         PESProcess::NotifyOnEvent(Event);
}

 
void PESProcessQueue::EvSignalReceive(const SCProcess* Process, const SCSignal*)
{
  if (Process->GetType() == processType) UpdateQLen(1);
}

 
void PESProcessQueue::EvSignalConsume(const SCProcess* Process, const SCSignal*)
{
  if (Process->GetType() == processType) UpdateQLen(-1);
}


void PESProcessQueue::EvSignalDrop(const SCProcess* Process, const SCSignal*)
{
  if (Process->GetType() == processType) UpdateQLen(-1);
}


/******************************************************************************\
 PESProcessQLen:
\******************************************************************************/  

PESProcessQLen::PESProcessQLen(const SCProcessType * ProcessType,
                               SCDuration Interval) :
  PESProcessQueue (ProcessType),
  PESQueueLength  (Interval)
{
}


PESProcessQLen::PESProcessQLen(const char * ProcessName,
                               SCDuration Interval) :
  PESProcessQueue (ProcessName),
  PESQueueLength  (Interval)
{
}


void PESProcessQLen::UpdateQLen(int QLenDiff)
{
  PESQueueLength::UpdateQLen(QLenDiff);
}


void PESProcessQLen::Report(SCStream& Out) const
{
  Out << "Queue length of '" << *processType << "':";
  Underline(Out, strlen(processType->GetName()) + 52); 
  PESTally::Report(Out);
}


/******************************************************************************\
 PESProcessQLenFrequency:
\******************************************************************************/  

PESProcessQLenFrequency::PESProcessQLenFrequency(const SCProcessType * ProcessType) :
  PESProcessQueue (ProcessType)
{
}


PESProcessQLenFrequency::PESProcessQLenFrequency(const char * ProcessName) :
  PESProcessQueue (ProcessName)
{
}


void PESProcessQLenFrequency::UpdateQLen(int QLenDiff)
{
  PESQueueLengthFrequency::UpdateQLen(QLenDiff);
}


void PESProcessQLenFrequency::Report(SCStream& Out) const
{
  Out << "Queue length frequency of '" << *processType << "':";
  Underline(Out, strlen(processType->GetName()) + 36); 
  PESFrequency::Report(Out);
}


/******************************************************************************\
 PESSignalWaitTime: 
\******************************************************************************/  

PESSignalWaitTime::PESSignalWaitTime(const SCProcessType * ProcessType, 
                                     SCDuration            Interval,
                                     const SCSignalType *  SignalType) :
  PESProcess (ProcessType),
  PESTally   (Interval),
  signalType (SignalType),
  signalName (NULL)
{
}

PESSignalWaitTime::PESSignalWaitTime(const char * ProcessName, 
                                     SCDuration   Interval,
                                     const char * SignalName) :
  PESProcess (ProcessName),
  PESTally   (Interval),
  signalType (NULL)
{
  if (SignalName)
    signalName = strdup(SignalName);
  else
    signalName = NULL;
}


PESSignalWaitTime::~PESSignalWaitTime(void)
{
  if (signalName)
  {
    delete signalName;
    signalName = NULL;
  }
}


SCBoolean PESSignalWaitTime::NotifyOnEvent(SCTraceAction Event) const
{
  return (Event == scTraceSignalConsume) || PESProcess::NotifyOnEvent(Event);
}


void PESSignalWaitTime::EvSignalConsume(const SCProcess* Process,
                                        const SCSignal * Signal)
{
  if (!signalType && signalName)
  {
    if (!strcmp(signalName, Signal->GetSignalType()->GetName()))
      signalType = Signal->GetSignalType();
  }
  
  if (Process->GetType() == processType)
  {
    if (signalType == NULL ||
        signalType == Signal->GetSignalType())
    {
      UpdateTally(Now() - Signal->GetCreationTime());
    }
  }
}


void PESSignalWaitTime::Report(SCStream& Out) const
{
  int Len = strlen(processType->GetName()) + 42;
  
  Out << "Wait time of ";
  if (signalType != NULL)
  {
    Out << "signal '" << *signalType << "'";
    Len += strlen(signalType->GetName()) + 8;
  }
  else
  {
    Out << "all signals";
    Len += 10;
  }
  Out << " at '" << *processType << "':";
  Underline(Out, Len);
  PESTally::Report(Out);
}


/******************************************************************************\
 PESSignalFrequency:
\******************************************************************************/ 

PESSignalFrequency::PESSignalFrequency(const SCProcessType * ProcessType, 
                                       SCBoolean             SignalIn) :
  PESProcess   (ProcessType),
  PESFrequency (SC_SIGNAL),
  sigIn        (SignalIn)
{
}


PESSignalFrequency::PESSignalFrequency(const char *          ProcessName, 
                                       SCBoolean             SignalIn) :
  PESProcess   (ProcessName),
  PESFrequency (SC_SIGNAL),
  sigIn        (SignalIn)
{
}


SCBoolean PESSignalFrequency::NotifyOnEvent(SCTraceAction Event) const
{
  return (Event == scTraceSignalReceive) || PESProcess::NotifyOnEvent(Event);
}


void PESSignalFrequency::EvSignalReceive(const SCProcess* Process,
                                         const SCSignal* Signal)
{
  if ((sigIn && Process->GetType() == processType) ||
      (!sigIn && Signal->GetSenderType() == processType))
  {
    UpdateFreq(Signal->GetSignalType()->GetID(), 1);
  }
}


void PESSignalFrequency::Report(SCStream& Out) const
{
  Out << "Signal frequency " 
      << (sigIn ? "at '" : "from '")
      << *processType << "':";   
  Underline(Out, strlen(processType->GetName()) + 38);
  PESFrequency::Report(Out);
}


/******************************************************************************\
 PESGlobalSignalFrequency: Implementierung  
\******************************************************************************/

PESGlobalSignalFrequency::PESGlobalSignalFrequency(void) :
  PESFrequency (SC_SIGNAL)
{
//  DisableAutoDelete();
}


SCBoolean PESGlobalSignalFrequency::NotifyOnEvent(SCTraceAction Event) const
{
  return (Event == scTraceSignalReceive);
}


void PESGlobalSignalFrequency::EvSignalReceive(const SCProcess *,
                                               const SCSignal * Signal)
{
  UpdateFreq(Signal->GetSignalType()->GetID(), 1);
}


void PESGlobalSignalFrequency::Report(SCStream& Out) const
{
  Out << "Global Signal Frequency:";
  Underline(Out, 23);
  PESFrequency::Report(Out);
}


/******************************************************************************\
 PESStateFrequency:
\******************************************************************************/ 

PESStateFrequency::PESStateFrequency(const SCProcessType * ProcessType) :
  PESProcess   (ProcessType),
  PESFrequency (SC_STATE),
  currentState (NULL)
{
}


PESStateFrequency::PESStateFrequency(const char * ProcessName) :
  PESProcess   (ProcessName),
  PESFrequency (SC_STATE),
  currentState (NULL)
{
}


PESStateFrequency::~PESStateFrequency(void)
{
}


SCBoolean PESStateFrequency::NotifyOnEvent(SCTraceAction Event) const
{
  return (Event == scTraceStateChange) || PESProcess::NotifyOnEvent(Event);
}


double PESStateFrequency::GetValue(int Index) const
{
  if (Index == ganttState)
  {
    if (currentState == NULL)
      return -1;
    else
      return (currentState->GetID() - stateTable.GetMinStateID() + 1);
  }
  else
    return PESFrequency::GetValue(Index);
}


void PESStateFrequency::Reset()
{
  PESFrequency::Reset();
  currentState = NULL;
  stateTable.Reset();
}


void PESStateFrequency::Report(SCStream& Out) const
{
  Out << "State frequency of '" << *processType << "':";
  Underline(Out, strlen(processType->GetName()) + 35);
  PESFrequency::Report(Out);
}


void PESStateFrequency::EvStateChange(const SCProcess* Process,
                                      const SCStateType* NewState)
{
  if (Process->GetType() == processType)
  {
    if (currentState == NULL)
    { // Erstes Ereignis
      lastChange = Now();
      currentState = NewState;
    }
    else
    {
      UpdateFreq(currentState->GetID(), Now() - lastChange);
      lastChange = Now();
      currentState = NewState;
    }
    stateTable.RegisterState(currentState);
  }
}

/******************************************************************************\
 PESProcessNumber:
\******************************************************************************/ 

PESProcessNumber::PESProcessNumber(const SCProcessType * ProcessType,
                                   SCDuration Intervall) :
  PESTally   (Intervall),
  processType(ProcessType),
  processName(NULL),
  lastChange (0),
  count      (0)  
{
  Reset();
}
    
PESProcessNumber::PESProcessNumber(const char * ProcessName,
                                   SCDuration Intervall) :
  PESTally   (Intervall),
  processType(NULL),
  lastChange (0),
  count      (0)  
{
  if (ProcessName)
    processName = strdup(ProcessName);
  else
    processName = NULL;

  Reset();
}

PESProcessNumber::~PESProcessNumber(void)
{
  if (processName)
  {
    delete processName;
    processName = NULL;
  }
}


void PESProcessNumber::Reset()
{
  PESTally::Reset();
  if (Now() > 0) lastChange = Now();
}


SCBoolean PESProcessNumber::NotifyOnEvent(SCTraceAction Event) const
{
  return (Event == scTraceProcessCreate || Event == scTraceProcessStop);
}

    
void PESProcessNumber::EvProcessCreate(const SCProcess* Process,
                                       const SCProcess*)
{
  if (!processType && processName)
  {
    if (!strcmp(processName, Process->GetType()->GetName()))
      processType = Process->GetType();
  }
  
  if (Process->GetType() == processType)
  {
    UpdateTally(count, Now() - lastChange);
    count++;
    lastChange = Now();
  }  
}

void PESProcessNumber::EvProcessDelete(const SCProcess* Process)
{
  if (Process->GetType() == processType)
  {
    UpdateTally(count, Now() - lastChange);
    count--;
    lastChange = Now();
  }
}

void PESProcessNumber::Report(SCStream& Out) const
{
  Out << "Number of '" << *processType << "' instances:";
  Underline(Out, strlen(processType->GetName()) + 30);
  PESTally::Report(Out);
}
