/******************************************************************************\
 Datei : PESensor.h
 Inhalt: Deklaration der abstrakten Sensorklassen, aus denen die speziellen
         Datenermittelnden Sensoren abgeleitet werden:
	 PESensor, PESTally, PESFrequency, PESQueueLength, 
	 PESQueueFrequency.
 Autor : Christian Rodemeyer, Marc Diefenbruch
 Datum : 20.10.95
 Status: Report ist noch nicht implementiert
\******************************************************************************/   

#ifndef __PESENSOR_H
#define __PESENSOR_H

#include <SCL/SCStream.h>

#include <SCL/SCMachine.h>
#include <SCL/SCSignal.h>
#include <SCL/SCRequest.h>
#include <SCL/SCTimer.h>
#include <SCL/SCStateType.h>
#include <SCL/SCScheduler.h>
#include <SCL/SCTraceTypes.h>

#ifndef __PDDATATYPE_H
#include "PDDataType.h"
#endif

#include <SCL/SCList.h>
#include <SCL/SCSensor.h>

/******************************************************************************\
 PESensor: Abstrakte Basisklasse der Leistungsdatenermittlung   
\******************************************************************************/  

class PESensor
{
  public:

    PESensor(void) {}
    virtual ~PESensor(void) {}  // Spezialisierung erwartet => Virtueller Destruktor
    
    virtual SCBoolean NotifyOnEvent(SCTraceAction Event) const = 0; // TRUE, falls Benachrichtigung erwünscht
    virtual void Reset() = 0;                            // Zurücksetzen des Sensors     
    virtual void Report(SCStream& Out) const = 0;  
    virtual double GetValue(int ValueIndex) const = 0;
    
    virtual const PDDataType * GetData(void) const { return NULL; };

    SCTime Now() const {return SCScheduler::GetCurrentTime();} 
    
    // Fuer jede Aktion wird eine Ereignisfunktion bereitgestellt, die
    // defaultmaessig gar nichts tut. Abgeleitete Sensor-Klassen ueberschreiben
    // die Ereignisse, die sie zur Ermittlung ihrer Daten bentigen
    // ----------------------------------------------------------------------
    
    // Allgemeine Ereignisse
    // --------------------------------
    virtual void EvSchedInit  (void)                     {}
    virtual void EvSchedStop  (void)                     {}
    virtual void EvEnd        (void)                     {}
    virtual void EvTimeChange (const double /*newTime*/) {}
    
    // Prozessverwaltung
    // ----------------
    virtual void EvProcessCreate(const SCProcess*   /* Process */, 
                                 const SCProcess*   /* Creator */)      {}
    virtual void EvProcessDelete(const SCProcess*   /* Process */)      {}
    virtual void EvStateChange  (const SCProcess*   /* Process */,
                                 const SCStateType* /* NewState */)     {} 
    virtual void EvSpontTrans   (const SCProcess*   /* Process */) {}
    virtual void EvContSignal   (const SCProcess*   /* Process */) {}

    // Prozess-Botschaften/Signale
    // --------------------------
    virtual void EvSignalSend    (const SCProcess*    /* Sender */,
                                  const SCProcess*    /* Receiver */,
                                  const SCSignal* /* Signal */) {}
    virtual void EvSignalConsume (const SCProcess*    /* Process */,
                                  const SCSignal* /* Signal */) {}
    virtual void EvSignalSave    (const SCProcess*    /* Process */,
                                  const SCSignal* /* Signal */) {}
    virtual void EvSignalDrop    (const SCProcess*    /* Process */,
                                  const SCSignal* /* Signal */) {}
    virtual void EvSignalReject  (const SCProcess*    /* Process */,
                                  const SCSignal* /* Signal */) {}
    virtual void EvSignalReceive (const SCProcess*    /* Process */, 
                                  const SCSignal* /* Signal */) {}
				 
    // Maschinen und Requests
    // ----------------------
    virtual void EvMachineCreate   (const SCMachine*      /* machine */) {}
    virtual void EvMachineDelete   (const SCMachine*      /* machine */) {}
    virtual void EvServiceRequest  (const SCMachine*      /* machine */,
                                    const SCRequest* /* request */) {}
    virtual void EvServiceFinish   (const SCMachine*      /* machine */,
                                    const SCRequest* /* request */) {}
    virtual void EvServiceStart    (const SCMachine*      /* machine */,
                                    const SCRequest* /* request */) {}
    virtual void EvServiceInterrupt(const SCMachine*      /* machine */,
                                    const SCRequest* /* request */) {}

    // Timer
    // -----
    virtual void EvTimerSet   (const SCTimer* /* timer */)   {}
    virtual void EvTimerReset (const SCTimer* /* timer */)   {}
    virtual void EvTimerFire  (const SCTimer* /* timer */,
                               const SCSignal* /* message */) {}
    
    friend SCStream& operator<< (SCStream& pStream,
                                 const PESensor& pData);

