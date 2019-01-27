/******************************************************************************\
 Datei : PESensor.cpp
 Inhalt: Implementierung der in PESensor.h deklarierten Dateien
 Autor : Christian Rodemeyer, Marc Diefenbruch
 Datum : 10.02.98
 Status: Komplett umgeschrieben (MD)
\******************************************************************************/   

#include <limits.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <iomanip>

#include "PESensor.h"

#if _SC_DMALLOC
  #include <dmalloc.h>
#endif

using namespace std;

/******************************************************************************\
 PESensor: Implementierung
\******************************************************************************/

void PESensor::Underline(SCStream& Out, int Len) const 
{
  Out << std::endl;
  for (int i = Len; i--;) Out.GetStream().put('=');
  Out << std::endl;
}


SCStream& operator<< (SCStream& pStream, const PESensor&)
{
  return pStream;
}


/******************************************************************************\
 PESTally: Implementierung
\******************************************************************************/

PESTally::PESTally(SCDuration IntervalLength) :
  intervalLen(IntervalLength)
{
  Reset();
}


void PESTally::Reset(void)
{
  minS = maxS = sumS = sumS2 = numS = 0.0;
  intervalStop = Now() + intervalLen;
  intervalAvg = intervalSum = intervalNum = 0.0;
}


void PESTally::Report(SCStream& Out) const
{ 
  Out.GetStream().setf(ios::left, ios::adjustfield|ios::floatfield);
  Out.GetStream().precision(5);
  Out.GetStream() << "  Min         Max         Avg         Var         Dev\n  "
      << std::setw(12) << GetValue(min) 
      << std::setw(12) << GetValue(max) 
      << std::setw(12) << GetValue(avg) 
      << std::setw(12) << GetValue(var) 
      << std::setw(12) << GetValue(dev) 
      << "\n\n";	
}


void PESTally::UpdateTally(double Sample, double Weight)
{
  if (numS == 0.0)
  {
    minS = maxS = Sample; 
  }
  else
  {
    if (Sample < minS) minS = Sample;
    else if (Sample > maxS) maxS = Sample;
  }
  numS += Weight;
  Sample *= Weight;
  sumS += Sample;
  sumS2 += Sample * Sample; 

  while (Now() >= intervalStop)
  {
    intervalAvg = (intervalNum > 0.0) ? (intervalSum / intervalNum) : 0.0;
    intervalNum = intervalSum = 0.0;
    intervalStop += intervalLen;
  }
  intervalSum += Sample;
  intervalNum += Weight;
}


double PESTally::GetValue(int ValIndex) const
{
  switch (ValIndex)
  {  
    case num: 
      return numS;          

    case min: 
      return numS ? minS : 0;
      
    case max: 
      return  numS ? maxS : 0;
      
    case avg: 
      return numS ? double(sumS / numS) : 0;

    case avi:
      return intervalAvg;

    case var:
      if (numS <= 1)
        return 0;
      else
        return double((sumS2 - ((sumS * sumS) /  numS)) / ( numS + 1));

    case dev:
      {
        double Var = GetValue(var); 
        return (!numS || Var <= 0) ? 0 : sqrt(Var);
      }

    default:
      std::cout << "Illegal ValIndex in Tally!\n"; abort();
  }
}


/******************************************************************************\
 PESCounter: Implementierung
\******************************************************************************/

PESCounter::PESCounter(SCDuration Interval) :    
  creation       (0),
  intervalLength (Interval)
{
  Reset();
}


void PESCounter::Reset(void)
{
  count = 0;
  intervalCount = 0;
  creation = Now();
  intervalStop = Now() + intervalLength;
  countsPerInterval = 0;
}


void PESCounter::Report(SCStream& Out) const
{
  Out.GetStream().setf(ios::left, ios::adjustfield|ios::floatfield);
  Out.GetStream().precision(5);
  Out << "  Counts = " << count 
      << ", CountsPerTime = " << (count / (Now() - creation))
      << "\n\n";
}


