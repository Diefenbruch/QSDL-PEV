/******************************************************************************\
 Datei : PESActivity.cpp
 Inhalt: Deklaration 
 Autor : Christian Rodemeyer, Marc Diefenbruch
 Datum : 20.10.95
 Status: 
\******************************************************************************/   

#include <SCL/SCProcess.h>
#include <SCL/SCProcessType.h>
#include <SCL/SCMachine.h>
#include <SCL/SCSignalType.h>
#include <SCL/SCRequestType.h>

#include "PESActivity.h"

#if _SC_DMALLOC
  #include <dmalloc.h>
#endif

/******************************************************************************\
 PESEvent: Sensor zur Registration einfacher Events   
\******************************************************************************/

PESEvent::PESEvent(const char *        Name, 
                   const PDEventType * EventType,
                   SCDuration          Interval) :
  PESTally   (Interval),
  PESCounter (Interval),
  name       (Name),
  evType     (EventType),
  runnable   (NULL)
{
  Reset();
}

SCBoolean PESEvent::NotifyOnEvent(SCTraceAction Event) const
{
  switch (Event)
  {
    case scTraceProcessCreate:  return evType->isSignal || !evType->isArrival;
    case scTraceMachineCreate:  return !evType->isSignal && evType->isArrival;
    case scTraceSignalReceive:  return evType->isSignal;
    case scTraceServiceRequest: return !evType->isSignal;
      
    default: return false;  
  }
}


void PESEvent::Reset(void)
{
  PESTally::Reset();
  PESCounter::Reset();
  lastEvent = Now();
}


void PESEvent::Report(SCStream& Out) const
{
  Out << "Event '" << name << "':";
  Underline(Out, strlen(name) + 8);  
  PESTally::Report(Out);
  PESCounter::Report(Out);
}


double PESEvent::GetValue(int ValIndex) const
{
  if (ValIndex < PESTally::__T) 
    return PESTally::GetValue(ValIndex);
  else if (ValIndex < PESCounter::__C)
    return PESCounter::GetValue(ValIndex);
  else {
    std::cout << "Illegal ValIndex in PESEvent!\n";
    abort();
  }
}

 
void PESEvent::EvProcessCreate(const SCProcess* Process, const SCProcess*)
{
  if (!runnable && Process->GetType()->GetName() == evType->nameProcMach)
  {
    runnable = Process;
  }
}


void PESEvent::EvMachineCreate(const SCMachine* Machine)
{
  if (!runnable && Machine->GetName() == evType->nameProcMach)
  {
    runnable = Machine;
  }
}

    
void PESEvent::EvSignalReceive(const SCProcess* Process,
                               const SCSignal* Signal)
{
  if ((   (evType->isArrival && (Process == runnable)) 
       || (!evType->isArrival && (Signal->GetSender() == runnable))
      )
      && (Signal->GetSignalType()->GetName() == evType->nameSigReq)) 
  {
    UpdateCounter();
    UpdateTally(Now() - lastEvent);
    lastEvent = Now();
  }
}


void PESEvent::EvServiceRequest(const SCMachine* Machine,
                                const SCRequest* Request)
{
  if ((   (evType->isArrival && (Machine == runnable))
       || (!evType->isArrival && (Request->GetCaller() == runnable))
       )	   
     && (Request->GetRequestType()->GetName() == evType->nameSigReq)) 
  {
    UpdateCounter();
    UpdateTally(Now() - lastEvent);
    lastEvent = Now();
  }
}


/******************************************************************************\
 PESAction: Sensor zur Registration einfacher Events   
\******************************************************************************/  

PESActivity::PESActivity(const char *       Name,
                         const PDEventType* Start,
                         const PDEventType* Stop,
                         SCDuration         Interval) :
  PESTally           (Interval),
  PESCounter         (Interval),
  name               (Name),
  evStart            (Start),
  evStop             (Stop),
  runnableStart      (NULL),
  runnableStop       (NULL),
  activityHasStopped (true)
{
  assert(Start && Stop);
}

