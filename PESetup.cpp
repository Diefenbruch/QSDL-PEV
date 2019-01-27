/******************************************************************************\
 Datei : PESetup.cpp
 Inhalt: Implementierung der PEEventDispatcher::Setup Funktion, als Einlesen
         einer Konfigurationsdatei mit einer Experimentbeschreibung.
 Autor : Christian Rodemeyer, Marc Diefenbruch
 Datum : 10.10.98
 Status: Primitiver Parser und Uebersetzungsmechanismus. Aehnliche Funktionen 
         werden nicht zusammengefasst sondern via Copy & Paste an spezielle
	 Faelle angepasst, Zeitdruck laesst eleganteres Konzept nicht mehr zu.
         Umgeschrieben von MD
\******************************************************************************/   

#include "PEEventDispatcher.h"
#include "PCController.h"
#include "PVDataDisplay.h"
#include "PESMachine.h"
#include "PESProcess.h"
#include "PESActivity.h"
#include "PESetup.h"
#include "PEScanner.h"

#include <stdio.h>
#include <ctype.h>
#include <assert.h>

#include <string.h>

#if _SC_DMALLOC
  #include <dmalloc.h>
#endif

static const char * SensorTypeNames[numSensorTypes + 1] = 
{ 
  "ProcessQLen",
  "ProcessQLenFreq",
  "ProcessNumber",
  "ProcessSigWaitTime", 
  "ProcessStateFreq",
  "ProcessInSigFreq",
  "ProcessOutSigFreq",
  "ProcessOutReqFreq",
  "MachineQLen",
  "MachineQLenFreq",
  "MachineReqWaitTime",
  "MachineReqThruTime",
  "MachineInReqFreq",
  "MachineUtilization",
  "GlobalSigFreq",
  "GlobalReqFreq",
  "SimpleEvent",
  "SimpleActivity",
  "" // Wichtiges Ende-Kenzeichen
};


static const char * DisplayTypeNames[numDisplayTypes + 1] = 
{
  "Curves", "FixedCurves", "Gantt", "Freqs",
  "" // Leerer String als Ende Kennzeichen
};


SCBoolean IsDiscreteValIndex(long ValIndexType)
{
  static const long IsDiscrete = (1 << vNum) |
                                 (1 << vMin) |
		   	         (1 << vMax) |
			         (1 << vCNT);  
  
  return (1 << ValIndexType) & IsDiscrete;
}


SCBoolean SensorTable::Add(const SensorDef& Sensor)
{
  TEntryIter iter(entryList);
  TEntry *entry;

  for (entry = iter++; entry; entry = iter++)
  {
    if (!strcmp(entry->sensor.name, Sensor.name)) return false;
  }
  entryList.InsertAfter(new TEntry(Sensor));

  return true;
}

SCBoolean SensorTable::Get(SensorDef& Sensor)
{
  TEntryIter iter(entryList);
  TEntry *entry;

  for (entry = iter++; entry; entry = iter++)
  {
    if (!strcmp(entry->sensor.name, Sensor.name))
    {
      Sensor = entry->sensor;
      return true;
    }
  }
  return false;
}


void DataTypeTable::Add(const PESensor* Sensor, int ValIndex,
                        PDDataType* DataType)
{
  entryList.InsertAfter(new TEntry(DataType, Sensor, ValIndex));
}


SCBoolean DataTypeTable::Get(const PESensor* Sensor, int ValIndex,
                             PDDataType*& DataType)
{
  TEntryIter iter(entryList);
  TEntry *entry;

  for (entry = iter++; entry; entry = iter++)
  {
    if (entry->sensor == Sensor && entry->valIndex == ValIndex)
    {
      DataType = entry->dataType;
      return true;
    }
  }
  return false;
}
  

