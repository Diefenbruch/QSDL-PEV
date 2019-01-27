/******************************************************************************\
 Datei : PVDataDisplay.cpp
 Inhalt: Implementierung der Displays zur Anzeige von PDDataType-Objekten:
         
 Autor : Christian Rodemeyer, Marc Diefenbruch
 Datum : 10.10.98
 Status: Komplett umgeschrieben (MD)
\******************************************************************************/   

#include <math.h>
#include <stdio.h>

#include <SCL/SCEnvironment.h>
#include <SCL/SCStateType.h>

#include "PVDataDisplay.h"

#if _SC_DMALLOC
  #include <dmalloc.h>
#endif

/******************************************************************************\
 PVCurvesDisplay: Implementierung  
\******************************************************************************/

PVCurvesDisplay::PVCurvesDisplay(Display* XDisplay,
                                 int AdaptRange,
                                 SCBoolean FixedBottom) :
  PVDataDisplay<PDCurve> (XDisplay), 
  fixedBottom            (FixedBottom),
  adaptRange             (AdaptRange),
  yAxisDirty             (true)  
{
  assert(adaptRange > 5 && adaptRange < 60); // Sinnvolle Bereiche

  mapX.SetOrgPixel(0); 
  mapY.SetOrgPixel(GetHeight() - 1);
  min.x = min.y = max.x = max.y = 0;
}


void PVCurvesDisplay::Resized(void)
{
  PVSubDisplay::Resized();

  rightAdaptRange = GetWidth();
  leftAdaptRange  = GetWidth() * (100 - adaptRange) / 100;

  upperTopAdaptRange    = 1;
  lowerTopAdaptRange    = (GetHeight() * adaptRange) / 100;
  upperBottomAdaptRange = GetHeight() * (100 - adaptRange) / 100;
  lowerBottomAdaptRange = GetHeight();
  
  mapX.SetDistPixel(GetWidth() - 1);
  mapY.SetOrgPixel(GetHeight() - 1); 
  mapY.SetDistPixel(-(GetHeight() - 1));
}


void PVCurvesDisplay::Paint(void)
{
  PDCurve * curve;
  DataIter  iter(dataList);

  PVSubDisplay::Paint();

  for (curve = iter++;
       curve;
       curve = iter++)
  {
    XSetForeground(xDpy, xGC, curve->GetColor());
    if (curve->GetNumPoints() > 3)
    {
      XPoint* xcurve = new XPoint[curve->GetNumPoints()];
      XPoint* xpoint = xcurve;
      
      for (PDCurveIter point(curve); point; point++)
      {
        xpoint->x = mapX(point->x);
        xpoint->y = mapY(point->y);
        xpoint++;
      }
      
      XDrawLines(xDpy, xWin, xGC, xcurve, curve->GetNumPoints(), CoordModeOrigin);
      delete[] xcurve;
    }  
  } 
}


