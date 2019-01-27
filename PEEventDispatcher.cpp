/******************************************************************************\
 Datei : PEEventDispatcher.cpp
 Inhalt: Implementierung des EventDispatcher (PEEventDispatcher) 
 Autor : Christian Rodemeyer, Marc Diefenbruch
 Datum : 10.10.98
 Status: Synchrone und asynchrone Updates
         Ergaenzungen und Interface-Anpassungen (MD)
\******************************************************************************/   

#include <iostream>
#include <time.h>
#include <sys/time.h>

#include "PCController.h"
#include "PEEventDispatcher.h"

#if _SC_DMALLOC
  #include <dmalloc.h>
#endif

/******************************************************************************\
 PEEventDispatcher  
\******************************************************************************/   

PEEventDispatcher::PEEventDispatcher(const char * Configuration,
                                     const char * Specification,
                                     double PicsPerSec) :
  SCTrace         (scfTraceAll),
  specification   (Specification),
  updateInterval  ((clock_t)(CLOCKS_PER_SEC / PicsPerSec)), 
  lastUpdateClock (clock()),
  sleepUSecs      ((long)(1000000 / (PicsPerSec + 1))), // Sleep beim synchronen Update 
  asyncUpdate     (false),
  reportInterval  (0.0),
  lastReport      (0)
{
  SCNatural i;

  assert(updateInterval > 0); 

  std::cout << "\n"
       << "PEV for QSDL - Performance Evaluation and Visualization\n"
       << "-------------------------------------------------------\n"
       << "(c) 1995-98 by Christian Rodemeyer and Marc Diefenbruch\n\n";
  std::cout << "Setting up experiment \"" << Configuration << "\" ...";
  std::cout.flush();

  Setup(Configuration, Specification);

  for (i = 0; i < scTraceMax; i++)
  {
    activateOnAction[i].SetDeleteElems(false);
  }

  std::cout << "Done.\n\n";
  std::cout.flush();
}


PEEventDispatcher::~PEEventDispatcher(void)
{
  registeredSensors.RemoveAllElements();
  registeredUpdaters.RemoveAllElements();

  CloseReport();
}


void PEEventDispatcher::RegisterSensor(PESensor* ToRegister)
{
  int i;

  registeredSensors.InsertBefore(ToRegister);

  for (i = scTraceMax; i--;)
  {
    if (ToRegister->NotifyOnEvent(SCTraceAction(i)))
    {
      activateOnAction[i].InsertBefore(ToRegister);
    }  
  }
}  


void PEEventDispatcher::RegisterUpdater(PCUpdater* ToRegister)
{
  registeredUpdaters.InsertBefore(ToRegister);
}


void PEEventDispatcher::ResetAllSensors(void)
{
  PESensor * sensor;

  SCListIter<PESensor> iter(registeredSensors);

  for (sensor = iter++;
       sensor;
       sensor = iter++)
  {
    sensor->Reset();
  }
}


void PEEventDispatcher::OpenReport(const char * FileName)
{
  report = new SCStream(FileName);
}


void PEEventDispatcher::CloseReport(void)
{
  delete report;

  report = NULL;
}


void PEEventDispatcher::ReportAllSensors(void)
{
  PESensor *             sensor;
  SCListIter<PESensor>   iter(registeredSensors);
  SCListCons<PESensor> * head;

  assert(report);

  head = registeredSensors.Head();
  sensor = (*head)();

  (*report) << std::endl << "PEV-Report for experiment '" << experiment << "' at ";
  (*report) << sensor->Now(); // Das Controller-Objekt
  (*report) << ":\n==========\n\n";

  for (sensor = iter++;
       sensor;
       sensor = iter++)
  {
    sensor->Report(*report);
  }
  (*report) << "<<< End of report >>>\n\n";
}


void PEEventDispatcher::Update(void)
{
  PCUpdater *           updater;
  SCListIter<PCUpdater> iter(registeredUpdaters);

  for (updater = iter++;
       updater;
       updater = iter++)
  {
    updater->Update();
  }
  xEventDispatcher.UpdateDisplays(); // Anzeige aktualisieren
}


void PEEventDispatcher::SetUpdateMode(SCBoolean Async)
{
  asyncUpdate = Async;
}


void PEEventDispatcher::SetReportInterval(double Interval)
{
  reportInterval = Interval;
}


void PEEventDispatcher::WrongSCLAction()
{
  std::cerr << "Invalid scTraceAction!\n";
  abort();
} 


// Umsetzung von LogEvents in Sensor-Ereignisse. Der Aufruf der Funktion exit()
// erfolgt immer dann, wenn der von der SCL gemeldete Trace-Typ nicht mit 
// der aufgerufenen LogEvent-Funktion uebereinstimmt.
// ----------------------------------------------------------------------------

