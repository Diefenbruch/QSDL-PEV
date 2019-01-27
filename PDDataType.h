/******************************************************************************\
 Datei : PDDataType.h
 Inhalt: Deklaration der Klassen PDDataType, PDPoint, PDCurve, PDCurveIter,
         PDDiscretePoint, PDFrequency, PDStateTable, PDEventType
 Autor : Christian Rodemeyer, Marc Diefenbruch
 Datum : 10.02.98
 Status: 
\******************************************************************************/   

#ifndef __PDDATATYPE_H
#define __PDDATATYPE_H

#include <SCL/SCList.h>

/******************************************************************************\ 
 PDDataType: Basisklasse zur automatischen Speicherfreigabe aller 
   PEV-Datentypen bei Programmende. 
\******************************************************************************/    

class PDDataType
{
  public:
    PDDataType(const long dataColor = 1L);
    virtual ~PDDataType(void);

    void DisableAutoDelete(void); 

    long GetColor(void) const {return color;}  
    void SetColor(const long newColor) { color = newColor;}  
    
    friend SCStream& operator<< (SCStream& pStream,
                                 const PDDataType& pData);

  private:
    static SCList<PDDataType> collection;
    
    SCListCons<PDDataType> *  container;      // actual SCListCons elem

    long      color;
};


/******************************************************************************\ 
 PDPoint, Repräsentation eines Punktes im Rohformat.
\******************************************************************************/    

struct PDPoint
{
  double x, y; // Koordinate
};    


/******************************************************************************\ 
 PDCurve, Repräsentation einer 2D-Kurve als einer Menge von Punkten.
   Die Punkte Menge wird als Queue-Struktur verwaltet. Neue Punkte verdrängen 
   die ältesten Punkte, sobald die Kurve mehr als 'maxPoints' Punkte enthält.
\******************************************************************************/    

class PDCurve: public PDDataType
{
  friend class PDCurveIter; // Der Iterator ueber die Punktemenge
  
  public:
    PDCurve(int MaxPoints = 32, const long dataColor = 1L);
    virtual ~PDCurve(void);
    
    virtual void AddPoint(double X, double Y); // Anhaengen eines neuen Punkts

    void GetMinMax(PDPoint& Min, PDPoint& Max) const;
    int  GetNumPoints() const {return numPoints;}
    
    const int maxPoints;  // Maximale Anzahl von Punkten der Kurve
    
  protected:
    int       numPoints;  // Aktuelle Anzahl Punkte, aus der die Kurve besteht
    int       first;      // Index des ersten Punktes 
    int       last;       // Index des letzten Punktes 
    PDPoint*  point;      // numPoints PVPoints
};


/******************************************************************************\ 
 PDCurveIter, einmaliges Iterieren über alle Punkte eines PVCurve-Objekts.
\******************************************************************************/    

class PDCurveIter
{
  public:
    PDCurveIter(const PDCurve& ToIter);
    PDCurveIter(const PDCurve* ToIter);

    void     operator ++(int)      {toVisit--; if (++cur == curve.maxPoints) cur = 0;}
             operator bool() const {return toVisit;}
    PDPoint* operator ->() const   {return &curve.point[cur];}
    
  private:
    const PDCurve& curve;   // Kurve über die iteriert wird
    int            cur;     // aktueller Punkt
    int            toVisit; // noch zu besuchende Punkte
};    


/******************************************************************************\ 
 PDDiscreteCurve: Kurve, mit diskreten Wertewechseln
\******************************************************************************/    

class PDDiscreteCurve: public PDCurve
{
  public:
    PDDiscreteCurve(int MaxPoints, const long dataColor = 1L);

    virtual void AddPoint(double X, double Y); // Anhaengen eines neuen Punkts
};


/******************************************************************************\
 PDFrequency: Dynamisches Array von double-Werten als Basis von Balkendiagrammen,
   jeder Eintrag entspricht der Hoehe eines Balken.
\******************************************************************************/   

class PDFrequency: public PDDataType 
{
  public:
    PDFrequency(int MinNum = 1, const long dataColor = 1L); // enthaelt mindestens MinNum Elemente
    ~PDFrequency();
    
    void   Reset(void);
    void   ChangeVal(int Index, double Change);
    void   Copy(const PDFrequency& From);
    double GetRelVal(int Index) const;
    double GetAbsVal(int Index) const           {return data[Index];}
    int    Num(void) const                      {return num;}
    
  private:
    double* data;
    double  sum;
    int     num;
};


/******************************************************************************\
 PDEventType: Einfaches Ereigniss, Ankuft oder Abgang eines Signals (Requests) an
  einem Prozeß (einer Maschine).
\******************************************************************************/  

class PDEventType: public PDDataType
{
  public:

  enum                // Benannte Konstanten zur lesbaren Programmierung
  {
    arrival = true, 
    exit    = false,
    signal  = true,
    request = false
  };

  PDEventType(SCBoolean IsArrival, SCBoolean IsSignal,
              char * SigReq = NULL, char * ProcMach = NULL);
  ~PDEventType(void);

  SCBoolean   isArrival;    // true => Ankunft, sonst Abgang
  SCBoolean   isSignal;     // true => Ereigniss betrifft Signal, sonst Request
  char *      nameSigReq;   // Name des Signals/Requests
  char *      nameProcMach; // Name des Prozeßes/der Maschine
};

/******************************************************************************\
 PDStateTable: Array von, State-IDs der SCL um Namen der Zustaende ermitteln
               zu koennen (Beschriftung in Diagrammen).
\******************************************************************************/

class PDStateTable
{
  public:
    PDStateTable(void);
    ~PDStateTable(void);
              
    const char * GetStateName(int Index) const;
    void         RegisterState(const class SCStateType *state);
    SCInteger    GetMaxStateID(void) const { return maxStateID; }
    SCInteger    GetMinStateID(void) const { return minStateID; }
    SCNatural    GetNumOfStates(void) const { return (maxStateID >= minStateID ?
                                              (maxStateID - minStateID + 1):
                                              0); }
    void         Reset(void) { maxStateID = -1;
                               minStateID = LONG_MAX; }
                                              
    friend SCStream& operator<< (SCStream& pStream,
                                 const PDStateTable& pData);

  private:
    SCInteger    maxStateID;
    SCInteger    minStateID;
};
                                    
#endif