void PVCurvesDisplay::Update(void)
{
  PDCurve * curve;
  DataIter  iter(dataList);

  curve = iter++;
  
  if (curve && curve->GetNumPoints() > 3)  // curven da und genug Punkte?
  {  
    // Ermittle Minimum und Maximum aller Kurven
    // -----------------------------------------    
    PDPoint TMin, TMax; // Temporaere Min/Max Werte

    curve->GetMinMax(TMin, TMax); 

    if (curve->GetNumPoints() < curve->maxPoints) // Approximation des Maximums
    {
      TMax.x = TMin.x + ((TMax.x - TMin.x) * curve->maxPoints) /
                        curve->GetNumPoints();
    }

    while ((curve = iter++))
    {                                // Da die X-Koordinaten fuer alle Kurven
      PDPoint CMin, CMax;            // gleich sind, brauchen nur die Y-Minima
                                     // und Maxima ermittelt werden. 
      curve->GetMinMax(CMin, CMax);
      if (CMin.y < TMin.y) TMin.y = CMin.y;
      if (CMax.y > TMax.y) TMax.y = CMax.y;
      if (CMin.x > TMin.x) TMin.x = CMin.x; // wegen Kurvenkomprimierung erforderlich
    }

    // Autoskalierung
    // --------------
    mapX.SetOrgPos(TMin.x);  
    int XPos = mapX(TMax.x);
    if (XPos < leftAdaptRange || XPos > rightAdaptRange)
    {
      mapX.SetDistPos((TMax.x - TMin.x) * 100 / (100 - adaptRange / 2));  
    }
    min.x = TMin.x;
    max.x = mapX.ReMap(GetWidth() - 1);

    // Sonderfaelle der Y-Skalierung - Patch vom 16.10.95 by CR
    if (fixedBottom) TMin.y = 0.0;
    if (TMax.y == TMin.y)
    {
      TMax.y *= 1.8;
      TMin.y = 0.0;
    }
    
    int YMaxPos = mapY(TMax.y);
    int YMinPos = mapY(TMin.y);

    if ( (YMinPos > lowerBottomAdaptRange) || (YMinPos < upperBottomAdaptRange) ||
         (YMaxPos > lowerTopAdaptRange)    || (YMaxPos < upperTopAdaptRange) )
    {
      double Dist, Org;
       
      Dist = (TMax.y - TMin.y) * 100 / (100 - adaptRange);
      Org  = TMin.y - Dist * (adaptRange / 2) / 100;
      if (Org < 0.0)
      {
        Dist += Org;
        Org  = 0.0;
      }
      if (Dist <= 0.0) Dist = 0.001;
      
      mapY.SetOrgPos(Org);
      mapY.SetDistPos(Dist);
      max.y = mapY.ReMap(0);
      min.y = mapY.ReMap(GetHeight() - 1);
      yAxisDirty = true;
    }
    else
    {
      yAxisDirty = false;
    }
  }
  
  // Neuzeichnen der Kurven
  // ----------------------
  XClearWindow(xDpy, xWin);
  Paint();
}


/******************************************************************************\
 PVCurvesFrame: Implementierung  
\******************************************************************************/

PVCurvesFrame::PVCurvesFrame(Display *    XDisplay, 
                             const char * Name,
                             int          AdjustRange,
                             SCBoolean    FixedBottom) :
  PVDataFrame<PDCurve> (XDisplay,   
                        new PVCurvesDisplay(XDisplay, AdjustRange,
                                            FixedBottom),
                        Name)
{
  SetMargin(XTextWidth(axisFont, "00.000e-00", 10), 16);
}


PVCurvesFrame::PVCurvesFrame(Display *         XDisplay, 
                             const char *      Name,
                             PVCurvesDisplay * DataDisplay) :
  PVDataFrame<PDCurve> (XDisplay, DataDisplay, Name)
{
}


inline PVCurvesDisplay* PVCurvesFrame::GetCurvesDisplay(void) const 
{
  return (PVCurvesDisplay *)sub;
}


void PVCurvesFrame::DrawXAxis(void)
{
  const int    YOffset  = topMargin + subHeight + 6;
  const int    YTxPos   = YOffset + axisFont->ascent + 4;
  const int    YPos1    = YOffset + 2;
  const int    YPos2    = YPos1 + 2;
  const int    XOffset  = leftMargin + 4;
  const double MaxX     = GetCurvesDisplay()->GetMaxX();
  const double MinX     = GetCurvesDisplay()->GetMinX();
  const int    TxWidth  = XTextWidth(axisFont, "00.000e-00", 10) + 4;
  const int    TxFields = subWidth / TxWidth; 

  double MarkDist = -1.0;  // (virtueller) Abstand zwischen Markierung
  double FirstMark = -1.0; // erste Hauptmarkierung
  double x;         // Schleifenvariable fuer MD

  static char Number[32]; // Lieber zuviel als zuwenig

  if (MaxX == MinX) return;
  
  XSetForeground(xDpy, xGC, mForeground);
  if (TxFields != 0)
  {
    double T1 = (MaxX - MinX) / TxFields; 
    if (T1 != 0)
    {
      double T2 = pow(10.0, floor(log10(T1)));
      MarkDist = ceil(T1 / T2) * T2;
      if (MarkDist <= 0) return;
      FirstMark = ceil(MinX / MarkDist) * MarkDist;
      XRectangle XAxis = {leftMargin + 2, YOffset, subWidth + 4, bottomMargin};
      XSetClipRectangles(xDpy, xGC, 0, 0, &XAxis, 1, YXBanded);
      XSetFont(xDpy, xGC, axisFont->fid);
      for (x = FirstMark; x < MaxX; x += MarkDist)
      {
        sprintf(Number, "%.5g", x);
        DrawCenteredString(axisFont, Number, GetCurvesDisplay()->MapX(x) + XOffset, YTxPos);
      }
      XSetClipMask(xDpy, xGC, None);
    } 
  }
  else
  { // keine Beschriftung sichtbar
    MarkDist = (MaxX - MinX) / 4;
    FirstMark = MinX + MarkDist / 4;
  }
  
  // Markierungen zeichnen
  // ---------------------
  int    XPos;
  double MarkOffset = MarkDist / 2;
  
  for (x = FirstMark; x < MaxX; x += MarkDist)
  {
    XPos = GetCurvesDisplay()->MapX(x) + XOffset;
    XDrawLine(xDpy, xWin, xGC, XPos, YOffset, XPos, YPos2);
  }  
  FirstMark += (FirstMark - MarkOffset < MinX) ? MarkOffset : -MarkOffset;
  for (x = FirstMark; x < MaxX; x += MarkDist)
  {
    XPos = GetCurvesDisplay()->MapX(x) + XOffset;
    XDrawLine(xDpy, xWin, xGC, XPos, YOffset, XPos, YPos1);
  }  
}

