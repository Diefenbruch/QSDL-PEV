/******************************************************************************\
 Datei : PCControllerontroller.cpp
 Inhalt: Interaktive Steuerung und Überwachung eins QSDL-Simulators.
         Deklaration der Klassen PCControlleronntroller
 Autor : Christian Rodemeyer, Marc Diefenbruch
 Datum : 10.10.98
 Status: Fehler korrigiert (MD)
\******************************************************************************/   

#include <iostream>
#include <string.h>

#include <SCL/SCSignal.h>

#include "PCController.h"
#include "PEEventDispatcher.h"

#if _SC_DMALLOC
  #include <dmalloc.h>
#endif

/******************************************************************************\
 PCControllerontroller: Implementierung
\******************************************************************************/   

PCController::PCController(PEEventDispatcher*  Parent,
                           PVXEventDispatcher& XParent,
                           const char *        SysName) :
  PVDisplay        (XParent.GetXDisplay(), SysName),
  xEventDispatcher (XParent),
  controlled       (*Parent),
  stopped          (false),
  asyncUpdate      (false)
{
  numProcesses = numSignals = numMachines = numRequests = 0;
  
  bWidth  = 74;
  bHeight = 20;
  for (int i = bNone, y = 45; ++i < bNum; y += bHeight + 5)
  {
    bPos[i].y = y;
    bPos[i].x = 178;
  }
  bCaption[bStop]  = "Stop";
  bCaption[bSync]  = "Rough";
  bCaption[bSnap]  = "Snapshot";
  bCaption[bReset] = "Reset";
  bCaption[bQuit]  = "Quit";
  pressed = bNone;
}


PCController::~PCController(void)
{
}


// Sensor-Funktionalitaet
// ---------------------
SCBoolean PCController::NotifyOnEvent(SCTraceAction Event) const
{
  switch (Event)
  {
    case scTraceProcessCreate:  case scTraceProcessStop:
    case scTraceMachineCreate:  case scTraceMachineStop:
    case scTraceSignalConsume:  case scTraceSignalReceive:   
    case scTraceServiceRequest: case scTraceServiceFinish:
    case scTraceSignalDrop: // added by MD
      return true;
      
    default:
      return false;
  }
}


void PCController::EvProcessCreate(const SCProcess*, const SCProcess*) 
{
  numProcesses++;
}


void PCController::EvProcessDelete(const SCProcess*)
{
  numProcesses--;
}


void PCController::EvMachineCreate(const SCMachine*) 
{
  numMachines++;
}


void PCController::EvMachineDelete(const SCMachine*) 
{
  numMachines--;
}


void PCController::EvSignalConsume(const SCProcess*, const SCSignal*)
{
  numSignals--;
}


void PCController::EvSignalDrop(const SCProcess*, const SCSignal*)
{
  numSignals--;
}


void PCController::EvSignalReceive(const SCProcess*, const SCSignal*)
{
  numSignals++;
}


void PCController::EvServiceRequest(const SCMachine*, const SCRequest*) 
{
  numRequests++;
}

 
void PCController::EvServiceFinish(const SCMachine*, const SCRequest*) 
{
  numRequests--;
}


// Display-Funktionalitaet
// ----------------------

long PCController::GetXEventMask(void) const
{
  return PVDisplay::GetXEventMask()
         |ButtonPressMask|ButtonReleaseMask|StructureNotifyMask;
}  

void PCController::GetXSizeHints(XSizeHints& Hints) const
{
  Hints.flags  = PSize|USSize|PMinSize|PMaxSize;
  Hints.width  = Hints.min_width  = Hints.max_width  = 262;
  Hints.height = Hints.min_height = Hints.max_height = 169;
}  
 
// 'Harte' Kodierung des Grafikaufbaus im Control Fenster, da nicht erwartet
// wird, daß hier Änderung erforderlich werden. Bei einem Ausbau der
// Benutzerschnittstelle sollten Tools eingesetzt werden.
// -------------------------------------------------------------------------

