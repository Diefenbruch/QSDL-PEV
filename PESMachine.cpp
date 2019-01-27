/******************************************************************************\
 Datei : PESMachine.cpp
 Inhalt: Implementierung von Auswertungssensoren für Maschinen
 Autor : Christian Rodemeyer, Marc Diefenbruch
 Datum : 10.10.98
 Status: Umgeschrieben von MD
\******************************************************************************/   

#include <string.h>

#include <SCL/SCMachine.h>
#include <SCL/SCRequest.h>
#include <SCL/SCRequestType.h>
#include <SCL/SCProcess.h>
#include <SCL/SCProcessType.h>

#include "PESMachine.h"

#if _SC_DMALLOC
  #include <dmalloc.h>
#endif

/******************************************************************************\
 PESMachine: Basisklasse aller Auswertungen über Maschinen
\******************************************************************************/  

PESMachine::PESMachine(const SCMachine * Machine) :
  machine (Machine),
  machineName (NULL)
{
}

  
PESMachine::PESMachine(const char * MachineName) :
  machine (NULL)
{
  if (MachineName)
    machineName = strdup(MachineName);
  else
    machineName = NULL;
}


PESMachine::~PESMachine(void)
{
  if (machineName)
  {
    delete machineName;
    machineName = NULL;
  }
}

SCBoolean PESMachine::NotifyOnEvent(SCTraceAction Event) const
{
  return (Event == scTraceMachineCreate);
}

  
void PESMachine::EvMachineCreate(const SCMachine * Machine)
{
  if (!machine && machineName)
  {
    if (!strcmp(Machine->GetName(), machineName))
      machine = Machine;
  }
}


/******************************************************************************\
 PESMachineQueue: Implementierung
\******************************************************************************/

PESMachineQueue::PESMachineQueue(const SCMachine * Machine) :
  PESMachine (Machine)
{
}


PESMachineQueue::PESMachineQueue(const char * MachineName) :
  PESMachine (MachineName)
{
}


SCBoolean PESMachineQueue::NotifyOnEvent(SCTraceAction Event) const
{
  switch (Event)
  {
    case scTraceServiceRequest:   // Request trifft neu ein
    case scTraceServiceStart:     // Request verlaesst Warteraum
    case scTraceServiceInterrupt: // unterbrochener Request kommt
                                  // wieder in den Warteraum
    
      return true;
      
    default: return PESMachine::NotifyOnEvent(Event);  
  }
}


void PESMachineQueue::EvServiceRequest(const SCMachine * Machine,
                                       const SCRequest *)
{
  if (Machine == machine) UpdateQLen(1);
}


void PESMachineQueue::EvServiceStart(const SCMachine * Machine,
                                     const SCRequest *)
{
  if (Machine == machine) UpdateQLen(-1);
}


void PESMachineQueue::EvServiceInterrupt(const SCMachine * Machine,
                                         const SCRequest *)
{
  if (Machine == machine) UpdateQLen(1);
}


/******************************************************************************\
 PESMachineQLen: Implementierung
\******************************************************************************/

PESMachineQLen::PESMachineQLen(const SCMachine * Machine,
                               SCDuration Interval) :
  PESQueueLength  (Interval),
  PESMachineQueue (Machine)
{
}


PESMachineQLen::PESMachineQLen(const char * MachineName,
                               SCDuration Interval) :
  PESQueueLength  (Interval),
  PESMachineQueue (MachineName)
{
}


void PESMachineQLen::UpdateQLen(int QLenDiff) 
{
  PESQueueLength::UpdateQLen(QLenDiff);
}


void PESMachineQLen::Report(SCStream& Out) const
{
  Out << "Queue length of '" << *machine << "':";
  Underline(Out, strlen(machine->GetName()) + 42); 
  PESTally::Report(Out);
}


/******************************************************************************\
 PESMachineQLenFrequency: Verteilung der Warteschlangenlänge
\******************************************************************************/  

PESMachineQLenFrequency::PESMachineQLenFrequency(const SCMachine * Machine) :
  PESMachineQueue(Machine)
{
}    

			       
PESMachineQLenFrequency::PESMachineQLenFrequency(const char * MachineName) :
  PESMachineQueue(MachineName)
{
}    

			       
void PESMachineQLenFrequency::UpdateQLen(int QLenDiff)
{
  PESQueueLengthFrequency::UpdateQLen(QLenDiff);
}


void PESMachineQLenFrequency::Report(SCStream& Out) const 
{
  Out << "Queue length frequency of '" << *machine << "':";
  Underline(Out, strlen(machine->GetName()) + 52);
  PESFrequency::Report(Out);
}