void PVCurvesFrame::DrawYAxis(void)
{
  const int    XOffset  = leftMargin + 1;
  const int    XTxPos   = XOffset - 5; 
  const int    XPos1    = XOffset - 4;
  const int    XPos2    = XOffset - 2;
  const int    YOffset  = topMargin + 4;
  const int    YTxPos   = YOffset + axisFont->ascent / 2;
  const double MaxY     = GetCurvesDisplay()->GetMaxY();
  const double MinY     = GetCurvesDisplay()->GetMinY();
  const int    TxHeight = axisFont->ascent + axisFont->descent + 10;
  const int    TxFields = subHeight / TxHeight;

  double MarkDist = -1.0;  // (virtueller) Abstand zwischen Markierung
  double FirstMark = -1.0; // erste Hauptmarkierung
  double y;                // Schleifenvariable fuer MD
  static char Number[32];  // Lieber zuviel als zuwenig

  if (MaxY == MinY) return;
  
  XSetForeground(xDpy, xGC, mForeground);
  if (TxFields != 0)
  {
    double T1 = (MaxY - MinY) / TxFields; 
    if (T1 != 0)
    { 
      double T2 = pow(10.0, floor(log10(T1)));
      MarkDist = ceil(T1 / T2) * T2;
      if (MarkDist <= 0) return;
      FirstMark = ceil(MinY / MarkDist) * MarkDist;
      XSetFont(xDpy, xGC, axisFont->fid);
      for (y = FirstMark; y < MaxY; y += MarkDist)
      {
      	sprintf(Number, "%.5g", y);
        DrawRightString(axisFont, Number, XTxPos,
                        GetCurvesDisplay()->MapY(y) + YTxPos);
      }
    } 
  }
  else
  { // keine Beschriftung sichtbar
    MarkDist = (MaxY - MinY) / 4;
    FirstMark = MinY + MarkDist / 4;
  }

  // Markierungen zeichnen
  // ---------------------
  int    YPos;
  double MarkOffset = MarkDist / 2;
 
  for (y = FirstMark; y < MaxY; y += MarkDist)
  {
    YPos = GetCurvesDisplay()->MapY(y) + YOffset;
    XDrawLine(xDpy, xWin, xGC, XOffset, YPos, XPos1, YPos);
  }
  FirstMark += (FirstMark - MarkOffset < MinY) ? MarkOffset : -MarkOffset;
  for (y = FirstMark; y < MaxY; y += MarkDist)
  {
    YPos = GetCurvesDisplay()->MapY(y) + YOffset;
    XDrawLine(xDpy, xWin, xGC, XOffset, YPos, XPos2, YPos);
  }
}