static int AlignVert   = 110;
static int ValueWidth  = 46;
static int AlignValueX = AlignVert + ValueWidth / 2;

void PCController::Paint(void) 
{
  int i, y; // Laufvariable in for-Schleifen am Anfang der Funktion!!!

  // Hintergrund
  // ----------
  DrawRaised(0, 0, GetWidth(), 40);             // Oben
  DrawRaised(0, 40, 168, GetHeight() - 40);    // Links
  DrawRaised(168, 40, 94, GetHeight() - 40);  // Rechts

  // Statische Beschriftung
  // ----------------------
  static const char * Label[4] = {"Requests waiting:", "Machines:",
			    "Signals waiting:", "Processes:"};
  
  XSetFont(xDpy, xGC, mMedium->fid);
  XSetBackground(xDpy, xGC, mBackground);
  for (i = 4, y = 52; i--; y += 29)
  {
    int len = strlen(Label[i]);
    XSetForeground(xDpy, xGC, mForeground); 
    XDrawImageString(xDpy, xWin, xGC, 
                     AlignVert -  XTextWidth(mMedium, Label[i], len) - 4,
                     y + 15,
                     Label[i], len);
    DrawLowered(AlignVert, y, ValueWidth, 20);
  }
  DrawCounters();

  // Simulationszeit
  // ---------------
  DrawLowered(AlignVert, 7, 142, 25);
  static const char * SimTime = "Modell Time:";
  XSetFont(xDpy, xGC, mBold->fid);
  XSetForeground(xDpy, xGC, mForeground);
  XSetBackground(xDpy, xGC, mBackground);
  XDrawImageString(xDpy, xWin, xGC, 31, 25, SimTime, strlen(SimTime));
  DrawSimTime();

  // Pushbuttons zeichnen
  // --------------------
  for (i = bNum; i--; )
  {
    DrawButton(bPos[i], bCaption[i], i == pressed);
  }
}

#include <stdio.h> // Wie werden numerischen Werte unter Unix in Strings konvertiert?
                   // dochwohl nicht nur durch sprintf...?! 
static char Buffer[32];

void PCController::DrawSimTime(void)
{
  static const char   Big[]   = "-adobe-helvetica-bold-r-*-*-14-*-*-*-*-*-*-*";
  static XFontStruct* BigFont = XLoadQueryFont(xDpy, Big);
  static const int x = 112;
  static const int y = 9;
  static const int w = 138;
  static const int h = 21;
  static const int tx = (x + (x + w)) / 2;
  static const int ty = (y + (y + h)) / 2;

  sprintf(Buffer, "%#.8g", Now());
  XSetForeground(xDpy, xGC, mSelected);
  XFillRectangle(xDpy, xWin, xGC, x, y, w, h);
  XSetForeground(xDpy, xGC, mForeground);
  XSetBackground(xDpy, xGC, mSelected);
  XSetFont(xDpy, xGC, BigFont->fid);
  DrawCenteredString(BigFont, Buffer, tx, ty + 6);
}

void PCController::DrawCounters(void)
{
  static const int x = AlignVert + 2;
  static const int w = ValueWidth - 4;  
  static const int h = 16;
  static const int t = 13;
  int              y = 54;

  XSetFont(xDpy, xGC, mBold->fid);
  XSetBackground(xDpy, xGC, mSelected);

  sprintf(Buffer, "%i", numProcesses);
  XSetForeground(xDpy, xGC, mSelected);
  XFillRectangle(xDpy, xWin, xGC, x, y, w, h);
  XSetForeground(xDpy, xGC, mForeground);
  DrawCenteredString(mBold, Buffer, AlignValueX, y + t);

  y += 29;
  sprintf(Buffer, "%i", numSignals);
  XSetForeground(xDpy, xGC, mSelected);
  XFillRectangle(xDpy, xWin, xGC, x, y, w, h);
  XSetForeground(xDpy, xGC, mForeground);
  DrawCenteredString(mBold, Buffer, AlignValueX, y + t);

  y += 29;
  sprintf(Buffer, "%i", numMachines);
  XSetForeground(xDpy, xGC, mSelected);
  XFillRectangle(xDpy, xWin, xGC, x, y, w, h);
  XSetForeground(xDpy, xGC, mForeground);
  DrawCenteredString(mBold, Buffer, AlignValueX, y + t);
  
  y += 29;
  sprintf(Buffer, "%i", numRequests);
  XSetForeground(xDpy, xGC, mSelected);
  XFillRectangle(xDpy, xWin, xGC, x, y, w, h);
  XSetForeground(xDpy, xGC, mForeground);
  DrawCenteredString(mBold, Buffer, AlignValueX, y + t);
}