SCBoolean PESActivity::NotifyOnEvent(SCTraceAction Event) const
{
  switch (Event)
  {
    case scTraceProcessCreate:
      return (evStart->isSignal || !evStart->isArrival)
          || (evStop->isSignal || !evStop->isArrival);

    case scTraceMachineCreate:
      return (!evStart->isSignal && evStart->isArrival)
          || (!evStop->isSignal && evStart->isArrival);

    case scTraceSignalReceive:
      return (evStart->isSignal || evStop->isSignal);

    case scTraceServiceRequest:
      return (!evStart->isSignal || !evStop->isSignal);

    default:
      return false;
  }
}


void PESActivity::Reset(void)
{
  PESTally::Reset();
  PESCounter::Reset();
  activityHasStopped = true;
}


void PESActivity::Report(SCStream& Out) const
{
  Out << "Activity '" << name << "':";
  Underline(Out, strlen(name) + 11);
  PESTally::Report(Out);
  PESCounter::Report(Out);
}


double PESActivity::GetValue(int ValIndex) const
{
  if (ValIndex < PESTally::__T)
    return PESTally::GetValue(ValIndex);
  else if (ValIndex < PESCounter::__C)
    return PESCounter::GetValue(ValIndex);
  else {
    std::cout << "Illegal ValIndex in PESActivity!\n";
    abort();
  }
}


void PESActivity::EvProcessCreate(const SCProcess* Process,
                                  const SCProcess*)
{
  if ((evStart->isSignal || !evStart->isArrival) && !runnableStart &&
      (Process->GetType()->GetName() == evStart->nameProcMach))
  {
    runnableStart = Process;
  }
  if ((evStop->isSignal || !evStop->isArrival) && !runnableStop &&
      (Process->GetType()->GetName() == evStop->nameProcMach))
  {
    runnableStop = Process;
  }
}


void PESActivity::EvMachineCreate(const SCMachine* Machine)
{
  if ((!evStart->isSignal && evStart->isArrival) && !runnableStart && 
      (Machine->GetName() == evStart->nameProcMach))
  {
    runnableStart = Machine;
  }
  if ((!evStop->isSignal && evStop->isArrival) && !runnableStop && 
      (Machine->GetName() == evStop->nameProcMach))
  {
    runnableStop = Machine;
  }
}

void PESActivity::ActivityStart(void)
{
  activityHasStopped = false;
  activityStart = Now();
  UpdateCounter();
}


void PESActivity::ActivityStop(void)
{
  activityHasStopped = true;
  UpdateTally(Now() - activityStart);
}


void PESActivity::EvServiceRequest(const SCMachine* Machine,
                                   const SCRequest* Request)
{
  if (activityHasStopped)
  { 
    if (   !evStart->isSignal
	&& (   (evStart->isArrival && (Machine == runnableStart)) 
	    || (!evStart->isArrival && (Request->GetCaller() == runnableStart))
	   )	
	&& (Request->GetRequestType()->GetName() == evStart->nameSigReq)
       )
    {
      ActivityStart();
    }
  }
  else
  {
    if (   !evStop->isSignal 
	&& (   (evStop->isArrival && (Machine == runnableStop)) 
	    || (!evStop->isArrival && (Request->GetCaller() == runnableStop))
	   )
	&& (Request->GetRequestType()->GetName() == evStop->nameSigReq)
       )
    {
      ActivityStop();
    }
  }
}


void PESActivity::EvSignalReceive(const SCProcess * Process,
                                  const SCSignal *  Signal)
{
  if (activityHasStopped)
  { 
    if (   evStart->isSignal 
	&& (   (evStart->isArrival && (Process == runnableStart))
            || (!evStart->isArrival && (Signal->GetSender() == runnableStart))
	   )
        && (Signal->GetSignalType()->GetName() == evStart->nameSigReq)
       )
    {
      ActivityStart();
    }
  }
  else
  {
    if (   evStop->isSignal 
	&& (   (evStop->isArrival && (Process == runnableStop))
            || (!evStop->isArrival && (Signal->GetSender() == runnableStop))
	   )
        && (Signal->GetSignalType()->GetName() == evStop->nameSigReq)
       )
    {
      ActivityStop();
    }
  }
}

