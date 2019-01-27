/******************************************************************************\
 Datei : PVXEventDispatcher.cpp
 Inhalt: Implementierung eins XEventDispatcher fuer die Visualisierungsklassen 
 Autor : Christian Rodemeyer, Marc Diefenbruch
 Datum : 10.10.98
 Status: Prototyping
\******************************************************************************/   

#include <SCL/SCEnvironment.h>
#include <SCL/SCList.h>
#include <SCL/SCListIter.h>

#include <iostream>
#include <unistd.h>
#include <math.h>

#include "PVXEventDispatcher.h"
#include "PVDisplay.h"

#if _SC_DMALLOC
  #include <dmalloc.h>
#endif

PVXEventDispatcher::PVXEventDispatcher(void) :
  displayList(true)
{
  xDpy = XOpenDisplay(NULL);  
  if (!xDpy)
  {
    std::cerr << "PEV: Cannot open display! (DISPLAY variable not set?)" << std::endl;
    exit(1);
  }
}


PVXEventDispatcher::~PVXEventDispatcher(void)
{
  // remove PCController-Windows (must this be?)
  displayList.Remove(displayList.Head());

  XCloseDisplay(xDpy);
}


void PVXEventDispatcher::AddDisplay(PVDisplay* ToAdd)
{
  if (ToAdd->GetSubDisplay()) AddDisplay(ToAdd->GetSubDisplay());
  displayList.InsertAfter(ToAdd);
  XSelectInput(xDpy, ToAdd->GetXWin(), ToAdd->GetXEventMask());
}  


void PVXEventDispatcher::RemoveDisplay(PVDisplay* ToRemove)
{
  std::cout << "PVXEventDispatcher::RemoveDisplay()" << std::endl;
  std::cout.flush();

  if (ToRemove->GetSubDisplay())
    RemoveDisplay(ToRemove->GetSubDisplay());

  displayList.Remove(ToRemove);
}  


void PVXEventDispatcher::ArrangeDisplays(void)
{
  assert(!displayList.IsEmpty()); // Displays muessen existieren

  const int  ScreenHeight = (XDisplayHeight(xDpy, DefaultScreen(xDpy)) * 2) / 3;
  const int  ScreenWidth  = XDisplayWidth(xDpy, DefaultScreen(xDpy)); 
  int        LeftMargin   = 0;  // Linke WM-Dekoration (Rand)
  int        TopMargin    = 0;  // Rechte WM-Dekoration (Rand) 
  XSizeHints Hints;             // Neue Fenstergroeße
  
  // Nun kommt etwas schmutzige Arbeit um festzustellen, wie breit die 
  // Raender sind, die der Window-Manager an das Fenster haengt.
  {
    Window Src   = (*displayList.Head())()->GetXWin();
    Window Dest  = DefaultRootWindow(xDpy);
    Window Child;

    XSync(xDpy, false);
    DoEvents(); // eventuell bereits vorhandene Events abarbeiten
    Hints.flags = USPosition|PPosition;
    Hints.x = Hints.y = 0;
    XMoveWindow(xDpy, Src, Hints.x, Hints.y);
    XSetNormalHints(xDpy, Src, &Hints);  
    XMapRaised(xDpy, Src);
    while (true)
    {
      XSync(xDpy, true); 
      XTranslateCoordinates(xDpy, Src, Dest, 0, 0,
                            &LeftMargin, &TopMargin, &Child);
      if (!LeftMargin || !TopMargin) 
	      sleep(1); // usleep oder nanosleep ist eleganter und schneller
      else
        break;
    }
    XUnmapWindow(xDpy, Src);
    XSync(xDpy, true);
  } // Die Schmutzarbeit ist hiermit erledigt 

  DispIter diter(displayList);
  PVDisplay *display = diter++;

  int      NumDisplays = 0, NumXDisplays, NumYDisplays;

  // Zaehlen der Displays
  // -------------------
  for (display = diter++; display; display = diter++)
  {
    if (!display->IsSubDisplay()) NumDisplays++;
  }
  
  if (NumDisplays)
  {
    NumXDisplays = NumYDisplays = int(sqrt(NumDisplays));
    if (NumXDisplays * NumYDisplays < NumDisplays) NumXDisplays++;
    if (NumXDisplays * NumYDisplays < NumDisplays) NumYDisplays++;
  
    const int WinWidth  = ScreenWidth / NumXDisplays;
    const int WinHeight = ScreenHeight / NumYDisplays; 
  
    Hints.flags      = USSize|USPosition|PMinSize;
    Hints.min_width  = 100;
    Hints.min_height = 100;
    Hints.x          = 0;
    Hints.y          = 0;
    Hints.width      = WinWidth - 2* LeftMargin;
    Hints.height     = WinHeight - TopMargin - LeftMargin;
 
    diter.GoToFirst();
    display = diter++;
    
    for (display = diter++; display; display = diter++)
    {
      if (!display->IsSubDisplay())
      {
        XMoveResizeWindow(xDpy, display->GetXWin(), 
 			  Hints.x, Hints.y, Hints.width, Hints.height);
        XSetNormalHints(xDpy, display->GetXWin(), &Hints);
        Hints.x += WinWidth;
        if (Hints.x >= NumXDisplays * WinWidth)
        {
          Hints.x = 0;
          Hints.y += WinHeight;
          assert(Hints.y <= ScreenHeight);  
        }
      }
      XMapRaised(xDpy, display->GetXWin());
    }
  }  
  
  // Positioniere das PCController-Window, welches immer das erste Display ist
  // -------------------------------------------------------------------------
  diter.GoToFirst();
  display = diter++;
  display->GetXSizeHints(Hints);
  Hints.flags |= USPosition;
  Hints.x = (ScreenWidth - Hints.width - 2 * LeftMargin) / 2;
  Hints.y = ScreenHeight + TopMargin;  
  XMoveWindow(xDpy, display->GetXWin(), Hints.x, Hints.y);
  XResizeWindow(xDpy, display->GetXWin(), Hints.width, Hints.height);
  XSetNormalHints(xDpy, display->GetXWin(), &Hints);
  XMapRaised(xDpy, display->GetXWin());
}


// Auf ein XEvent warten, ohne daß Event zu bearbeiten
void PVXEventDispatcher::WaitForEvent(void)
{
  XEvent ev;
  XPeekEvent(xDpy, &ev);
}


void PVXEventDispatcher::DoEvents(void)
{
  PVDisplay * display;
  DispIter    diter(displayList);
  XEvent      ev;
  
  while (XPending(xDpy))                  // solange X-Ereignisse vorliegen
  {
    XNextEvent(xDpy, &ev);                 // hole sie aus der Warteschlange 
    for (diter.GoToFirst(), display = diter++;
         display;
	 display = diter++)
    {
      if (display->GetXWin() == ev.xany.window) break;
    }
    assert(display != 0); // Event für unangemeldetes XWindow bekommen
    switch(ev.type)
    {
      case Expose:
        if (ev.xexpose.count == 0) display->Paint();
        break;

      case ConfigureNotify:
        if ((ev.xconfigure.width != display->GetWidth()) ||
            (ev.xconfigure.height != display->GetHeight()))
        {  
          display->Resized();
        }
        break;
      
      default: 
        display->Default(ev);
        break;
    }
  }
  XFlush(xDpy);
}

void PVXEventDispatcher::UpdateDisplays(void)
{
  DispIter    iter(displayList);
  PVDisplay * display;

  for (display = iter++;
       display;
       display = iter++)
  {
    display->Update();
  }
}