void PCController::DrawButton(const XPoint& Pos, const char * Caption,
                              SCBoolean Lowered)
{
  static const int TxMarg = 14; // Y-Ausrichtung des Button-Textes
  static const int Offset = bWidth / 2 - 2;

  XSetFont(xDpy, xGC, mBold->fid);
  XSetBackground(xDpy, xGC, mBackground);
  
  if (Lowered)
  {
    DrawLowered(Pos.x, Pos.y, bWidth + 1, bHeight + 1);
    XSetForeground(xDpy, xGC, mForeground);
    DrawCenteredString(mBold, Caption, Pos.x + Offset + 2, Pos.y + TxMarg + 2);
  }
  else
  {
    DrawRaised(Pos.x, Pos.y, bWidth, bHeight);
    XSetForeground(xDpy, xGC, mForeground);
    DrawCenteredString(mBold, Caption, Pos.x + Offset, Pos.y + TxMarg);
  }  
}


void PCController::ClearButton(const XPoint& Button)
{
  XSetForeground(xDpy, xGC, mBackground);
  XFillRectangle(xDpy, xWin, xGC, Button.x, Button.y, bWidth, bHeight);
}  


SCBoolean PCController::PosInButton(int PosX, int PosY, const XPoint& Button)
{
  return ((PosX > Button.x) && (PosX < Button.x + bWidth))  &&
         ((PosY > Button.y) && (PosY < Button.y + bHeight));
}  
 

void PCController::Default(XEvent& ev)
{
  switch (ev.type)
  {
    case ButtonPress:
      if (ev.xbutton.button == Button1)
      {
        for (int i = bNum; i--;)
        {
          if (PosInButton(ev.xbutton.x, ev.xbutton.y, bPos[i]))
          {
            ClearButton(bPos[i]);
            DrawButton(bPos[i], bCaption[i], true);
            pressed = Button(i);
          }
        }
      }
      break;

    case ButtonRelease:      
      if (ev.xbutton.button == Button1)
      {
        if (pressed != bNone)
        {
          switch (pressed)
          {
            case bStop:
              if (stopped)
              {
                bCaption[bStop] = "Stop";
                stopped = false;
              }
              else
              {
                bCaption[bStop] = "Continue";
                stopped = true;
                ClearButton(bPos[bStop]);
                DrawButton(bPos[bStop], bCaption[bStop], false);
                pressed = bNone;
                while (stopped)
                {
                  xEventDispatcher.WaitForEvent();
                  xEventDispatcher.DoEvents();
                }
                pressed = bStop;
              }
              break;
	    
      	    case bSync:
      	      bCaption[bSync] = asyncUpdate ? "Rough" : "Detail";
      	      asyncUpdate = !asyncUpdate;
      	      controlled.SetUpdateMode(asyncUpdate);
      	      break;

      	    case bSnap:
      	      controlled.ReportAllSensors();
              break;
 
      	    case bReset:
      	      controlled.ResetAllSensors();
              break;
 
      	    case bQuit:
      	      stopped = false;
      	      SCScheduler::Shutdown();  // Quit-Befehl der SCL
      	      break;
	    
	          default: break;
          }
      	  ClearButton(bPos[pressed]);
      	  DrawButton(bPos[pressed], bCaption[pressed], false);
      	  pressed = bNone;
      	}
      }
      break;  
    
    default: break;
  }
}


void PCController::Update(void)
{
  DrawSimTime();
  DrawCounters();
}

