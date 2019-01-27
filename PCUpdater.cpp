/******************************************************************************\
 Datei : PCUpdater.cpp
 Inhalt: 
 Autor : Christian Rodemeyer, Marc Diefenbruch
 Datum : 10.10.98
 Status: Verbessert von MD
\******************************************************************************/   

#include "PCUpdater.h"

#if _SC_DMALLOC
  #include <dmalloc.h>
#endif

SCStream& operator<< (SCStream& pStream, const PCUpdater&)
{
  return pStream;
}

/******************************************************************************\
 PCCurveUpdater: Transport von Daten zu einem PDCurve-Datenbehälter.  
\******************************************************************************/  

PCCurveUpdater::PCCurveUpdater(PDCurve *        Curve,
                               const PESensor * Sensor,
                               int              ValIndex) :
  curve    (Curve),
  sensor   (Sensor),
  valIndex (ValIndex)
{
}

 
void PCCurveUpdater::Update(void)
{
  double Value = sensor->GetValue(valIndex);

  if (Value >= 0)
    curve->AddPoint(sensor->Now(), Value);
}


/******************************************************************************\
 PCFreqUpdater: 
\******************************************************************************/  

PCFreqUpdater::PCFreqUpdater(PDFrequency *    Frequency,
                             const PESensor * Sensor) :
  freq   (Frequency),
  sensor (Sensor)
{
}    

    
void PCFreqUpdater::Update(void)
{
  // hier muss nichts mehr gemacht werden, da nun
  // das PDFrequency-Objekt, welches in Sensor enthalten
  // ist die ganze Arbeit macht. freq ist in Zeiger
  // auf dieses enthaltene Objekt. MD
}