void InstantiateSensor(SensorDef &                 Sensor,
                       const char *                Buf1,
                       const char *                Buf2,
                       PDEventType* const          Ev1,
                       PDEventType* const          Ev2,
                       SCDuration                  Interval)
{
  PESStateFrequency * stateFreqSensor;

  Sensor.sensor = NULL;
  Sensor.stateTable = NULL;
    
  // Monstermaessige switch-Anweisung um fuer jeden Typ den richtigen Sensor
  // mit den richtigen Parametern zu erzeugen
  // --------------------------------------------------------------------
  switch (Sensor.type)
  {
    case sProcQLen:
      Sensor.sensor = new PESProcessQLen(Buf1, Interval);
      break;

    case sProcQLenFreq:
      Sensor.sensor = new PESProcessQLenFrequency(Buf1); 
      break;

    case sProcNumber:
      Sensor.sensor = new PESProcessNumber(Buf1, Interval);
      break;
	 
    case sProcSigWaitTime:   
      Sensor.sensor = new PESSignalWaitTime(Buf2, Interval, Buf1); 
      break;

    case sProcStateFreq:
      stateFreqSensor = new PESStateFrequency(Buf1); 
      Sensor.sensor = stateFreqSensor;
      Sensor.stateTable = &stateFreqSensor->GetStateTable();
      break;

    case sProcInSigFreq:
      Sensor.sensor = new PESSignalFrequency(Buf1, true); 
      break;

    case sProcOutSigFreq:
      Sensor.sensor = new PESSignalFrequency(Buf1, false); 
      break;
    
    case sProcOutReqFreq:      
      Sensor.sensor = new PESRequestFrequency(NULL, Buf1, false); 
      break;
      
    case sMachQLen:  
      Sensor.sensor = new PESMachineQLen(Buf1, Interval); 
      break;

    case sMachQLenFreq:  
      Sensor.sensor = new PESMachineQLenFrequency(Buf1); 
      break;
    
    case sMachReqWaitTime: 
      Sensor.sensor = new PESRequestWaitTime(Buf2, Interval, Buf1); 
      break;
	 
    case sMachReqThruTime:
      Sensor.sensor = new PESRequestThruTime(Buf2, Interval, Buf1);
      break;

    case sMachInReqFreq:      
      Sensor.sensor = new PESRequestFrequency(Buf1, NULL, true); 
      break;

    case sMachUtilization:  
      Sensor.sensor = new PESMachineUtilization(Buf1, Interval); 
      break;
      
    case sGlobalSigFreq: 
      Sensor.sensor = new PESGlobalSignalFrequency();
      break;

    case sGlobalReqFreq: 
      Sensor.sensor = new PESGlobalRequestFrequency();
      break;
    
    case sEvent:
      Sensor.sensor = new PESEvent(Buf1, Ev1, Interval); 
      break; 
 
    case sActivity:
      Sensor.sensor = new PESActivity(Buf1, Ev1, Ev2, Interval); 
      break;
    
    default:
      std::cout << "Internal Error while constructing Sensor\n";
      abort();
      break;  
  }
}

PVFrameDisplay* InstantiateFrame(Display*                    Dpy,
                                 int                         DispType,
                                 const char *                Name,
                                 int                         Adaption,
                                 const SensorDef&            FirstSensor)
{
  switch (DispType)
  {
	    
    case dCurves: 
      return new PVCurvesFrame(Dpy, Name, Adaption);
    
    case dFixedCurves:
      return new PVCurvesFrame(Dpy, Name, Adaption, true);
      
    case dGantt:
      return new PVGanttFrame(Dpy, Name, Adaption);
	
    case dFreqs: 
      
      switch(FirstSensor.type)
      {
        case sProcStateFreq: 
          return new PVFreqFrame(Dpy, Name, SC_STATE);
	  
        case sProcInSigFreq:
        case sProcOutSigFreq:
        case sGlobalSigFreq:
          return new PVFreqFrame(Dpy, Name, SC_SIGNAL);
 
        case sProcOutReqFreq:
        case sMachInReqFreq:
        case sGlobalReqFreq:
          return new PVFreqFrame(Dpy, Name, SC_REQUEST);
 
        case sProcQLenFreq:
        case sMachQLenFreq:
          return new PVFreqFrame(Dpy, Name, SC_NONE);
      }
 
    default: std::cout << "Internal Error while creation Display\n"; abort();
  }
}

