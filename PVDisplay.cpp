/******************************************************************************\
 Datei : PVDisplay.cpp
 Inhalt: Deklaration der Basisklassen fuer die Anzeige von Leistungsdaten:
         PVDisplay, PVSubDisplay, PVFrameDisplay 
 Autor : Christian Rodemeyer, Marc Diefenbruch
 Datum : 16.08.95
 Status: Farbverwaltung muss noch genauer eingestellt werden
\******************************************************************************/   

#include <string.h>
#include "PVDisplay.h"

#if _SC_DMALLOC
  #include <dmalloc.h>
#endif

const char MotifBold[]   = "-adobe-helvetica-bold-r-*-*-12-*-*-*-*-*-*-*";
const char MotifMedium[] = "-adobe-helvetica-medium-r-*-*-12-*-*-*-*-*-*-*";
const char AxisFont[]    = "-adobe-helvetica-medium-r-*-*-10-*-*-*-*-*-*-*";

/******************************************************************************\
 PVDisplay Implementierung
\******************************************************************************/   

long PVDisplay::color[numColors];
SCBoolean PVDisplay::colorInit = true;

PVDisplay::PVDisplay(Display* XDisplay, const char * Name) :
  xDpy  (XDisplay),
  xScr  (DefaultScreen(xDpy)),
  xWin  (XCreateSimpleWindow(xDpy, RootWindow(xDpy, xScr), 0, 0, 100, 100, 0, 
	 		     BlackPixel(xDpy, xScr), WhitePixel(xDpy, xScr))), 
  xGC   (XCreateGC(xDpy, xWin, 0, NULL)),
  xCmap (DefaultColormap(xDpy, xScr)),
  xMono (DefaultDepth(xDpy, xScr) == 1),
  
  mForeground   (AllocColor("Black")),
  mBackground   (AllocColor(xMono ? "White" : "ivory3")),
  mSelected     (AllocColor("White")),
  mTopShadow    (AllocColor("ivory1")),
  mBottomShadow (AllocColor(xMono ? "Black" : "ivory4")),
  mHighlight    (AllocColor(xMono ? "Black" : "Yellow")),
  mBold         (XLoadQueryFont(xDpy, MotifBold)),
  mMedium       (XLoadQueryFont(xDpy, MotifMedium)),
  nextUnusedColor (-1)
{
  XSetBackground(xDpy, xGC, mBackground);
  XSetForeground(xDpy, xGC, mForeground); 

  XStoreName(xDpy, xWin, Name);
  XSetWindowBackground(xDpy, xWin, mBackground);
}


PVDisplay::~PVDisplay(void)
{
  XFreeGC(xDpy, xGC);
  XDestroyWindow(xDpy, xWin);
}


long PVDisplay::AllocColor(const char * ColorName)
{
  XColor e, c;

  XAllocNamedColor(xDpy, xCmap, ColorName, &e, &c);

  return c.pixel;
}


long PVDisplay::GetXEventMask(void) const
{
  return ExposureMask|StructureNotifyMask;
}


void PVDisplay::GetXSizeHints(XSizeHints& Hints) const
{
  Hints.flags = 0;
}   


void PVDisplay::Resized(void)
{
  XWindowAttributes attrib;
  
  XGetWindowAttributes(xDpy, xWin, &attrib);
  width = attrib.width;
  height = attrib.height;
}


void PVDisplay::Paint(void)
{
  DrawRaised(0, 0, width, height);
}


// Simulation einer OSF/Motif Oberfläche durch eigene Draw-Funktionen
// ------------------------------------------------------------------