#if 0
#define ENTER DebugPrintAction(pAction)
#define LEAVE cout << "Action ended!\n"

void DebugPrintAction(long action)
{
  cout << "Beginning Action: ";
  switch (action)
  {
    case scTraceSchedulerInit:    cout << "Init"; break;
    case scTraceSchedulerStop:    cout << "Stop"; break;
    case scTraceEnd:              cout << "End"; break;
    case scTraceTimeChange:       cout << "TimeChange"; break;
    case scTraceProcessCreate:    cout << "ProcessCreate"; break;
    case scTraceProcessDelete:    cout << "ProcessDelete"; break;
    case scTraceMachineCreate:    cout << "MachineCreate"; break;
    case scTraceMachineDelete:    cout << "MachineDelete"; break;
    case scTraceStateChange:      cout << "StateChange"; break;
    case scTraceSignalSend:       cout << "SignalSend"; break;
    case scTraceSignalReceive:    cout << "SignalReceive"; break;
    case scTraceSignalConsume:    cout << "SignalConsume"; break;
    case scTraceSignalSave:       cout << "SignalSave"; break;
    case scTraceSignalDrop:       cout << "SignalDrop"; break;
    case scTraceSpontTrans:       cout << "SpontTrans"; break;
    case scTraceContSignal:       cout << "ContSignal"; break;
    case scTraceServiceRequest:   cout << "ServiceRequest"; break;
    case scTraceServiceFinish:    cout << "ServiceFinish"; break;
    case scTraceServiceStart:     cout << "ServiceStart"; break;
    case scTraceServiceInterrupt: cout << "ServiceInterrupt"; break;
    case scTraceTimerSet:         cout << "TimerSet"; break;
    case scTraceTimerReset:       cout << "TimerReset"; break;
    case scTraceTimerFire:        cout << "TimerFire"; break;
    case scTraceTimerRemove:      cout << "TimerRemove"; break;
    default: cout << "Unknown"; break;					    
  }
  cout << endl;
}

#else
#define ENTER
#define LEAVE
#endif

// Scheduler Init, Scheduler Stop, Simulation End, Deadlock
void PEEventDispatcher::LogEvent(const SCInteger pAction)
{
  ENTER;
  
  PESensor *           sensor;
  SCListIter<PESensor> iter(activateOnAction[pAction]);

  DoXEvents();
  
  switch(pAction)
  {

    case scTraceSchedulerInit: 
      for (sensor = iter++;
           sensor;
           sensor = iter++)
      {
        sensor->EvSchedInit(); 
      }
      break;
    
    case scTraceSchedulerStop: 
      for (sensor = iter++;
           sensor;
           sensor = iter++)
      {
        sensor->EvSchedStop(); 
      }
      iter.GoToFirst();
      for (sensor = iter++;
           sensor;
           sensor = iter++)
      {
        sensor->EvEnd(); 
      }
      ReportAllSensors();
      break;

    case scTraceDeadlock:
      break;

    default: WrongSCLAction();
  }
  LEAVE;
}


// Simulation time change
void PEEventDispatcher::LogEvent(const SCInteger pAction, const SCTime newTime)
{
  PESensor *           sensor;
  SCListIter<PESensor> iter(activateOnAction[scTraceTimeChange]);

  if (newTime < 0) return;

  ENTER;
  
  // Generiere Update-Ereignis
  // -------------------------
  if (asyncUpdate)
  {
    if ((clock() - lastUpdateClock) > updateInterval)
    {
      Update();
      DoXEvents(); // X-Server kriegt Arbeit
      lastUpdateClock = clock();
    }
  }
  else
  {
    Update();
    DoXEvents();
    {  // Dieser Block simuliert ein Sleep mit einer Aufloesung kleiner als Sekunden    
       struct timeval TimeOut;
       TimeOut.tv_sec = 0;
       TimeOut.tv_usec = sleepUSecs; // 1000000 usecs = 1 sec
       select(0, NULL, NULL, NULL, &TimeOut);
    }
  }
  DoXEvents();
  
  // Erzeuge Report gemaess Intervalleinstellung
  // -----------------------------------------
  if (reportInterval && ((newTime - lastReport) > reportInterval))
  {
    lastReport = newTime;
    ReportAllSensors();
    ResetAllSensors();
  }
  
  if (pAction != scTraceTimeChange) WrongSCLAction(); 
  for (sensor = iter++; sensor; sensor = iter++)
  {
    sensor->EvTimeChange(newTime);
  }  
  LEAVE;
}


