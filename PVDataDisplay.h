/******************************************************************************\
 Datei : PVDataDisplay.h
 Inhalt: Displays zur Darstellung von Kurven und Staebchendiagrammen.
         Deklaration der Klassen PVCurvesFrame (PVCurvesDisplay),
	 PVFreqFrame (PVFreqDisplay) und PVGanttFrame
 Autor : Christian Rodemeyer, Marc Diefenbruch
 Datum : 25.10.95
 Status: Implementierung der Achsenbeschriftung!
\******************************************************************************/   

#ifndef __PVDATADISPLAY_H
#define __PVDATADISPLAY_H

#include <SCL/SCList.h>
#include <SCL/SCListIter.h>

#ifndef __PVDISPLAY_H
#include "PVDisplay.h"
#endif
#ifndef __PVMAPPER_H
#include "PVMapper.h"
#endif
#ifndef __PDDATATYPE_H
#include "PDDataType.h"
#endif

/******************************************************************************\ 
 PVDataDisplay: Typisiertes SubDisplay zur Anzeige von Datentypen.
\******************************************************************************/    

template<class T>
class PVDataDisplay: public PVSubDisplay
{ 
  public:
    
    void AddDataType(T * ToAdd) 
    {
      dataList.InsertAfter(ToAdd);
    }

    typedef SCListIter<T> DataIter;
  
    const SCList<T> &  GetDataList(void) const { return dataList; }
    
  protected:
    PVDataDisplay(Display* XDisplay) : PVSubDisplay(XDisplay), dataList(false) {}

    SCList<T>     dataList;
};


/******************************************************************************\ 
 PVDataFrameDisplay: Typisiertes Rahmendisplay eines PVDataDisplays
\******************************************************************************/    

template<class T>
class PVDataFrame: public PVFrameDisplay
{
  public:  
    PVDataFrame(Display* XDisplay,
                PVDataDisplay<T>* Sub,
                const char * Name) :
      PVFrameDisplay(XDisplay, Sub, Name) {}
    
    void AddDataType(T * ToAdd) 
    {
      ((PVDataDisplay<T>*)sub)->AddDataType(ToAdd);
    }
};


/******************************************************************************\ 
 PVCurvesFrame: Rahmenfenster zur Darstellung von 2D-Kurven. 
\******************************************************************************/    

class PVCurvesDisplay;

class PVCurvesFrame: public PVDataFrame<PDCurve>
{
  public:
    PVCurvesFrame(Display*      XDisplay, 
                  const char *  Name,
                  int           AdjustRange,
                  SCBoolean     FixedBottom = false);

    // Redefinierte virtuelle Funktionen
    // --------------------------------
    void Update(void);  
    
  protected:  
    PVCurvesFrame(Display*         XDisplay, 
                  const char *     Name,
                  PVCurvesDisplay* DataDisplay);
    
    void DrawXAxis(void);  
    void DrawYAxis(void);
     
  private:  
    PVCurvesDisplay * GetCurvesDisplay(void) const;    
};    


/******************************************************************************\ 
 PVCurvesDisplay: Private Subklasse fuer PVCurvesDisplay
\******************************************************************************/    

class PVCurvesDisplay: public PVDataDisplay<PDCurve>
{
  public: 

  // Redefinierte Funktionen eines Displays
  // --------------------------------------
  void Paint(void);   // => Skalierung bleibt konstant
  void Resized(void); // => Skalierungsaenderung moeglich
  void Update(void);  // => Skalierungsaenderung moeglich
  
  // Skalierungsinformation
  // ----------------------
  int MapX(double XPos) const {return mapX(XPos);}
  int MapY(double YPos) const {return mapY(YPos);}

  double GetMinX(void) const {return min.x;}
  double GetMaxX(void) const {return max.x;}
  double GetMinY(void) const {return min.y;}
  double GetMaxY(void) const {return max.y;}

  SCBoolean YAxisDirty(void) const {return yAxisDirty;}

protected:  

  PVCurvesDisplay(Display* XDisplay, int AdaptRange, SCBoolean FixedBottom);
  friend PVCurvesFrame::PVCurvesFrame(Display*, const char *, int, SCBoolean);

  // Skalierungsobjekte fuer X und Y Dimension
  // ----------------------------------------
  PVMapper        mapX; // X-Koordinaten Abbildung
  PVMapper        mapY; // Y-Koordinaten Abbildung
  PDPoint         min;  // linke untere Ecke des virtuellen Koordinatensystems
  PDPoint         max;  // obere rechte Ecke des virtuellen Koordinatensystems
  
