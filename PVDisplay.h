/******************************************************************************\
 Datei : PVDisplay.h
 Inhalt: Deklaration der Basisklassen fuer die Anzeige von Leistungsdaten:
         PVDisplay, PVSubDisplay, PVFrameDisplay
 Autor : Christian Rodemeyer, Marc Diefenbruch
 Datum : 16.08.95
 Status: Farbverwaltungsexperimente
\******************************************************************************/   

#ifndef __PVDISPLAY_H
#define __PVDISPLAY_H

#include <SCL/SCBasicTypes.h>
#include <SCL/SCStream.h>

#ifndef _XLIB_H_
#include <X11/Xlib.h>
#endif
#ifndef _XUTIL_H_
#include <X11/Xutil.h>
#endif

/******************************************************************************\
 PVDisplay: Anzeige eines X-Window, dessen Ereignisse vom PVXEventDispatcher
  verwaltet werden. Der Dispatcher ruft als Antwort auf das Eintretetn von
  XEvents die entsprechenden Antwortefunktionen auf.
\******************************************************************************/   

class PVDisplay
{
  public:
    PVDisplay(Display* XDisplay, const char * Name);
    virtual ~PVDisplay(void);

    virtual void Update() = 0;       // Aktualisierung nach Sensor‰nderung 
    
    // XEvent-Antwortefunktionen
    // -------------------------
    virtual void Paint();            // Ende eines Exposure Ereignisses
    virtual void Resized();          // Aufruf nach Groeﬂen‰nderung 
    virtual void Default(XEvent&) {} // Unbenannte Events (nicht unbekannt!)

    virtual long       GetXEventMask() const;
    virtual void       GetXSizeHints(XSizeHints& Hints) const;
    virtual SCBoolean  IsSubDisplay() const {return false;}
    virtual PVDisplay* GetSubDisplay() const {return NULL;}
    
    Window GetXWin()   const {return xWin;}
    int    GetWidth()  const {return width;}  // tats‰chliche Breite in Pixeln
    int    GetHeight() const {return height;} // tats‰chliche Hoehe in Pixeln
  
    friend SCStream& operator<< (SCStream& pStream,
                                 const PVDisplay& pData);

    long GetColor(const char * ColorName = ""); // Reserviert angegebene Farbe oder
                                                // liefert naechste Standardfarbe
    
  protected:
  
    // X-Objekte f¸r Grafikfunktionen
    // ------------------------------
    Display* const xDpy;  // Verbindung zum XServer
    const int      xScr;  // Default-Screen
    const Window   xWin;  // Id Datenstruktur des Drawable (Widget Verbindung)
    const GC       xGC;   // GrafikContext
    const Colormap xCmap; // Farbtabelle
    const SCBoolean     xMono; // true bei monochromen Screen
    
    // Farben und Resourcen f¸r X11-OSF/Motif Simulation
    // -------------------------------------------------
    const long   mForeground;   // z.B. f¸r Button-Text
    const long   mBackground;   // Standard 3D-Hintergrund
    const long   mSelected;     // Tempor‰re Selektion oder TextEdit
    const long   mTopShadow;    // Aufhellung  (Erhebung)
    const long   mBottomShadow; // Abdunkelung (Erhebung)
    const long   mHighlight;    // Umriﬂ um fokussiertes Element (NYI)
    XFontStruct* mBold;         // Fontauswahlprobleme ungeloest
    XFontStruct* mMedium;
    
    // Grafik-Primitive zum Erzeugen einer OSF/Motif konformen Oberfl‰che
    // ------------------------------------------------------------------
    long AllocColor(const char * ColorName);
    void DrawSeparator(int Left, int Top, int Width, int Height);
    void DrawRaised(int Left, int Top, int Width, int Height);
    void DrawLowered(int Left, int Top, int Width, int Height);
    int  DrawCenteredString(XFontStruct* Font, const char * ToDraw, int CX, int Y);
    int  DrawRightString(XFontStruct* Font, const char * ToDraw, int RX, int Y);

//  private:  
    int width, height; // Tats‰chliche Dimensionen des Pixelfensters

    // Verf¸gbare Farben f¸r Diagrammdarstellung
    // -----------------------------------------
    enum {numColors = 8};           // Verf¸gbare Standardfarben
    static long color[numColors];   
    static SCBoolean colorInit;
    int nextUnusedColor;
};


/******************************************************************************\
 PVFrameDisplay: Ein Rahmenfenster kann ein SubDisplay aufnehmen, welches
   in der Regel zur Visualisierung von Datenstroemen eingesetzt wird. Im
   Rahmenfenster selber kann dann die Achsenbeschriftung erscheinen.
\******************************************************************************/   

class PVSubDisplay;

class PVFrameDisplay: public PVDisplay
{
  public:
    PVFrameDisplay(Display* XDisplay, PVSubDisplay* Sub, const char * Name);

    // Redefinierte Funktionen
    // -----------------------
    void Paint();   
    void Resized(); // ¸bernimmt Groeﬂen‰nderung des SubDisplays

    PVDisplay* GetSubDisplay() const {return (PVDisplay*)sub;}

    // neue Funktionen
    // ---------------
    void SetMargin(int Left, int Bottom);

  protected:
    PVSubDisplay* sub; // Das untergeordnete Fenster
    
    // Definitions des Abstands des Grafikfensters (PVGraphDisplay)
    // ------------------------------------------------------------
    int       leftMargin;   // linker Rand
    const int rightMargin;  // rechter und Rand
    const int topMargin;    // oberer Rand sind konstant
    int       bottomMargin; // Abstand des Koordinatensystems
    int       subWidth;     // Breite des SubFensters
    int       subHeight;    // Hoehe des SubFensters
    
    // Schriftart der Achsenbeschriftung
    // ----------------------------------------------
    XFontStruct* axisFont;
    
    // Zeichnen der Achsenbeschriftung
    // -------------------------------
    virtual void DrawXAxis() = 0;
    virtual void DrawYAxis() = 0;
};


/******************************************************************************\
 PVSubDisplay: SubFenster, welches in ein FrameDisplay eingebunden ist.
\******************************************************************************/   

class PVSubDisplay: public PVDisplay
{ 
  public:

    // Redefinierte virtuelle Funktionen
    // ---------------------------------
    SCBoolean IsSubDisplay() const {return true;}
    void Resized();
    void Paint();

    // Kommunikation mit dem ParentDisplay w‰hrend des Resizens
    // --------------------------------------------------------
    SCBoolean IsWaitingForResize() const {return waitingForResize;}
    
  protected: 
    PVSubDisplay(Display* XDisplay); // Hier ist der Konstruktor versteckt!

  private: 
    
    // Elemente, die sowohl vom Sub- als auch vom FrameDisplay verwaltet 
    // werden m¸ssen, um Paint und Exposure-Events zu koordinieren
    // ----------------------------------------------------------------
    PVFrameDisplay* parent;           // Init durch Frame-Konstruktor
    SCBoolean       waitingForResize; // Init durch Frame-Resize
    
    friend PVFrameDisplay::PVFrameDisplay(Display*, PVSubDisplay*, const char *);
    friend void PVFrameDisplay::Resized();
};

#endif