// Machine create & delete
void PEEventDispatcher::LogEvent(const SCInteger pAction, SCMachine* machine)
{
  ENTER;
  DoXEvents();
  
  PESensor *           sensor;
  SCListIter<PESensor> iter(activateOnAction[pAction]);

  switch (pAction)
  {
    case scTraceMachineCreate: 
      for (sensor = iter++;
           sensor;
           sensor = iter++)
      {
        sensor->EvMachineCreate(machine);
      }
      break;
    
    case scTraceMachineStop:
      for (sensor = iter++;
           sensor;
           sensor = iter++)
      {
        sensor->EvMachineDelete(machine);
      }
      break;
   
    default: WrongSCLAction();
  }
  LEAVE;
}

// Process delete
void PEEventDispatcher::LogEvent (const SCInteger pAction, SCProcess *process)
{
  ENTER;
  DoXEvents();
  
  PESensor * sensor;
  SCListIter<PESensor> iter(activateOnAction[pAction]);

  switch (pAction)
  {
    case scTraceProcessStop:
      for (sensor = iter++;
           sensor;
           sensor = iter++)
      {
        sensor->EvProcessDelete(process);
      }
      break;
   
    default: WrongSCLAction();
  }
  LEAVE;
}

// Process create
void PEEventDispatcher::LogEvent(const SCInteger pAction, 
                                 SCProcess* process,
                                 SCProcess* creator)
{
  ENTER;
  DoXEvents();
  
  PESensor *           sensor;
  SCListIter<PESensor> iter(activateOnAction[pAction]);

  switch (pAction)
  {
    
    case scTraceProcessCreate: 
      for (sensor = iter++;
           sensor;
           sensor = iter++)
      {
        sensor->EvProcessCreate(process, creator);
      }
      break;
    
    default: WrongSCLAction();
  }
  LEAVE;
}

//
// Procedure return
//

void PEEventDispatcher::LogEvent (const SCInteger pAction,
                                  SCProcedure *procedure)
{
  (void)procedure;
  
  switch (pAction)
  {
    case scTraceProcedureReturn:
      break;

    default:
      break;
  }
}


//
// Procedure call
//

void PEEventDispatcher::LogEvent (const SCInteger pAction,
                                  SCProcedure *   procedure,
                                  SCAutomaton *   caller)
{
  (void)procedure;
  (void)caller;
  
  switch (pAction)
  {
    case scTraceProcedureCall:
      break;

    default:
      break;
  }
}

// Request issue, start, stop, finish
void PEEventDispatcher::LogEvent(const SCInteger  pAction,
                                 SCMachine*       machine,
                                 const SCRequest* request)
{
  ENTER;
  DoXEvents();
  
  PESensor * sensor;
  SCListIter<PESensor> iter(activateOnAction[pAction]);

  switch (pAction)
  {
    case scTraceServiceRequest: 
      for (sensor = iter++;
           sensor;
           sensor = iter++)
      {
        sensor->EvServiceRequest(machine, request);
      }
      break;
    
    case scTraceServiceStart:
      for (sensor = iter++;
           sensor;
           sensor = iter++)
      {
        sensor->EvServiceStart(machine, request);
      }
      break;
 
    case scTraceServiceInterrupt:
      for (sensor = iter++;
           sensor;
           sensor = iter++)
      {
        sensor->EvServiceInterrupt(machine, request);      
      }
      break;
      
    case scTraceServiceFinish:
      for (sensor = iter++;
           sensor;
           sensor = iter++)
      {
        sensor->EvServiceFinish(machine, request);      
      }
      break;
    
    default: WrongSCLAction();
  }
  LEAVE;
}

// Signal send
void PEEventDispatcher::LogEvent(const SCInteger  pAction, 
                                 SCProcess *      sender,
                                 SCProcess *      receiver,
                                 const SCSignal * signal,
                                 const SCDuration delay)
{
  (void)delay;

  ENTER;
  DoXEvents();
  
  PESensor *           sensor;
  SCListIter<PESensor> iter(activateOnAction[scTraceSignalSend]);

  if (pAction != scTraceSignalSend)
    WrongSCLAction();

  if ((actionFlags & scfTraceSignalSend))
  {
    for (sensor = iter++;
         sensor;
         sensor = iter++)
    {
      sensor->EvSignalSend(sender, receiver, signal);
    }
  }

  LEAVE;
}

// Signal not sent
void PEEventDispatcher::LogEvent (const SCInteger      pAction,
                                  SCProcess *          sender,
                                  const SCSignalType * signalType)
{
  ENTER;
  DoXEvents();

  (void)sender;
  (void)signalType;

  switch (pAction)
  {
    case scTraceSignalNotSent:
      break;

    default: WrongSCLAction();
  }
  LEAVE;
}