void PVCurvesFrame::Update(void)
{
  // X-Achse muss immer geloescht und neu gezeichnet werden

  XClearArea(xDpy, xWin, 
             leftMargin + 2, topMargin + subHeight + 6,
             subWidth + 4, bottomMargin,
             false);
  DrawXAxis();
  if (GetCurvesDisplay()->YAxisDirty())  // die Y-Achse nur bei Bedarf
  {
    XClearArea(xDpy, xWin,
               2, 2, leftMargin, GetHeight() - 4,
               false);
    DrawYAxis();
  }
}


/******************************************************************************\
 PVGanttDisplay: Implementierung  
\******************************************************************************/

PVGanttDisplay::PVGanttDisplay(Display* XDisplay, int AdaptRange) : 
  PVCurvesDisplay (XDisplay, AdaptRange, true)
{
}


void PVGanttDisplay::Paint(void)
{
  double                   Offset = 0.0;
  PDCurve *                curve;
  DataIter                 citer(dataList);
  SCListIter<PDStateTable> siter(stateTableList);
  PDStateTable *           stateTable;
  
  PVSubDisplay::Paint();
  
  for (curve = citer++, stateTable = siter++;
       curve && stateTable;
       curve = citer++, stateTable = siter++)
  {
    XSetForeground(xDpy, xGC, curve->GetColor());
    if (curve->GetNumPoints() > 3)
    {
      XPoint* xcurve = new XPoint[curve->GetNumPoints()];
      XPoint* xpoint = xcurve;
      for (PDCurveIter point(curve); point; point++)
      {
        xpoint->x = mapX(point->x);
        xpoint->y = mapY(point->y + Offset);
        xpoint++;
      }
      XDrawLines(xDpy, xWin, xGC, xcurve, curve->GetNumPoints(), CoordModeOrigin);
      delete[] xcurve;
    } 
    Offset += stateTable->GetNumOfStates();
  }
}


void PVGanttDisplay::Update(void)
{
  PDCurve *                curve;
  DataIter                 citer(dataList);
  SCListIter<PDStateTable> siter(stateTableList);
  PDStateTable *           stateTable;

  curve = citer++;
       
  if (curve && curve->GetNumPoints() > 3)  // Falls ueberhaupt Kurven existieren
  {  
    // Ermittle Minimum und Maximum aller Kurven
    // -----------------------------------------
    PDPoint TMin, TMax; // Temporaere Min/Max Werte

    curve->GetMinMax(TMin, TMax);

    while ((curve = citer++))
    {
      PDPoint CMin, CMax;

      curve->GetMinMax(CMin, CMax); 
      if (CMin.x > TMin.x) TMin.x = CMin.x;
    }

    citer.GoToFirst();
    curve = citer++;

    if (curve->GetNumPoints() < curve->maxPoints) // Approximation des Maximums
    {
      TMax.x = TMin.x + ((TMax.x - TMin.x) * curve->maxPoints) /
                        curve->GetNumPoints();
    }

    TMin.y = TMax.y = 0;

    for (stateTable = siter++;
         stateTable;
         stateTable = siter++)
    {
      TMax.y += stateTable->GetNumOfStates();
    }

    // Autoskalierung
    // --------------
    mapX.SetOrgPos(TMin.x);  
    int XPos = mapX(TMax.x);
    if (XPos < leftAdaptRange || XPos > rightAdaptRange)
    {
      mapX.SetDistPos((TMax.x - TMin.x) * 100 / (100 - adaptRange / 2));  
    }
    min.x = TMin.x;
    max.x = mapX.ReMap(GetWidth() - 1);

    // Eigentlich muss immer dann ein Update durchgefuehrt werden, wenn ein
    // neuer Zustandsname entdeckt wird. Leider fehlt die Zeit fuer eine 
    // Routine, die diese Situation entdecken kann.

    if (TMax.y > max.y) 
    {
      max.y = TMax.y;
      mapY.SetDistPos(max.y);
      yAxisDirty = true;
    }
    else
    {
      yAxisDirty = false;
    }
  }
  
  // Neuzeichnen der Kurven
  // ----------------------
  XClearWindow(xDpy, xWin);
  Paint();
}


/******************************************************************************\
 PVGanttFrame: Implementierung  
\******************************************************************************/