  // Steuerung der Autoskalierung
  // ----------------------------
  const SCBoolean fixedBottom;
  const int       adaptRange;      // Adaptionsbereich in Prozent (0..100);
  int             leftAdaptRange;  // Linke und 
  int             rightAdaptRange; // rechte Grenze fuer Autoskalierung
  int             lowerBottomAdaptRange;
  int             lowerTopAdaptRange;
  int             upperBottomAdaptRange;
  int             upperTopAdaptRange;
  SCBoolean       yAxisDirty;
};

/******************************************************************************\ 
 PVGanttFrame: Gantt-Diagramme nach [Klar]
\******************************************************************************/    

class PVGanttDisplay;

class PVGanttFrame: public PVCurvesFrame
{
  public:
    PVGanttFrame(Display*      XDisplay, 
                 const char *  Name, 
                 int           AdjustRange);
  
    void AddStateTable(const PDStateTable * StateTable);
    
  protected:
    void DrawYAxis(void);
    
  private:  
    PVGanttDisplay * GetGanttDisplay(void) const;    
};


/******************************************************************************\ 
 PVGanttDisplay: 
\******************************************************************************/    

class PVGanttDisplay: public PVCurvesDisplay
{
    friend class PVGanttFrame;
    
  public: 

    // Redefinierte Funktionen eines Displays
    // --------------------------------------
    void Paint(void);   
    void Update(void);  

  protected:  
    PVGanttDisplay(Display* XDisplay, int AdaptRange);

  private:
    SCList<PDStateTable> stateTableList;
//    typedef SCListIter<PDNameTable> TStateIter;
};


/******************************************************************************\ 
 PVFreqFrame: Rahmen eine Haeufigkeitszaehlers. An der Y-Achse wird die 
   prozentuale Haeufigkeit aufgetragen, an der X-Achse die Name der Typen, deren
   Haeufigkeit ermittelt wurden.
\******************************************************************************/    

class PVFreqDisplay;

class PVFreqFrame: public PVDataFrame<PDFrequency>
{
  public:
    PVFreqFrame(Display* XDisplay,
                const char * Name,
                const SCObjectType ObjectType);
 
    // Redefinierte virtuelle Funktionen
    // ---------------------------------
    void Update(void);

  protected:  
    void DrawXAxis(void);  
    void DrawYAxis(void);
  
  private:   
    const SCObjectType objectType;

    PVFreqDisplay* GetFreqDisplay(void) const; // Cast von sub nach FreqDisplay
};


/******************************************************************************\ 
 PVFreqDisplay:
\******************************************************************************/    

class PVFreqDisplay: public PVDataDisplay<PDFrequency>
{
  public:

    // Redefinierte Funktionen eines Displays
    // --------------------------------------
    void Paint(void);   // => Skalierung bleibt konstant
    void Resized(void); // => Skalierungsaenderung
    void Update(void);  // => moegliche Änderung der Balkenskalierung

    // Einige Funktionen die vor allem fuer PVFreqFrame Display gedacht
    // sind, um die Achsenbeschriftung zu implementieren
    // ---------------------------------------------------------------
    int       GetMaxCol(void) const   { return maxNum; }
    int       GetColWidth(void) const { return beamColWidth; }
    SCBoolean UpdateFrame(void) const { return updateFrame; }
    int       MapY(double p) const    { return mapY(p); }
    
    PDFrequency * GetDataType (void);

  protected:
  
    PVFreqDisplay(Display* XDisplay);
    ~PVFreqDisplay(void);
    friend PVFreqFrame::PVFreqFrame(Display*, const char *, const SCObjectType);

  private:
    const int wideTxWidth;  // Prozent Textbreite im Langformat
    const int smallTxWidth; // Prozent Textbreite im Kurzformat
    const int txHeight;     // Texthoehe, fuer beide Formate
        
    SCBoolean updateFrame; // true, wenn Frame sich neuzeichnen muss
  
    // Variablen zur X-Skalierung
    // ---------------------------
    enum
    {
      colDist = 5,  // Pixel-Abstand zwischen Balkenspalten
      beamDist = 2  // Pixel-Abstand zwischen Balken in Spalten
    };	  
    int         numFreqs;     // Anzahl darzustellender Haeufigkeitsobjekte
    int         maxNum;       // Max Num() aller Freq-Objekte
    int         beamWidth;    // Pixel-Breite eines Balkens + 2
    int         beamColWidth; // Pixel-Abstand zwischen Balken eines PDFrequency-Objekts 
    int         numVisible;   // Anzahl sichtbarer Balken
    
    // Y-Skalierung
    // ------------
    PVMapper mapY;
    
    void InitWhiteOut(XRectangle* WhiteOut, int XPos, int YPos, int TxWidth);
};


#endif
