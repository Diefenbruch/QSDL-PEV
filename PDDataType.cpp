/******************************************************************************\
 Datei : PDataType.cpp
 Inhalt: Implementierung der Klassen PDDataType, PDPoint, PDCurve, PDCurveIter
         und PDArray.
 Autor : Christian Rodemeyer, Marc Diefenbruch
 Datum : 10.10.98
 Status: Komplett und geprüft.
         Teilweise umgeschrieben von MD
\******************************************************************************/   

#include <SCL/SCEnvironment.h>
#include <SCL/SCStateType.h>

#include <stdio.h>
#include <iostream>
#include <string.h>

#include "PDDataType.h"

#if _SC_DMALLOC
  #include <dmalloc.h>
#endif

/******************************************************************************\
 PDDataType: Implementierung  
\******************************************************************************/

SCList<PDDataType> PDDataType::collection;

PDDataType::PDDataType(const long dataColor) :
  color(dataColor)
{
  container = collection.InsertAfter(this);
}


PDDataType::~PDDataType(void)
{
  collection.Remove(container);
}  


void PDDataType::DisableAutoDelete(void)
{
  collection.SetDeleteElems(false);
}


SCStream& operator<< (SCStream& pStream, const PDDataType&)
{
  return pStream;
}


/******************************************************************************\
 PVCurve: Implementierung  
\******************************************************************************/

PDCurve::PDCurve(int MaxPoints, const long dataColor) :
  PDDataType(dataColor),
  maxPoints (MaxPoints),
  numPoints (0),
  first     (0),
  last      (-1)
{
  assert(MaxPoints > 4);

  point = new PDPoint[maxPoints];
}  

PDCurve::~PDCurve(void)
{
  delete[] point;
  point = NULL;
}


void PDCurve::AddPoint(double X, double Y)
{
  if (numPoints == maxPoints)
  {
    if (!((last > 1) && (point[last].y == Y) && (point[last - 1].y == Y)))
    {
      if (++first == maxPoints) first = 0; 
      if (++last  == maxPoints) last = 0;
    }
  }
  else
  {
    last = numPoints++;
  }  
  point[last].x = X;
  point[last].y = Y;
}


void PDCurve::GetMinMax(PDPoint& Min, PDPoint& Max) const
{
  Min.x = Max.x = point[0].x;
  Min.y = Max.y = point[0].y;

  for (int i = numPoints; i--;)
  {
    if (point[i].x < Min.x) Min.x = point[i].x;
    if (point[i].y < Min.y) Min.y = point[i].y;
    if (point[i].x > Max.x) Max.x = point[i].x;
    if (point[i].y > Max.y) Max.y = point[i].y;
  }
}


/******************************************************************************\
 PDCurveIter: Implementierung  
\******************************************************************************/

PDCurveIter::PDCurveIter(const PDCurve& ToIter) :
  curve(ToIter)
{
  cur = curve.first;
  toVisit = curve.numPoints;
}  

PDCurveIter::PDCurveIter(const PDCurve* ToIter) :
  curve(*ToIter)
{
  cur = curve.first;
  toVisit = curve.numPoints;
}


/******************************************************************************\
 PDDiscreteCurve: Implementierung
\******************************************************************************/   

PDDiscreteCurve::PDDiscreteCurve(int MaxPoints, const long dataColor) :
  PDCurve (MaxPoints * 2 - 1, dataColor)
{
}


void PDDiscreteCurve::AddPoint(double X, double Y)
{
  if (GetNumPoints())
  {
    PDCurve::AddPoint(X, point[last].y);
  }  
  PDCurve::AddPoint(X, Y);  
}


/******************************************************************************\
 PDFrequency: Implementierung
\******************************************************************************/   

PDFrequency::PDFrequency(int MinNum, const long dataColor) :
  PDDataType(dataColor)
{
  num = MinNum;
  data = new double[num];
  Reset();
}


PDFrequency::~PDFrequency(void)
{
  delete[] data;
  data = NULL;
}


void PDFrequency::Reset(void)
{
  for (int i = num; i--;)
    data[i] = 0.0;
  sum = 0.0;
}


void PDFrequency::ChangeVal(int Index, double Change)
{
  //assert(Index >= 0);
  if (Index >= num)
  {
    double* OldData = data;
    data = new double[Index + 1];
    memcpy(data, OldData, num * sizeof(double));
    for (int i = num; i <= Index; i++) data[i] = 0.0;
    num = Index + 1;
    delete[] OldData;
  }
  data[Index] += Change;
  sum += Change;
}


double PDFrequency::GetRelVal(int Index) const
{
  return sum ? (data[Index] / sum) : 0;
}


void PDFrequency::Copy(const PDFrequency& From)
{
  if (num != From.num)
  {
    delete[] data;
    num = From.num;
    data = new double[num];
  }
  memcpy(data, From.data, num * sizeof(double));
  sum = From.sum;
}


/******************************************************************************\
 PDStateTable: Implementierung
\******************************************************************************/   

PDStateTable::PDStateTable(void) :
  maxStateID (-1),
  minStateID (LONG_MAX)  
{
}


PDStateTable::~PDStateTable(void)
{
}


const char * PDStateTable::GetStateName(int Index) const
{
  if (Index > maxStateID || Index < minStateID)
    return NULL;

  return SCType::GetObjectName(SC_STATE, Index);
} 


void PDStateTable::RegisterState(const SCStateType *state)
{
  if ((SCInteger)state->GetID() > maxStateID)
  {
    maxStateID = state->GetID();
  }

  if ((SCInteger)state->GetID() < minStateID)
  {
    minStateID = state->GetID();
  }
}


SCStream& operator<< (SCStream& pStream, const PDStateTable&)
{
  return pStream;
}


/******************************************************************************\
 PDEventType: Implementierung
\******************************************************************************/   

PDEventType::PDEventType(SCBoolean IsArrival,
                         SCBoolean IsSignal,
                         char *    SigReq,
                         char *    ProcMach) :
  isArrival    (IsArrival),
  isSignal     (IsSignal),
  nameSigReq   (SigReq),
  nameProcMach (ProcMach)
{
}


PDEventType::~PDEventType(void)
{
  if (nameSigReq)
  {
    delete nameSigReq;
    nameSigReq = NULL;
  }
  if (nameProcMach)
  {
    delete nameProcMach;
    nameProcMach = NULL;
  }
}