PVGanttFrame::PVGanttFrame(Display*      XDisplay,
                           const char *  Name,
                           int           AdjustRange) :
  PVCurvesFrame(XDisplay, Name, new PVGanttDisplay(XDisplay, AdjustRange))
{
  SetMargin(XTextWidth(axisFont, "xxxxxXXXXX", 10), 16);
}


inline PVGanttDisplay * PVGanttFrame::GetGanttDisplay(void) const
{
  return (PVGanttDisplay*)GetSubDisplay();
}


void PVGanttFrame::AddStateTable(const PDStateTable * StateTable)
{
  assert(GetGanttDisplay());

  GetGanttDisplay()->stateTableList.InsertAfter((PDStateTable *)StateTable);
}


void PVGanttFrame::DrawYAxis(void)
{
  const int XOffset = leftMargin + 1;
  const int XTxPos  = XOffset - 5; 
  const int XPos    = XOffset - 4;
  const int YOffset = topMargin + 4;
  const int YTxPos  = axisFont->ascent / 2;
  SCListIter<PDStateTable> iter(GetGanttDisplay()->stateTableList);
  PDStateTable * stateTable;
  int i;
  int y = 1;     
  int YPos;
    
  XSetForeground(xDpy, xGC, mForeground);
  XSetFont(xDpy, xGC, axisFont->fid);

  for (stateTable = iter++;
       stateTable;
       stateTable = iter++)
  {
    if (stateTable->GetNumOfStates() > 0)
    {
      for (i = stateTable->GetMinStateID();
           i <= stateTable->GetMaxStateID();
           i++, y++)
      {
        const char * stateName = SCType::GetObjectName(SC_STATE, i);
        assert(stateName);
 
        YPos = GetGanttDisplay()->MapY(y) + YOffset;

        XDrawLine(xDpy, xWin, xGC, XOffset, YPos, XPos, YPos);
        DrawRightString(axisFont, stateName, XTxPos, YPos + YTxPos);
      }
      y++; // Luecke lassen
    }
  }
}


/******************************************************************************\
 PVFreqDisplay: Implementierung  
\******************************************************************************/

PVFreqDisplay::PVFreqDisplay(Display* XDisplay) : 
  PVDataDisplay<PDFrequency> (XDisplay),
  wideTxWidth                (XTextWidth(mBold, "00.0%", 5) - beamDist),
  smallTxWidth               (XTextWidth(mMedium, "00%", 3) - beamDist),
  txHeight                   (mBold->ascent + mBold->descent),
  numFreqs                   (0),
  maxNum                     (0),
  numVisible                 (0)
{
  // Konstante Mappingwerte setzen:
  // Y-Achse 0.0 - 1.1, Ursrpung in 0|0
  // ----------------------------------
  mapY.SetOrgPos(0.0);
  mapY.SetDistPos(1.0); 
}


PVFreqDisplay::~PVFreqDisplay(void)
{
}


void PVFreqDisplay::Resized(void)
{
  PVSubDisplay::Resized();
  
  mapY.SetOrgPixel(GetHeight() - 1);
  mapY.SetDistPixel(-(GetHeight() - 16)); // Rand falls 100% erreicht werden
  if (numVisible)
  {
    beamColWidth = GetWidth() / numVisible;
    beamWidth = (beamColWidth - colDist) / numFreqs - beamDist;
  }
}