PDDataType* InstantiateDataType(PEEventDispatcher* dispatcher,
                                int                DispType,
                                PESensor *         Sensor,
                                int                ValIndex,
                                int                Points,
                                const long         color)
{
  PDDataType* Data;
  
  if (DispType == dGantt)
  { 
    PDDiscreteCurve* DCurve;
    static const int GS = PESStateFrequency::ganttState;
    Data = DCurve = new PDDiscreteCurve(Points, color);

    dispatcher->RegisterUpdater(new PCCurveUpdater(DCurve, Sensor, GS)); 
  }  
  else if (DispType == dCurves || DispType == dFixedCurves)
  { 
    PDCurve* Curve;

    if (IsDiscreteValIndex(ValIndex))
    {
      Curve = new PDDiscreteCurve(Points, color);
    }
    else
    {
      Curve = new PDCurve(Points, color);
    }
    Data = Curve;

    dispatcher->RegisterUpdater(new PCCurveUpdater(Curve, Sensor, ValIndex));
  }
  else
  {
    PDFrequency* Freq;

    Data = Freq = (PDFrequency *)Sensor->GetData();
    Freq->SetColor(color);

    dispatcher->RegisterUpdater(new PCFreqUpdater(Freq, Sensor));
  }
  return Data;
}


void ConnectFrameWithData(int                  DispType, 
                          PVFrameDisplay *     Frame,
                          PDDataType *         Data,
                          const PDStateTable * StateTable)
{
  switch (DispType)
  {
    case dCurves:
    case dFixedCurves:
      ((PVCurvesFrame*)Frame)->AddDataType((PDCurve*)Data);
      break;
    case dGantt:
      ((PVGanttFrame*)Frame)->AddDataType((PDCurve*)Data);
      ((PVGanttFrame*)Frame)->AddStateTable(StateTable);
      break;
    default:
      ((PVFreqFrame*)Frame)->AddDataType((PDFrequency*)Data);
      break;
  }
}

// Einleseroutine
// --------------

