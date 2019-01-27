/******************************************************************************\
 Datei : PVMapper.cpp
 Inhalt: Implementierung der Klasse PVMapper
 Autor : Christian Rodemeyer, Marc Diefenbruch
 Datum : 06.09.95
 Status: 
\******************************************************************************/   

#include "PVMapper.h"

#if _SC_DMALLOC
  #include <dmalloc.h>
#endif

/******************************************************************************\
 PVMapper Implementierung
\******************************************************************************/   

PVMapper::PVMapper(void)
{
  orgPos    = 0;
  orgPixel  = 0;
  distPos   = 1;
  distPixel = 1;
  scaling   = 1;
}

void PVMapper::SetOrgPos(double OrgPos)
{
  orgPos = OrgPos;
}
    
void PVMapper:: SetOrgPixel(int OrgPixel)
{
  orgPixel = OrgPixel;
}
   
void PVMapper::SetDistPos(double DistPos)
{
  distPos = DistPos;
  scaling = distPixel / distPos;     
}

void PVMapper::SetDistPixel(int DistPixel)
{
  distPixel = DistPixel;
  scaling = distPixel / distPos;
}  