/******************************************************************************\
 PESRequestWaitTime: Wartezeit von Requests in der Warteschlange. 
\******************************************************************************/  

PESRequestWaitTime::PESRequestWaitTime(const SCMachine *     Machine, 
                                       SCDuration            Interval,
                                       const SCRequestType * RequestType) :
  PESMachine (Machine),
  PESTally   (Interval),
  requestType(RequestType),
  requestName(NULL)  
{
}


PESRequestWaitTime::PESRequestWaitTime(const char * MachineName, 
                                       SCDuration   Interval,
                                       const char * RequestName) :
  PESMachine (MachineName),
  PESTally   (Interval),
  requestType(NULL)
{
  if (RequestName)
    requestName = strdup(RequestName);
  else
    requestName = NULL;
}


PESRequestWaitTime::~PESRequestWaitTime(void)
{
  if (requestName)
  {
    delete requestName;
    requestName = NULL;
  }
}


SCBoolean PESRequestWaitTime::NotifyOnEvent(SCTraceAction Event) const
{
  return (Event == scTraceServiceStart) || PESMachine::NotifyOnEvent(Event);
}


void PESRequestWaitTime::EvServiceStart(const SCMachine * Machine,
                                        const SCRequest * Request)
{
  if (!requestType && requestName)
  {
    if (!strcmp(requestName, Request->GetRequestType()->GetName()))
      requestType = Request->GetRequestType();
  }
  
  if (Machine == machine)
  {
    if (requestType == NULL ||
        Request->GetRequestType() == requestType)
    {
      UpdateTally(Now() - Request->GetWaitStartTime());
    }
  }
}


void PESRequestWaitTime::Report(SCStream& Out) const 
{
  int Len = strlen(machine->GetName()) + 38;
  
  Out << "Wait time of ";
  if (requestType != NULL)
  {
    Out << "request '" << *requestType << "'";
    Len += strlen(requestType->GetName()) + 9;
  }
  else
  {
    Out << "all requests";
    Len += 16;
  }
  Out << " at '" << *machine << "':";
  Underline(Out, Len);
  PESTally::Report(Out);
}


/******************************************************************************\
 PESRequestThruTime: 
\******************************************************************************/  

PESRequestThruTime::PESRequestThruTime(const SCMachine *     Machine, 
                                       SCDuration            Interval,
                                       const SCRequestType * RequestType) :
  PESMachine (Machine),
  PESTally   (Interval),
  requestType(RequestType),
  requestName(NULL)
{
}


PESRequestThruTime::PESRequestThruTime(const char * MachineName, 
                                       SCDuration   Interval,
                                       const char * RequestName) :
  PESMachine (MachineName),
  PESTally   (Interval),
  requestType(NULL)
{
  if (RequestName)
    requestName = strdup(RequestName);
  else
    requestName = NULL;
}


PESRequestThruTime::~PESRequestThruTime(void)
{
  if (requestName)
  {
    delete requestName;
    requestName = NULL;
  }
}


SCBoolean PESRequestThruTime::NotifyOnEvent(SCTraceAction Event) const
{
  return (Event == scTraceServiceFinish) || PESMachine::NotifyOnEvent(Event);
}


void PESRequestThruTime::EvServiceFinish(const SCMachine * Machine,
                                         const SCRequest * Request)
{
  if (!requestType && requestName)
  {
    if (!strcmp(requestName, Request->GetRequestType()->GetName()))
      requestType = Request->GetRequestType();
  }
  
  if (Machine == machine)
  {
    if (requestType == NULL ||
        Request->GetRequestType() == requestType)
    {
      UpdateTally(Now() - Request->GetCreationTime());
    }
  }
}


void PESRequestThruTime::Report(SCStream& Out) const 
{
  int Len = strlen(machine->GetName()) + 28;
  
  Out << "Thru time of ";

  if (requestType != NULL)
  {
    Out << "request '" << *requestType << "'";
    Len += strlen(requestType->GetName()) + 9;
  }
  else
  {
    Out << "all requests";
    Len += 11;
  }
  Out << " at '" << *machine << "':";
  Underline(Out, Len);
  PESTally::Report(Out);
}


/******************************************************************************\
 PESRequestFrequency: 
\******************************************************************************/  

PESRequestFrequency::PESRequestFrequency(const     SCMachine *     Machine,
                                         const     SCProcessType * ProcessType,
	                                       SCBoolean RequestIn) :
  PESMachine   (Machine),
  PESProcess   (ProcessType),
  PESFrequency (SC_REQUEST),
  reqIn        (RequestIn)
{
}    


