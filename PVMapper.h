/******************************************************************************\
 Datei : PVMapper.h
 Inhalt: Deklaration der Mapping-Klasse fuer skalierbare Grafiken: PVMapper
 Autor : Christian Rodemeyer, Marc Diefenbruch
 Datum : 30.09.95
 Status: 
\******************************************************************************/   

#ifndef __PVMAPPER_H
#define __PVMAPPER_H

/******************************************************************************\
 PVMapper: Ermoeglicht die Abbildung eines virtuellen Koordinatensystems auf
  das Pixel-Koordinatensystem von X-Windows (z.B. fuer Skalierung)
\******************************************************************************/   

class PVMapper
{
  public:
    PVMapper(void);
    
    int operator ()(double Pos) const  // bildet Position auf ein Pixel ab
      {return int((Pos - orgPos) * scaling + orgPixel);} 
      
    double ReMap(int Pixel) const // Umkehrfunktion zu operator()
      {return (Pixel - orgPixel) / scaling + orgPos;}
      
    void SetOrgPos(double OrgPos);    
    void SetOrgPixel(int OrgPixel);   
    void SetDistPos(double DistPos);     
    void SetDistPixel(int DistPixel);
     
  private:
    double orgPos;     // virtueller Ursprung, der auf 
    int    orgPixel;   // Ursprung im GC abgebildet wird
    double distPos;    // virtueller Abstand, der
    int    distPixel;  // auf Pixel-Abstand abgebildet wird
    double scaling;    // Berechneter Skalierungsfaktor
};

#endif 