void PEEventDispatcher::Setup(const char * Configuration, 
                              const char * Specification)
{
  SensorTable   SensorInstances;
  DataTypeTable DataTypeInstances;
  Scanner       Scan(Configuration);
  int           Points;
  int           Adaption;
  SCDuration    DefaultInterval;

  // Allgemeine Experimentbeschreibung einlesen
  // ------------------------------------------
  char Buffer[128];
  
  Scan.GetKeyString("Experiment", experiment);  
  {
    PCController* t = new PCController(this, xEventDispatcher, experiment);
    RegisterSensor(t);
    xEventDispatcher.AddDisplay(t);
  }
  
  Scan.GetKeyString("Specification", Buffer);
  if (strcmp(Buffer, Specification)) Scan.Error("Wrong specification error");
  
  Scan.GetKeyWord(Buffer); 
  if (strcmp(Buffer, "Report")) Scan.Error("Keyword 'Report' expected");
  Scan.GetChar(':', "after 'Report'"); Scan.GetString(Buffer);
  OpenReport(Buffer);
  if (Scan.CheckChar(','))
  {
    double Interval;
    Scan.GetChar(',', "before Reportinterval");
    Scan.GetDbl(Interval);
    SetReportInterval(Interval);
  }
  Scan.GetChar(';', "");
  
  Scan.GetKeyInt("CurvePoints", Points);
  if (Points < 8 || Points > 256) Scan.Error("Range error");
  
  Scan.GetKeyInt("ScaleAdaption", Adaption);
  if (Adaption < 5 || Adaption > 50) Scan.Error("Range error");

  Scan.GetKeyDbl("DefaultInterval", DefaultInterval);
    
  // SensorCreation
  // --------------
  {
    SensorDef    Sensor;
    char         Buf1[128], Buf2[128];
    PDEventType* Ev1;
    PDEventType* Ev2;
    double       Interval;
   
    Scan.GetKeyBlock("SensorCreation");
    while (Scan.GetKeyWordIndex(SensorTypeNames, Sensor.type))
    { 
      Scan.GetKeyWord(Sensor.name);
      if (SensorInstances.Get(Sensor))
      {
	std::cerr << "Redefinition of sensor " << Sensor.name << std::endl;
      }
      if (Sensor.type != sGlobalSigFreq && Sensor.type != sGlobalReqFreq)
      {
        Scan.GetChar(':', "after sensor identifier");
      }	
      Interval = DefaultInterval;
      Scan.GetSensorParameters(Sensor.type, Buf1, Buf2, Ev1, Ev2, Interval);
      InstantiateSensor(Sensor, Buf1, Buf2, Ev1, Ev2,
                        Interval);
      if (Sensor.sensor == NULL)
      {
        std::cerr << "Illegal sensor " << Sensor.name << "." << std::endl;
      }
      assert(Sensor.sensor);
      SensorInstances.Add(Sensor);
      RegisterSensor(Sensor.sensor);
    } 	
    Scan.GetChar('}', "or unknown sensortype");
  }  

  
  // DisplayCreation
  // ---------------
  {
    int             DispType;
    char            DispName[128];
    char            ColorName[128];
    PVFrameDisplay* Frame;
    SensorDef       Sensor;
    int             ValIndex;
    PDDataType*     Data;
    
    Scan.GetKeyBlock("DisplayCreation");
    while (Scan.GetKeyWordIndex(DisplayTypeNames, DispType))
    { 
      Scan.GetString(DispName);
      Scan.GetChar(':', "after displayname");
      ValIndex = -1;
      Frame = NULL;
      while (!Scan.CheckChar(';'))
      {
        Scan.GetKeyWord(Sensor.name);
        if (!SensorInstances.Get(Sensor))
        {
          std::cerr << "Sensor " << Sensor.name << " is undefined" << std::endl;
        } 
        Scan.GetConParas(ValIndex, ColorName, DispType, Sensor.type);
	
        // Erzeuge Display, falls noch nicht existent
        // ------------------------------------------
        if (!Frame)
        {
          Frame = InstantiateFrame(xEventDispatcher.GetXDisplay(),
                                   DispType, DispName, Adaption,
                                   Sensor);
          xEventDispatcher.AddDisplay(Frame);
        }
        else if (DispType == dFreqs && Sensor.type == sProcStateFreq)
        {
          Scan.Error("Only one process per Process-State-Freqs-Display allowed");
        }

        // Finde oder erzeuge korrekten Datentyp
        // -------------------------------------
        if (!DataTypeInstances.Get(Sensor.sensor, ValIndex, Data))
        {
          Data = InstantiateDataType(this, DispType, Sensor.sensor,
                                     ValIndex, Points,
                                     Frame->GetColor(ColorName));
          DataTypeInstances.Add(Sensor.sensor, ValIndex, Data);
        }
        ConnectFrameWithData(DispType, Frame, Data, Sensor.stateTable);

        if (Scan.CheckChar(',')) Scan.GetChar(',', "");
      }  // naechstes Argument
      Scan.GetChar(';', "");
    } // naechstes Display
    Scan.GetChar('}', "or unknown display type");
  }
  
  xEventDispatcher.ArrangeDisplays();
}


SCStream& operator<< (SCStream& pStream, const SensorTable::TEntry&)
{
  return pStream;
}


SCStream& operator<< (SCStream& pStream, const DataTypeTable::TEntry&)
{
  return pStream;
}