void PVFreqDisplay::Paint(void)
{ 
  PVSubDisplay::Paint();
  
  if (numVisible)
  {
    int           XPos, YPos;
    int           i, j;       
    int           NumBeams   = numFreqs * numVisible; 
    XRectangle *  WhOutArray = new XRectangle[NumBeams * 3];
    XRectangle *  WhOutIter  = WhOutArray;
    XRectangle *  XRectArray = new XRectangle[NumBeams];
    XRectangle *  XRectIter  = XRectArray;
    double *      FValArray  = new double[NumBeams];
    double *      FValIter   = FValArray;
    PDFrequency * frequency;
    DataIter      iter(dataList);

    // Balkenpositionen berechnen
    // --------------------------

    for (i = 0, frequency = iter++;
         i < numFreqs && frequency;
         i++, frequency = iter++)
    {
      XPos = i * (beamWidth + beamDist) + beamDist;
      for (j = 0; j < maxNum; j++)
      {
        if (frequency->GetRelVal(j) > 0.0)
        {
          *FValIter = j < frequency->Num() ? frequency->GetRelVal(j) : 0;
          XRectIter->x      = XPos;
          XRectIter->width  = beamWidth;
          XRectIter->y      = mapY(*FValIter);
          XRectIter->height = GetHeight() - XRectIter->y;
          FValIter++, XRectIter++;
          XPos += beamColWidth;
        }
      }
    }
    
    iter.GoToFirst();

    // Farbige Balken zeichnen
    // -----------------------
    for (i = 0, frequency = iter++;
         i < numFreqs && frequency;
         i++, frequency = iter++)
    {  
      XSetForeground(xDpy, xGC, frequency->GetColor());
      XFillRectangles(xDpy, xWin, xGC, XRectArray + (i * numVisible), numVisible);
    }

    // Prozentangaben-text ueber den Balken plazieren
    // ---------------------------------------------
    if (beamWidth >= smallTxWidth)
    {
      static const char Wide[]  = "%.1f%%";
      static const char Small[] = "%.0f%%";
      const int         XOffset = beamWidth / 2;
      const char*       Format  = (beamWidth >= wideTxWidth) ? Wide : Small; 
      XFontStruct*      Font    = (beamWidth >= wideTxWidth) ? mBold : mMedium;
      int               TxWidth;
     
      XSetForeground(xDpy, xGC, mForeground);
      XSetFont(xDpy, xGC, Font->fid);
      for (i = NumBeams, FValIter = FValArray, XRectIter = XRectArray; 
           i--; 
           FValIter++, XRectIter++) 
      {	
        char Perc[8]; 
        sprintf(Perc, Format, *FValIter * 100);
        YPos = XRectIter->y - Font->descent;
        if (YPos - Font->ascent < 1) YPos = Font->ascent + 1;
        TxWidth = DrawCenteredString(Font, Perc, XRectIter->x + XOffset, YPos);
        InitWhiteOut(WhOutIter, XRectIter->x, XRectIter->y, TxWidth);
        WhOutIter += 3;
      }
    }
    else
    { // WhiteOut wenn kein Text angezeigt wird
      for (i = NumBeams, XRectIter = XRectArray; i--; XRectIter++)
      {
        InitWhiteOut(WhOutIter, XRectIter->x, XRectIter->y, 0);
        WhOutIter += 3;
      }
    }
    
    // Weissen Hintergrund zeichnen
    // ---------------------------
    XSetForeground(xDpy, xGC, mSelected);
    XFillRectangles(xDpy, xWin, xGC, WhOutArray, NumBeams * 3);

    // Speicher fuer dynamische Arrays wieder freigeben
    // -----------------------------------------------
    delete[] WhOutArray;     
    delete[] XRectArray; 
    delete[] FValArray; 
  }
}

void PVFreqDisplay::InitWhiteOut(XRectangle* WhiteOut, int XPos, int YPos, int TxWidth)
{
  
  // Feintuning fehlt noch, aber andere Sachen sind erst mal wichtiger
  
  WhiteOut[0].x      = XPos - 1;     
  WhiteOut[0].width  = beamWidth + beamDist;
  WhiteOut[0].y      = 0;
  WhiteOut[0].height = YPos - txHeight;
  
  // Diese etwas komplizierte Formel sorgt dafuer, dass die gleichen
  // Rundungsfehler wie beim zentrieren des Strings auftreten.
  // Vielleicht faellt jemandem ja eine logischere Formel ein.
  
  int Width = (WhiteOut[0].width >> 1) - (TxWidth >> 1);
  
  WhiteOut[1].x      = WhiteOut[0].x;
  WhiteOut[1].width  = Width; 
  WhiteOut[1].y      = WhiteOut[0].height;
  WhiteOut[1].height = txHeight;

  WhiteOut[2].x      = XPos + WhiteOut[1].width + TxWidth - 1;
  WhiteOut[2].width  = XPos + beamWidth + beamDist - WhiteOut[2].x;
  WhiteOut[2].y      = WhiteOut[0].height;
  WhiteOut[2].height = txHeight;
}