void PVDisplay::DrawSeparator(int Left, int Top, int Width, int Height)
{
  int X1  = Left;
  int X2  = Left + Width;
  int Y1  = Top;
  int Y2  = Top + Height;
  int XI1 = X1 + 1;
  int XI2 = X2 - 1;
  int YI1 = Y1 + 1;
  int YI2 = Y2 - 1;
  
  if (xMono)
  {
    XSetForeground(xDpy, xGC, mBottomShadow);
    XSetLineAttributes(xDpy, xGC, 0, LineOnOffDash, CapButt, JoinMiter);
    XSetDashes(xDpy, xGC, 0, "\1\1", 2);
  }
  else
  {
    XSetForeground(xDpy, xGC, mTopShadow);
  }
  XDrawLine(xDpy, xWin, xGC, XI1, YI1, XI1, YI2); // vert
  XDrawLine(xDpy, xWin, xGC, XI1, YI1, XI2, YI1); // horz
  XDrawLine(xDpy, xWin, xGC, X2, Y1, X2, Y2);     // vert
  XDrawLine(xDpy, xWin, xGC, X1, Y2, X2, Y2);     // horz
    
  if (xMono)
  {
    XSetLineAttributes(xDpy, xGC, 0, LineSolid, CapButt, JoinMiter); 
  }  
  XSetForeground(xDpy, xGC, mBottomShadow);  
  XDrawLine(xDpy, xWin, xGC, X1, Y1, X1, YI2);    // vert
  XDrawLine(xDpy, xWin, xGC, X1, Y1, XI2, Y1);    // horz
  XDrawLine(xDpy, xWin, xGC, XI2, YI1, XI2, YI2); // vert
  XDrawLine(xDpy, xWin, xGC, XI1, YI2, XI2, YI2); // horz  
}

 
void PVDisplay::DrawRaised(int Left, int Top, int Width, int Height)
{
  int X1  = Left;
  int X2  = Left + Width;
  int Y1  = Top;
  int Y2  = Top + Height;
  int XI1 = X1 + 1;
  int XI2 = X2 - 1;
  int YI1 = Y1 + 1;
  int YI2 = Y2 - 1;
  
  if (xMono)
  {
    XSetForeground(xDpy, xGC, mBottomShadow);
    XSetLineAttributes(xDpy, xGC, 0, LineOnOffDash, CapButt, JoinMiter);
    XSetDashes(xDpy, xGC, 0, "\1\1", 2);
  }
  else
  {
    XSetForeground(xDpy, xGC, mTopShadow);
  }
  XDrawLine(xDpy, xWin, xGC, X1, Y1, X1, Y2);     // vert 1
  XDrawLine(xDpy, xWin, xGC, XI1, YI1, XI1, YI2); // vert 2
  XDrawLine(xDpy, xWin, xGC, X1, Y1, X2, Y1);     // horz 1
  XDrawLine(xDpy, xWin, xGC, XI1, YI1, XI2, YI1); // horz 2
    
  if (xMono)
  {
    XSetLineAttributes(xDpy, xGC, 0, LineSolid, CapButt, JoinMiter); 
  }  
  XSetForeground(xDpy, xGC, mBottomShadow);  
  XDrawLine(xDpy, xWin, xGC, XI1, YI2, XI2, YI2);  // horz 1
  XDrawLine(xDpy, xWin, xGC, X1, Y2, X2, Y2);      // horz 2
  XDrawLine(xDpy, xWin, xGC, XI2, YI1, XI2, YI2);  // vert 1
  XDrawLine(xDpy, xWin, xGC, X2, Y1, X2, Y2);      // vert 2 
}


void PVDisplay::DrawLowered(int Left, int Top, int Width, int Height)
{
  int X1  = Left;
  int X2  = Left + Width - 1;
  int Y1  = Top;
  int Y2  = Top + Height - 1; 
  int XI1 = X1 + 1;
  int XI2 = X2 - 1;
  int YI1 = Y1 + 1;
  int YI2 = Y2 - 1;
  
  if (xMono)
  {
    XSetForeground(xDpy, xGC, mBottomShadow);
    XSetLineAttributes(xDpy, xGC, 0, LineOnOffDash, CapButt, JoinMiter);
    XSetDashes(xDpy, xGC, 0, "\1\1", 2);
  }
  else
  {
    XSetForeground(xDpy, xGC, mTopShadow);
  }
  XDrawLine(xDpy, xWin, xGC, XI1, YI2, XI2, YI2);  // horz 1
  XDrawLine(xDpy, xWin, xGC, X1, Y2, X2, Y2);      // horz 2
  XDrawLine(xDpy, xWin, xGC, XI2, YI1, XI2, YI2);  // vert 1
  XDrawLine(xDpy, xWin, xGC, X2, Y1, X2, Y2);      // vert 2 
  
  if (xMono)
  {
    XSetLineAttributes(xDpy, xGC, 0, LineSolid, CapButt, JoinMiter); 
  }  
  XSetForeground(xDpy, xGC, mBottomShadow);  
  XDrawLine(xDpy, xWin, xGC, X1, Y1, X1, Y2);     // vert 1
  XDrawLine(xDpy, xWin, xGC, XI1, YI1, XI1, YI2); // vert 2
  XDrawLine(xDpy, xWin, xGC, X1, Y1, X2, Y1);     // horz 1
  XDrawLine(xDpy, xWin, xGC, XI1, YI1, XI2, YI1); // horz 2
}


