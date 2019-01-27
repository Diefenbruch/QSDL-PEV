/******************************************************************************\
 Datei : PEEventDispatcher.h
 Inhalt: Deklaration des EventDispatcher (PEEventDispatcher) 
         Der EventDispatcher ist die Schnittstelle zwischen einem auf der SCL
	 basierenden QSDL-Simulator und der Leistungsbewertung sowie der
	 Visualisierung der ermittelten Ergebnisse
 Autor : Christian Rodemeyer, Marc Diefenbruch
 Datum : 20.10.95
 Status: Komplett
\******************************************************************************/   

#ifndef __PEEVENTDISPATCHER_H
#define __PEEVENTDISPATCHER_H

#include <time.h>  // Koennte systemspezifisch sein

#include <SCL/SCEnvironment.h>
#include <SCL/SCTrace.h>

#ifndef __PESENSOR_H
#include "PESensor.h"
#endif
#ifndef __PCUPDATER_H
#include "PCUpdater.h"
#endif
#ifndef __PVXEVENTDISPATCHER_H
#include "PVXEventDispatcher.h"   // Verwaltung der Xlib-Ereignisse
#endif

/******************************************************************************\
 PEEventDispatcher: Verwaltung der LogEvent-Ereignisse aus der SCL und 
  Low-Level Steuerung des Simulators (siehe Dipl-Doku). 
    Jedes LogEvent wird in Basis-Ereignisse (scTraceAction) aufgeloest. Jeder 
  Sensor, der sich fuer ein Basis-Ereignis registriert hat, wird benachrichtigt.
  XEvents, die sich waehrend der Simulationszeit angesammelt haben werden
  bearbeitet. Zu diesen Zeitpunkten kann der Benutzer in die Steuerung des
  Simulators eingreifen. Im Gegensatz zu den ueblichen Anwendung mittels 
  X Toolkit Intrinsics oder OSF/Motif hat der Simulator die Kontrolle und nicht
  eine vom Toolkit bereitgestellte Hauptschleife. Waehrend  jedes LogEvents gibt 
  er die Kontrolle an die SCTrace Klasse ab. Diese arbeitet mit DoXEvents die
  aufgelaufenen Ereignisse ab, bevor die Simulation fortgesetzt wird. DoXEvents
  ist eine an die SCL angepasste Neuimplementierung von XtAppMainLoop die sofort
  zurueckkehrt, sobald alle XEvents abgearbeitet wurden. Aufgrund der Haeufigkeit
  der LogEvents bemerkt der Benutzer keinen Unterschied in der Reaktionszeit
  zu einer konventionellen Anwendung (zumindest auf einem 486'er).
\******************************************************************************/

class PEEventDispatcher: public SCTrace 
{
  public:

    PEEventDispatcher(const char * Configuration,
                      const char * Specification,
                      double PicsPerSec = 10); // Updates pro Sekunde
    ~PEEventDispatcher(void);

    void RegisterSensor(PESensor* ToRegister);   // Anmelden und
    // void UnRegisterSensor(PESensor* Sensor);  // Abmelden eines Sensors
    void RegisterUpdater(PCUpdater* ToRegister); // Anmelden
    // void UnRegisterUpdater();                 // und Abmelden
    void ResetAllSensors(void);                  // alle Sensoren zuruecksetzen
    void ReportAllSensors(void);                 // Report ueber Sensoren erzeugen
    void OpenReport(const char * File);          // Oeffnet fstream report
    void CloseReport(void);                      // Schlieﬂt reportstream
    void SetReportInterval(double Interval);     // 
    void SetUpdateMode(SCBoolean Async);         // Asynchrone oder synchrone Updates

    // Von SCTrace geerbte Ereignisfunktionen
    // --------------------------------------

    // Scheduler start, stop
    virtual void LogEvent(const SCInteger             pAction);  

    // Simulation time change
    virtual void LogEvent(const SCInteger             pAction, 
                          const SCTime                newTime);

    // Process delete
    virtual void LogEvent (const SCInteger            pAction,
                           class SCProcess *          process);

    // Process create
    virtual void LogEvent(const SCInteger             pAction,
                          class SCProcess*            process,
                          class SCProcess*            creator);

    // Procedure return
    virtual void LogEvent(const SCInteger             pAction,
                          class SCProcedure *         procedure);

    // Procedure call
    virtual void LogEvent(const SCInteger             pAction,
                          class SCProcedure *         procedure,
                          class SCAutomaton *         caller);

    // Machine create & delete
    virtual void LogEvent(const SCInteger             pAction,
                          class SCMachine*            machine);

    // Request issue, start, stop, finish
    virtual void LogEvent(const SCInteger             pAction,
                          class SCMachine*            machine,
                          const class SCRequest*      request);

    // Signal send
    virtual void LogEvent(const SCInteger             pAction,
                          class SCProcess*            sender,
                          class SCProcess*            receiver,
                          const class SCSignal*       signal,
                          const SCDuration            delay = 0.0);

    // Signal not sent
    virtual void LogEvent (const SCInteger            pAction,
                           class SCProcess *          sender,
                           const class SCSignalType * signalType);

    // Signal receive, save, drop, reject
    // Timer remove
    virtual void LogEvent(const SCInteger             pAction,
                          class SCProcess*            process,
                          const class SCSignal*       signal);

    // Signal consume
    virtual void LogEvent (const SCInteger            pAction,
                           class SCProcess*           process,
                           const class SCSignal*      signal,
                           const class SCTransition*  transition);

    // Spontaneous Transition, Continuous Signal
    virtual void LogEvent(const SCInteger             pAction,
                          class SCProcess*            process,
                          const class SCTransition*   transition);

    // Timer set & reset
    virtual void LogEvent(const SCInteger             pAction,
                          class SCProcess *           process,
                          const class SCTimer*        timer);

    // Timer fire
    virtual void LogEvent(const SCInteger             pAction,
                          class SCProcess *           process,
                          const class SCTimer*        timer,
                          const class SCSignal*       signal);

    // State change
    virtual void LogEvent(const SCInteger             pAction,
                          class SCProcess*            process,
                          const class SCStateType*    newState,
                          const SCDuration            awakeDelay = kSCNoAwakeDelay);

  private:
    PVXEventDispatcher  xEventDispatcher;
    SCList<PESensor>    registeredSensors;
    SCList<PESensor>    activateOnAction[scTraceMax];
    SCList<PCUpdater>   registeredUpdaters;
    const char *        specification;
    char                experiment[80];
    const clock_t       updateInterval;
    clock_t             lastUpdateClock;
    const long          sleepUSecs;
    SCBoolean           asyncUpdate;
    SCStream *          report;
    SCDuration          reportInterval;
    SCTime              lastReport;

    void Update(void); // Update an alle Updater senden
    void Setup(const char * Config, const char * SpecName);
    void DoXEvents(void) {xEventDispatcher.DoEvents();}
    void WrongSCLAction(void);
};

#endif