double PESCounter::GetValue(int ValIndex) const
{
  switch (ValIndex)
  {
    case cnt: return count;
    case cpt: return (Now() > creation) ? (count / (Now() - creation)) : 0;
    case cpi: return countsPerInterval;
    default : std::cout << "Illegal ValIndex in Counter!\n"; abort();
  }
}


void PESCounter::UpdateCounter(void)
{
  count++;
  while (Now() >= intervalStop)
  {
    countsPerInterval = intervalCount / intervalLength;
    intervalCount = 0;
    intervalStop += intervalLength;
  }
  intervalCount++;
}


/******************************************************************************\
 PESFrequency: Implementierung
\******************************************************************************/

PESFrequency::PESFrequency(SCObjectType ObjectType) :
  objectType(ObjectType)
{
//  freq.DisableAutoDelete();
}


void PESFrequency::Reset(void)
{
  freq.Reset();
} 


double PESFrequency::GetValue(int Index) const
{
  return freq.GetRelVal(Index);
}


const PDFrequency& PESFrequency::GetFrequency(void) const // wird von PCUpdater aufgerufen
{
  return freq;
} 

    
void PESFrequency::UpdateFreq(int i, double Diff)
{
  freq.ChangeVal(i, Diff);
}


void PESFrequency::Report(SCStream& Out) const
{
  Out << "  Frequency of | Relative | Absolute\n"
      << "  -------------+----------+---------\n  ";
  Out.GetStream().setf(ios::fixed|ios::right, ios::adjustfield|ios::floatfield);
  Out.GetStream().precision(2);

  for (int i = 0; i < freq.Num(); i++)
  {
    if (freq.GetAbsVal(i) != 0)
    {
      if (strlen(SCType::GetObjectName(objectType, i)) > 12)
      {
        Out.GetStream().write(SCType::GetObjectName(objectType, i), 12);
      }
      else
      {
        Out << SCType::GetObjectName(objectType, i);
        for (int j = 12 - strlen(SCType::GetObjectName(objectType, i)); j--;) Out.GetStream().put(' ');
      }
      Out.GetStream() << " | "
	                    << std::setw(6)
	                    << (freq.GetRelVal(i) * 100) << "%  |"
	                    << std::setw(10) << freq.GetAbsVal(i)
	                    << "\n  ";
    }
  }
  Out << std::endl;
}


/******************************************************************************\
 PESQueue: Implementierung
\******************************************************************************/

PESQueue::PESQueue(void) :
  qLen (0)
{
  PESQueue::Reset();
}


void PESQueue::Reset(void)
{
  lastUpdate = Now();
}


void PESQueue::UpdateQLen(int QLenDiff)
{
  lastUpdate = Now();
  qLen += QLenDiff;
}


/******************************************************************************\
 PESQueueLength: Implementierung
\******************************************************************************/

void PESQueueLength::Reset()
{
  PESQueue::Reset();
  PESTally::Reset();
}

double PESQueueLength::GetValue(int ValIndex) const
{
  if (ValIndex == cql) return QLen();
  else                 return PESTally::GetValue(ValIndex);
}

void PESQueueLength::UpdateQLen(int QLenDiff)
{
  UpdateTally(QLen(), Duration());
  PESQueue::UpdateQLen(QLenDiff);
}


/******************************************************************************\
 PESQueueLengthFrequency: Implementierung
\******************************************************************************/

int PESQueueLengthFrequency::maxNumber = -1;

PESQueueLengthFrequency::PESQueueLengthFrequency(void) :
  PESFrequency (SC_NONE)
{
}


void PESQueueLengthFrequency::Reset(void)
{
  PESQueue::Reset();
  PESFrequency::Reset();
}


void PESQueueLengthFrequency::UpdateQLen(int QLenDiff)
{
  UpdateFreq(QLen(), Duration());
  PESQueue::UpdateQLen(QLenDiff);	     
  if (QLen() > maxNumber)
  {
    maxNumber = QLen();
  }
}