int PVDisplay::DrawCenteredString(XFontStruct* Font, const char * ToDraw,
                                  int CX, int Y)
{
  int l = strlen(ToDraw);
  int w = XTextWidth(Font, ToDraw, l);

  XDrawImageString(xDpy, xWin, xGC, CX - w / 2, Y, ToDraw, l);

  return w;
}


int PVDisplay::DrawRightString(XFontStruct* Font, const char * ToDraw,
                         int RX, int Y)
{
  int l = strlen(ToDraw);
  int w = XTextWidth(Font, ToDraw, l);

  XDrawImageString(xDpy, xWin, xGC, RX - w, Y, ToDraw, l);

  return w;
}


long PVDisplay::GetColor(const char * ColorName)
{
  if (xMono) return AllocColor("Black");
  if (ColorName == NULL)
  {
    if (++nextUnusedColor == numColors) nextUnusedColor = 0;
    return color[nextUnusedColor];
  }
  else {
    return AllocColor(ColorName);
  }
} 


SCStream& operator<< (SCStream& pStream, const PVDisplay&)
{
  return pStream;
}

/******************************************************************************\
 PVSubDisplay Implementierung
\******************************************************************************/   

PVSubDisplay::PVSubDisplay(Display* XDisplay) : 
  PVDisplay(XDisplay, "SubWindow")
{
  XSetWindowBackground(xDpy, xWin, mSelected);
  XSetBackground(xDpy, xGC, mSelected);
  XSetWindowBorderWidth(xDpy, xWin, 0);
  
  if (colorInit)
  {
    colorInit = false;
    if (xMono)
    {
      for (int i = numColors; i--;) color[i] = AllocColor("Black");
    }
    else
    {
      color[0] = AllocColor("DarkGoldenRod");
      color[1] = AllocColor("Red");
      color[2] = AllocColor("DeepSkyBlue");
      color[3] = AllocColor("Firebrick");
      color[4] = AllocColor("DarkSeaGreen");
      color[5] = AllocColor("ForestGreen");
      color[6] = AllocColor("InidianRed");
      color[7] = AllocColor("MediumPurple");
    }
  }
}

void PVSubDisplay::Resized(void)
{
  PVDisplay::Resized();

  if (parent)
  {
    XClearArea(xDpy, parent->GetXWin(), 0, 0, 0, 0, true); // Parent aktualisieren
    XClearWindow(xDpy, GetXWin());
  }
  waitingForResize = false;
}

void PVSubDisplay::Paint(void)
{
  // PVDisplay::Paint() wird NICHT aufgerufen um Rahmen zu verhindern
}


/******************************************************************************\
 PVFrameDisplay Implementierung
\******************************************************************************/   

PVFrameDisplay::PVFrameDisplay(Display* XDisplay, PVSubDisplay* Sub,
                               const char * Name) :
  PVDisplay   (XDisplay, Name),
  sub         (Sub),
  rightMargin (5),
  topMargin   (5),
  axisFont    (XLoadQueryFont(xDpy, AxisFont))
{    
  XReparentWindow(xDpy, sub->GetXWin(), xWin, 0, 0);
  sub->parent = this;
  SetMargin(40, 25);
}


void PVFrameDisplay::SetMargin(int Left, int Bottom)
{
  leftMargin = Left;
  bottomMargin = Bottom;
  subWidth  = GetWidth() - leftMargin - rightMargin - 8;
  subHeight = GetHeight() - topMargin - bottomMargin - 8;
  XMoveResizeWindow(xDpy, sub->GetXWin(), 
                    leftMargin + 4, topMargin + 4, subWidth, subHeight);
}


void PVFrameDisplay::Resized(void)
{
  PVDisplay::Resized();

  sub->waitingForResize = true;
  subWidth  = GetWidth() - leftMargin - rightMargin - 8;
  subHeight = GetHeight() - topMargin - bottomMargin - 8;
  XResizeWindow(xDpy, sub->GetXWin(), subWidth, subHeight);
}


void PVFrameDisplay::Paint(void)
{
  if (sub->IsWaitingForResize()) return;
  PVDisplay::Paint();
  DrawLowered(leftMargin + 2, topMargin + 2, subWidth + 4, subHeight + 4);
  DrawXAxis();
  DrawYAxis();
}
