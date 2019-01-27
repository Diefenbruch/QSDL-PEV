#include <SCL/SCList.h>

// Datenstrukturen die die erlaubten Sensor- und Displaytypen bestimmen
// --------------------------------------------------------------------

enum SensorType
{
  sProcQLen = 0,
  sProcQLenFreq,
  sProcNumber,
  sProcSigWaitTime,
  sProcStateFreq,
  sProcInSigFreq,
  sProcOutSigFreq,
  sProcOutReqFreq,
  sMachQLen,
  sMachQLenFreq,
  sMachReqWaitTime,
  sMachReqThruTime,
  sMachInReqFreq,
  sMachUtilization,
  sGlobalSigFreq,
  sGlobalReqFreq,
  sEvent,
  sActivity,
  numSensorTypes
};

enum DisplayType
{
  dCurves, dFixedCurves, dGantt, dFreqs,
  numDisplayTypes
};

enum ValIndexType // Achtung, muessen mit den entsprechenden PESTally und
{                 // PESCounter-Werten wertemaessig uebereinstimmen
  vNum, vMin, vMax, vAvg, vAvI, vVar, vDev, vCNT, vCPT, vCPI, vCQL,
  numValIndexTypes
};


// Tabelle aller instanziierten Sensoren
// -------------------------------------

struct SensorDef
{
  char                 name[128];
  PESensor*            sensor;
  int                  type;
  const PDStateTable * stateTable; // nur fuer PESStateFrequency
};

class SensorTable
{
  public:

    SCBoolean Add(const SensorDef& Sensor);
    SCBoolean Get(SensorDef& Sensor);   
   
//  private:
    struct TEntry //: public TSLinkTo<TEntry>
    {
      TEntry(const SensorDef& Sensor) : sensor(Sensor) {}	
      SensorDef sensor;				    
      friend SCStream& operator<< (SCStream& pStream,
                                   const TEntry& pData);
    };
    typedef SCListIter<TEntry> TEntryIter;
    SCList<TEntry> entryList;
};

class DataTypeTable
{
  public:
    
    void Add(const PESensor* Sensor, int ValIndex, PDDataType* Data);
    SCBoolean Get(const PESensor* Sensor, int ValIndex, PDDataType*& Data);
    
//  private:
    struct TEntry //: public TSLinkTo<TEntry>
    {
      TEntry(PDDataType* DataType, const PESensor* Sensor, int ValIndex)
      :  dataType(DataType), sensor(Sensor), valIndex(ValIndex) {};
      
      PDDataType*     dataType;
      const PESensor* sensor;   // Schlüssel
      int             valIndex; // Schlüssel
      friend SCStream& operator<< (SCStream& pStream,
                                   const TEntry& pData);
    };
    SCList<TEntry> entryList;
    typedef SCListIter<TEntry> TEntryIter;
};

