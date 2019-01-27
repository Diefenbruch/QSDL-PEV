/******************************************************************************\
 Datei : PCUpdater.h
 Inhalt: Deklaration  
 Autor : Christian Rodemeyer, Marc Diefenbruch
 Datum : 10.10.98
 Status: 
\******************************************************************************/   

#ifndef __PCUPDATER_H
#define __PCUPDATER_H

#ifndef __PESENSOR_H
#include "PESensor.h"
#endif
#ifndef __PDDATATYPE_H
#include "PDDataType.h"
#endif

/******************************************************************************\
 PCUpdater: Abstrakte Basisklasse einer Verbindung zwischen einem Sensor und
   einem Speicher zur Aufbewahrung der ermittelten Meﬂwerte. Ein Speicher kann
   z.B. eine Zeitreihe von Meﬂwerten sein, die in einem Graphen gezeigt wird.
\******************************************************************************/  

class PCUpdater // : public TSLinkTo<PCUpdater>
{
  public:
    virtual void Update(void) = 0; // Snapshotaktualisierung

    friend SCStream& operator<< (SCStream& pStream,
                                 const PCUpdater& pData);
};    

/******************************************************************************\
 PCCurveUpdater: Transport von Daten zu einem PDCurve-Datenbeh‰lter.  
\******************************************************************************/  

class PCCurveUpdater: public PCUpdater
{
  public:
    PCCurveUpdater(PDCurve* Curve,
                   const PESensor* Sensor,
                   int ValIndex);
    
    void Update(void);
    
  private:
    PDCurve*        curve;
    const PESensor* sensor;
    int             valIndex;
};


/******************************************************************************\
 PCFreqUpdater: Transport von Daten 
\******************************************************************************/

class PCFreqUpdater: public PCUpdater
{
  public:
    PCFreqUpdater(PDFrequency* Frequency,
                  const PESensor* Sensor);
    
    void Update(void);
    
  private:  
    PDFrequency *    freq;
    const PESensor * sensor;
};

#endif