void PVFreqDisplay::Update(void)
{
  int           i;
  int           OldNumVisible = numVisible;
  PDFrequency * frequency;
  DataIter      iter(dataList);
 
  numFreqs = 0;

  for (frequency = iter++;
       frequency;
       frequency = iter++)
  {
    if (frequency->Num() > maxNum)
      maxNum = frequency->Num();
    numFreqs++;
  }
  iter.GoToFirst();

  frequency = iter++;

  numVisible = 0;

  for (i = 0; i < maxNum; i++)
  {
    if (frequency->GetRelVal(i) != 0.0)
      numVisible++;
  }

  if (numVisible)
  {
    beamColWidth = GetWidth() / numVisible;
    beamWidth = (beamColWidth - colDist) / numFreqs - beamDist;
  } 
  
  updateFrame = (numVisible != OldNumVisible);

  if (updateFrame) XClearWindow(xDpy, xWin); 
  Paint();
}


/******************************************************************************\
 PVFreqFrame: Implementierung  
\******************************************************************************/

PVFreqFrame::PVFreqFrame(Display*           XDisplay,
                         const char *       Name,
                         SCObjectType       ObjectType) :
  PVDataFrame<PDFrequency> (XDisplay, 
                            new PVFreqDisplay(XDisplay),
                            Name),
  objectType(ObjectType)
{
  SetMargin(35, 15); // experimentell ermittelt
}


inline PVFreqDisplay* PVFreqFrame::GetFreqDisplay(void) const 
{
  return (PVFreqDisplay*)GetSubDisplay();
}


void PVFreqFrame::DrawXAxis(void)
{
  int Width = GetFreqDisplay()->GetColWidth();
  int XPos = leftMargin + Width / 2;
  int YPos = topMargin + subHeight + 6 + axisFont->ascent + 2;
  SCListIter<PDFrequency> iter(GetFreqDisplay()->GetDataList());
  
  PDFrequency * frequency;
  
  XSetForeground(xDpy, xGC, mForeground);
  XSetFont(xDpy, xGC, axisFont->fid);

  for (frequency = iter++;
       frequency;
       frequency = iter++)
  { 
    for (int i = 0; i < frequency->Num(); i++)
    {
      if (frequency->GetRelVal(i) > 0.0)
      {
        const char * name = SCType::GetObjectName(objectType, i);
        if (name != NULL)
        {
          DrawCenteredString(axisFont, name, XPos, YPos);
        }
        XPos += Width;
      }
    }
  }
}


void PVFreqFrame::DrawYAxis(void)
{
  const int XOffset = leftMargin + 1;
  const int XTxPos  = XOffset - 5; 
  const int XPos1   = XOffset - 4;
  const int XPos2   = XOffset - 2;
  const int YOffset = topMargin + 4;
  const int YTxPos  = YOffset + axisFont->ascent / 2;

  #define PStep 20
  #define YStep 0.2
  int    p;
  double y;

  XSetForeground(xDpy, xGC, mForeground);
  XSetFont(xDpy, xGC, axisFont->fid);
  for (p = 0, y = 0.0; p <= 100; p += PStep, y += YStep)
  {
    static char Percent[8];
    sprintf(Percent, "%i%%", p);
    DrawRightString(axisFont, Percent, XTxPos, GetFreqDisplay()->MapY(y) + YTxPos);
  }
  
  // Markierungen zeichnen
  // ---------------------
  int YPos;
 
  for (y = 0.0; y <= 1.0; y += YStep)
  {
    YPos = GetFreqDisplay()->MapY(y) + YOffset;
    XDrawLine(xDpy, xWin, xGC, XOffset, YPos, XPos1, YPos);
  }

  for (y = YStep / 2; y <= 1.0; y += YStep)
  {
    YPos = GetFreqDisplay()->MapY(y) + YOffset;
    XDrawLine(xDpy, xWin, xGC, XOffset, YPos, XPos2, YPos);
  }
}

void PVFreqFrame::Update(void)
{
  if (GetFreqDisplay()->UpdateFrame())
  {
    XClearArea(xDpy, xWin, 
               leftMargin, topMargin + subHeight + 6,
               subWidth, bottomMargin,
               false);
    DrawXAxis();
  }
}