// Signal Receive, Save, Drop, Reject
void PEEventDispatcher::LogEvent(const SCInteger  pAction,
                                 SCProcess *      process,
                                 const SCSignal * signal)
{
  ENTER; 
  DoXEvents();

  PESensor * sensor;
  SCListIter<PESensor> iter(activateOnAction[pAction]);

  switch (pAction)
  {
    case scTraceSignalReceive:
      for (sensor = iter++;
           sensor;
           sensor = iter++)
      {
        sensor->EvSignalReceive(process, signal);
      }
      break;

    case scTraceSignalConsume: 
      for (sensor = iter++;
           sensor;
           sensor = iter++)
      {
        sensor->EvSignalConsume(process, signal);
      }
      break;
    
    case scTraceSignalSave:
      for (sensor = iter++;
           sensor;
           sensor = iter++)
      {
        sensor->EvSignalSave(process, signal);
      }
      break;
 
    case scTraceSignalDrop:
      for (sensor = iter++;
           sensor;
           sensor = iter++)
      {
        sensor->EvSignalDrop(process, signal);
      }
      break;
      
    case scTraceSignalLose:
      for (sensor = iter++;
           sensor;
           sensor = iter++)
      {
        sensor->EvSignalReject(process, signal);
      }
      break;

    case scTraceTimerRemove:
      for (sensor = iter++;
           sensor;
           sensor = iter++)
      {
        sensor->EvSignalDrop(process, signal);
      }
      break;

    default: WrongSCLAction();
  }
  LEAVE;
}

// Signal consume
void PEEventDispatcher::LogEvent (const SCInteger pAction,
                                  SCProcess *process, const SCSignal *signal,
                                  const SCTransition *transition)
{
  ENTER; 
  DoXEvents();

  (void)transition;

  PESensor * sensor;
  SCListIter<PESensor> iter(activateOnAction[pAction]);

  switch (pAction)
  {
    case scTraceSignalConsume: 
      for (sensor = iter++;
           sensor;
           sensor = iter++)
      {
        sensor->EvSignalConsume(process, signal);
      }
      break;
    default: WrongSCLAction();
  }
  LEAVE;
}

// Spontaneous Transition, Continuous Signal
void PEEventDispatcher::LogEvent(const SCInteger  pAction,
                   SCProcess* process,
                   const SCTransition *transition)
{
  ENTER;
  DoXEvents();

  (void)transition;
  
  PESensor * sensor;
  SCListIter<PESensor> iter(activateOnAction[pAction]);

  switch (pAction)
  {
    case scTraceSpontTrans:
      for (sensor = iter++;
           sensor;
           sensor = iter++)
      {
        sensor->EvSpontTrans(process);
      }
      break;
      
    case scTraceContSignal:
      for (sensor = iter++;
           sensor;
           sensor = iter++)
      {
        sensor->EvContSignal(process);
      }
      break;
      
    default: WrongSCLAction();
  }
  LEAVE;
}

// Timer set & reset + delayed output
void PEEventDispatcher::LogEvent(const SCInteger pAction,
                                 SCProcess *     process,
                                 const SCTimer * timer)
{
  (void)process;

  ENTER;
  DoXEvents();
  
  PESensor *           sensor;
  SCListIter<PESensor> iter(activateOnAction[pAction]);

  switch (pAction)
  {
    
    case scTraceTimerSet: 
      for (sensor = iter++;
           sensor;
           sensor = iter++)
      {
        sensor->EvTimerSet(timer);
      }
      break;
    
    case scTraceTimerReset:
      for (sensor = iter++;
           sensor;
           sensor = iter++)
      {
        sensor->EvTimerReset(timer);
      }
      break;
 
    case scTraceSignalSend: // Delayed Output
      break;                // not supported by PEV!

    default: WrongSCLAction();
  }
  LEAVE;
}

// Timer fire
void PEEventDispatcher::LogEvent(const SCInteger  pAction,
                                 SCProcess *      process,
                                 const SCTimer *  timer,
                                 const SCSignal * signal)
{
  (void)process;

  ENTER;
  DoXEvents();
  
  PESensor *           sensor;
  SCListIter<PESensor> iter(activateOnAction[scTraceTimerFire]);

  if (pAction != scTraceTimerFire)
    WrongSCLAction();

  if ((actionFlags & scfTraceTimerFire))
  {
    for (sensor = iter++; sensor; sensor = iter++)
    {
      sensor->EvTimerFire(timer, signal);
    }
  }
  LEAVE;
}
    
// State change
void PEEventDispatcher::LogEvent(const SCInteger     pAction,
                                 SCProcess *         process,
                                 const SCStateType * newState,
                                 const SCDuration    awakeDelay)
{
  (void)awakeDelay;
  
  ENTER;
  DoXEvents();
  
  PESensor *           sensor;
  SCListIter<PESensor> iter(activateOnAction[scTraceStateChange]);

  if (pAction != scTraceStateChange)
    WrongSCLAction();

  if ((actionFlags & scfTraceStateChange))
  {
    for (sensor = iter++; sensor; sensor = iter++)
    {
      sensor->EvStateChange(process, newState);
    }
  }

  LEAVE;
}