  protected:
    void Underline(SCStream& Out, int len) const; // kleine Hilfsfunktion für Reports
};

/******************************************************************************\
 PESTally: Statistische Auswertung 
\******************************************************************************/ 

class PESTally: virtual public PESensor
{
  public:

    PESTally(SCDuration IntervalLength = 1.0);
    
    enum { // Indices für GetValue zum Auslesen statistischer Informationen
      num, // Anzahl der Stichproben 
      min, // Minimum
      max, // Maximum
      avg, // Durchschnitt
      avi, // Durchsnitt im letzten Interval
      var, // Varianz
      dev, // Standardabweichung
      __T  // Ende Kennzeichen für Tally
    };
    double GetValue(int ValIndex) const;
  
    void Reset(void);
    void Report(SCStream& Out) const;

  protected:  
    void UpdateTally(double Sample, double Weight = 1.0);
    
  private:  
    double      numS;  // Anzahl Stichproben (Summe der Gewichte)
    long double sumS;  // Summe aller Stichproben
    long double sumS2; // Summe aller Stichprobenquadrate
    double      minS;  // Minimum aller Stichproben
    double      maxS;  // Maximum
    double      intervalAvg;    // Mittelwert im letzten Intervall
    double      intervalNum;    // Summe Gewichte im laufenden Intervall
    double      intervalSum; // Aktueller Mittelwert im laufenden Intervall
    SCDuration  intervalLen;
    SCTime      intervalStop;
};

/******************************************************************************\
 PESCounter: Zähler für Ereignisse
\******************************************************************************/ 

class PESCounter: virtual public PESensor
{
  public:
    PESCounter(SCDuration IntervalLength = 1.0);
  
    enum { // Indices für GetValue zum Auslesen statistischer Informationen
      cnt = PESTally::__T, // Anzahl gezählter Ereignisse
      cpt,                 // Ereignisse pro Zeit (Lebenszeit des Sensors)
      cpi,                 // Ereignisse pro letztes Interval
      __C                  // Ende Kennzeichen für Counter
    };
    double GetValue(int ValIndex) const;
  
    void Reset();
    void Report(SCStream& Out) const;

  protected:
    void UpdateCounter(); // Erhoeht Counter um eins
    
  private:
    long       count;
    long       intervalCount;
    SCTime     creation;
    double     countsPerInterval;
    SCTime     intervalStop;
    SCDuration intervalLength;
};
  
/******************************************************************************\
 PESFrequency: Sensor fuer (relative) Haeufigkeiten. Die Haeufigkeiten werden
   von abgeleitetetn Klassen ermittelt (wie beim Tally)
\******************************************************************************/ 

class PESFrequency: virtual  public PESensor
{
  public:
    PESFrequency(const SCObjectType ObjectType);           
    
    void   Reset(void);
    void   Report(SCStream& Out) const;
    double GetValue(int Index) const;
    const  PDFrequency& GetFrequency() const;
    
    const PDDataType * GetData(void) const { return &GetFrequency(); }
    
  protected:
    void   UpdateFreq(int i, double Diff);
    
  private:  
    PDFrequency        freq;      // Haeufigkeitsobjekt
    SCObjectType       objectType;
};

/******************************************************************************\
 PESBasicQLen: Hilfsklasse um Gemeinsamkeiten von Laengen und Verteilungs-
   sensoren zu implementieren.
\******************************************************************************/ 

class PESQueue: virtual public PESensor
{
  public:
    void Reset();
    
  protected:  
    PESQueue();
    
    virtual void UpdateQLen(int QLenDiff); 
    
    int    QLen() const          {return qLen;}
    SCTime Duration() const      {return Now() - lastUpdate;}
    
  private:
    int    qLen;        // Aktuelle Länge der Warteschlange
    SCTime lastUpdate;  // Zeitpunkt der letzten Queue-Änderung 
};


/******************************************************************************\
 PESQueueLength: 
\******************************************************************************/ 

class PESQueueLength: public PESTally, protected virtual PESQueue
{
  public:
    
    PESQueueLength(SCDuration Interval): PESTally(Interval) {};
    
    enum {                    // Weitere Indices fuer GetValue
      cql = PESCounter::__C,  // Zugriff auf aktuelle Laenge der Warteschlange
      _QL                     // Ende Kennzeichen fuer QueueLength
    };	
    double GetValue(int ValIndex) const;
    
    void Reset();
    
  protected:  
    virtual void UpdateQLen(int QLenDiff); 
};


/******************************************************************************\
 PESQueueLengthFrequency: Verteilung der Warteschlangenlänge 
\******************************************************************************/ 

class PESQueueLengthFrequency: public PESFrequency,
                               protected virtual PESQueue
{
  public:
    PESQueueLengthFrequency();
    
    void  Reset(void);
    
  protected:  
    void UpdateQLen(int QLenDiff);
    
  private:
    static int         maxNumber;
};

#endif