PESRequestFrequency::PESRequestFrequency(const     char * MachineName,
                                         const     char * ProcessName,
	                                       SCBoolean RequestIn) :
  PESMachine   (MachineName),
  PESProcess   (ProcessName),
  PESFrequency (SC_REQUEST),
  reqIn        (RequestIn)
{
}    


SCBoolean PESRequestFrequency::NotifyOnEvent(SCTraceAction Event) const
{
  return (Event == scTraceServiceRequest)
      || (reqIn && PESMachine::NotifyOnEvent(Event)) 
      || (!reqIn && PESProcess::NotifyOnEvent(Event));
}

    
void PESRequestFrequency::EvServiceRequest(const SCMachine * Machine,
                                           const SCRequest * Request)
{
  if ((reqIn && Machine == machine) ||
      (!reqIn && ((SCProcess *)Request->GetCaller())->GetType() == processType))
  {
    UpdateFreq(Request->GetRequestType()->GetID(), 1);
  }
}


void PESRequestFrequency::Report(SCStream& Out) const
{
  Out << "Request frequency ";
  if (reqIn)
  {
    assert(machine);
    Out << "at '" << *machine << "':";
    Underline(Out, strlen(machine->GetName()) + 38);
  }
  else
  {
    assert(processType);
    Out << "from '" << *processType << "':";
    Underline(Out, strlen(processType->GetName()) + 38);
  } 
  PESFrequency::Report(Out);
}


/******************************************************************************\
 PESGlobalRequestFrequency: 
\******************************************************************************/  

PESGlobalRequestFrequency::PESGlobalRequestFrequency() :
  PESFrequency (SC_REQUEST)
{
//  DisableAutoDelete();
}


SCBoolean PESGlobalRequestFrequency::NotifyOnEvent(SCTraceAction Event) const
{
  return (Event == scTraceServiceRequest);
}


void PESGlobalRequestFrequency::EvServiceRequest(const SCMachine *,
                                                 const SCRequest * Request)
{
  SCInteger Key = Request->GetRequestType()->GetID();
  UpdateFreq(Key, 1);
}


void PESGlobalRequestFrequency::Report(SCStream& Out) const
{
  Out << "Global Request Frequency:";
  Underline(Out, 24);
  PESFrequency::Report(Out);
}


/******************************************************************************\
 PESMachineUtilization: 
\******************************************************************************/  

PESMachineUtilization::PESMachineUtilization(const SCMachine * Machine,
                                             SCDuration Interval) :
  PESMachine  (Machine),
  PESTally    (Interval),
  lastChange  (0)
{
}


PESMachineUtilization::PESMachineUtilization(const char * MachineName,
                                             SCDuration Interval) :
  PESMachine  (MachineName),
  PESTally    (Interval),
  lastChange  (0)
{
}


void PESMachineUtilization::Reset(void)
{
  PESTally::Reset();
  lastChange = Now();
}


SCBoolean PESMachineUtilization::NotifyOnEvent(SCTraceAction Event) const
{
  return (Event == scTraceServiceStart) 
      || (Event == scTraceServiceInterrupt)
      || (Event == scTraceServiceFinish)
      || PESMachine::NotifyOnEvent(Event);
}


void PESMachineUtilization::EvServiceStart(const SCMachine * Machine,
                                           const SCRequest *)
{
  if (Machine == machine)
  {
    SCDuration weight = Now() - lastChange;

    SCReal sample = 1.0 - (((SCReal)machine->NumOfFreeServers() + 1)/
                           (SCReal)machine->NumOfServers());
    assert(sample >= 0.0);
    UpdateTally (sample, weight);
    
    lastChange = Now();
  }
}

    
void PESMachineUtilization::EvServiceInterrupt(const SCMachine * Machine,
                                               const SCRequest *)
{
  if (Machine == machine)
  {
    SCDuration weight = Now() - lastChange;

    SCReal sample = 1.0 - (((SCReal)machine->NumOfFreeServers() - 1)/
                           (SCReal)machine->NumOfServers());
    assert(sample >= 0.0);
    UpdateTally (sample, weight);
    
    lastChange = Now();
  }
}


void PESMachineUtilization::EvServiceFinish(const SCMachine * Machine,
                                            const SCRequest *)
{
  if (Machine == machine)
  {
    SCDuration weight = Now() - lastChange;

    SCReal sample = 1.0 - (((SCReal)machine->NumOfFreeServers() - 1)/
                           (SCReal)machine->NumOfServers());
    assert(sample >= 0.0);
    UpdateTally (sample, weight);
    
    lastChange = Now();
  }
}


void PESMachineUtilization::Report(SCStream& Out) const
{
  Out << "Utilization of '" << *machine << "':";
  Underline(Out, 58);
  PESTally::Report(Out);
}

