#include <ctype.h>
#include <string.h>

#include <SCL/SCScheduler.h>

#include "PEScanner.h"
#include "PESensor.h"
#include "PESProcess.h"
#include "PESMachine.h"
#include "PESetup.h"

#if _SC_DMALLOC
  #include <dmalloc.h>
#endif

static const char * ValIndexTypeNames[numValIndexTypes] =
{
  "num", "min", "max", "avg", "avi", "var", "dev", "cnt", "cpt", "cpi", "cql"
};


SCBoolean SensorHasInterval(long SensorType)
{
  static const long HasInterval = (1 << sProcQLen )       |
                                  (1 << sProcNumber)      |
				  (1 << sProcSigWaitTime) |
				  (1 << sMachQLen)        |
				  (1 << sMachReqWaitTime) |
				  (1 << sMachReqThruTime) |  
				  (1 << sMachUtilization) | 
				  (1 << sEvent)           |
				  (1 << sActivity);
  return (1 << SensorType) & HasInterval;
}

SCBoolean IsFreqSensor(long SensorType)
{
  static const long IsFreq = (1 << sProcQLenFreq)   |
                             (1 << sProcStateFreq)  |
			     (1 << sProcInSigFreq)  |
			     (1 << sProcOutSigFreq) |
			     (1 << sMachQLenFreq)   |
			     (1 << sProcOutReqFreq) |
			     (1 << sMachInReqFreq)  |
			     (1 << sGlobalSigFreq)  |  
			     (1 << sGlobalReqFreq);
  return (1 << SensorType) & IsFreq;
}


Scanner::Scanner(const char * Config) :
  in   (Config),
  line (1),
  col  (0)
{
  if (!in.good() || in.eof())
  {
    std::cerr << "Cannot open file " << Config << "!\n";
    abort();
  }
  in.getline(buf, maxCol);
}


void Scanner::Error(const char * Msg)
{
  std::cerr << '\n' << Msg << " in line " << line << "!\n";
  SCScheduler::Shutdown();
}


void Scanner::GetChar(char C, const char * Msg)
{
  SkipIt();
  if (buf[col++] != C)
    std::cerr << "'" << C << "' expected " << Msg << std::endl;
}


SCBoolean Scanner::CheckChar(char C)
{
  SkipIt();
  return buf[col] == C;
}


void Scanner::SkipIt()
{
  while ((col == maxCol) || !((buf[col] == ':')  || 
                              (buf[col] == ',')  || 
                              (buf[col] == ';')  ||
			      (buf[col] == '{')  ||
			      (buf[col] == '}')  ||
			      (buf[col] == '(')  ||
			      (buf[col] == ')')  ||
			      (buf[col] == '"')  ||
			      (buf[col] == '_')  ||
                              isalnum(buf[col]))	
	 )
  {  
    if (col == 128 || buf[col] == '\n' || !buf[col])
    {
      if (in.eof()) Error("Unexpected end of file");
      in.getline(buf, 128);
      line++;
      col = 0;
    }
    else if ((buf[col] == '/') && (buf[col + 1] == '/'))
    {
      col = 128;
    }
    else if (buf[col] == ' ' || buf[col] == '\t')
    {
      col++;
    }  
    else
    { 
      std::cout << col << ", " << &buf[col] << std::endl;
      Error ("Illegal Character");
    }  
  }
}


void Scanner::GetKeyWord(char * KeyWord)
{
  int i;
  
  SkipIt();
  for (i = col; (isalnum(buf[i]) || buf[i] == '_') && i < maxCol; i++);
  strncpy(KeyWord, buf + col, i - col);
  KeyWord[i - col] = '\0';
  col = i;
}


void Scanner::GetString(char * Buffer)
{
  int i;
  
  GetChar('"', "before String");
  for (i = col;
       buf[i] != '"' && buf[i] != '\n' && i < maxCol;
       i++);
  if (buf[i] != '"') Error("Missing '\"' after String");
  strncpy(Buffer, &(buf[col]), i - col);
  Buffer[i - col] = '\0';
  col = i + 1;
}


void Scanner::GetKeyString(const char * KeyWord, char * Buffer)
{
  char Word[128] = "";
  char tmp[128] = "after Keyword ";
  
  GetKeyWord(Word);
  if (strcmp(Word, KeyWord))
    std::cerr << KeyWord << " expected" << std::endl;
  strcat(tmp, KeyWord);
  GetChar(':', tmp);
  GetString(Buffer);
  GetChar(';', "");
}


void Scanner::GetInt(int& Buffer)
{
  int i;
  
  SkipIt();
  for (i = col; i < maxCol && isdigit(buf[i]); i++);
  if (i == col) Error("Integer expected");
  sscanf(&buf[col], "%i", &Buffer);
  col = i;
}


void Scanner::GetDbl(double& Buffer)
{ 
  int i;
   
  SkipIt();
  for (i = col; i < maxCol && (isdigit(buf[i]) || buf[i] == '.'); i++);
  if (!sscanf(&buf[col], "%lg", &Buffer))
    Error("Double expected");
  col = i; 
}

  
void Scanner::GetKeyDbl(const char * KeyWord, double& Buffer)
{
  char Word[128];
  char tmp[128] = "after keyword ";

  GetKeyWord(Word);
  if (strcmp(Word, KeyWord))
    std::cerr << KeyWord << " expected" << std::endl;
  strcat(tmp, KeyWord);
  GetChar(':', tmp);
  GetDbl(Buffer);
  GetChar(';', "");
}


void Scanner::GetKeyInt(const char * KeyWord, int& Buffer)
{
  char Word[128];
  char tmp[128] = "after keyword ";
  
  GetKeyWord(Word);
  if (strcmp(Word, KeyWord))
    std::cerr << KeyWord << " expected" << std::endl;
  strcat(tmp, KeyWord);
  GetChar(':', tmp);
  GetInt(Buffer);
  GetChar(';', "");
}


SCBoolean Scanner::GetKeyWordIndex(const char * Key[], int& Index)
{
  char Word[128];
  
  GetKeyWord(Word);
  for (Index = 0;
       (Key[Index])[0] != '\0' && strcmp(Key[Index], Word);
       Index++);
  return ((Key[Index])[0] != '\0');
}

    
void Scanner::GetKeyBlock(const char * KeyWord)
{
  char Word[128];
  char tmp[128] = "after keyword ";
  
  strcat(tmp, KeyWord);

  GetKeyWord(Word);
  if (strcmp(Word, KeyWord))
    std::cerr << "Keyword " << KeyWord << " expected" << std::endl;
  GetChar(':', tmp);
  strcat(tmp, " and ':'");
  GetChar('{', tmp);
}


void Scanner::GetEvent(PDEventType*& Event)
{
  char Buffer[128];
  char * nameSigReq = NULL;
  char * nameProcMach = NULL;
  SCBoolean isSignal = false;
  SCBoolean isArrival = false;
  
  GetChar(',', "after Parameter");
  GetKeyWord(Buffer);
  if (!strcmp(Buffer, "In")) 
    isArrival = true;
  else if (!strcmp(Buffer, "Out"))
    isArrival = false;
  else 
    Error("'In' or 'Out' expected");
  
  GetChar(',', "after Parameter");
  GetKeyWord(Buffer);
  if (!strcmp(Buffer, "Signal"))
    isSignal = true;
  else if (!strcmp(Buffer, "Request"))
    isSignal = false;
  else 
    Error("'Signal' or 'Request' expected");
  
  GetChar(',', "after Parameter");

  GetString(Buffer);  
  nameSigReq = strdup(Buffer);

  GetChar(',', "after Parameter");

  GetString(Buffer);  
  nameProcMach = strdup(Buffer);

  Event = new PDEventType(isArrival, isSignal,
                          nameSigReq, nameProcMach);
}


void Scanner::GetConParas(int& ValIndex, char * ColorName,
                          int DispType, int SensorType)
{
  char Name[128];
  SCBoolean   ColorOnly = !(DispType == dCurves || DispType == dFixedCurves);

  if (DispType == dGantt) ValIndex = PESStateFrequency::ganttState;
  strcpy(ColorName, "");
  if (CheckChar('('))
  {
    GetChar('(', "");
    SkipIt(); 
    if (!ColorOnly)
    {
      if (IsFreqSensor(SensorType))
      {
        if (!isdigit(buf[col])) 
          Error("Numeric Index expected for frequency sensor");
        else 
          GetInt(ValIndex);
      }
      else
      {
        GetKeyWord(Name);
        for (ValIndex = 0; 
             ValIndex < numValIndexTypes && strcmp(ValIndexTypeNames[ValIndex], Name); 
             ValIndex++);
        if (ValIndex == numValIndexTypes) Error("Unknown or missing named value index");
      }
    }
    if (ColorOnly || CheckChar(','))
    {
      if (!ColorOnly) GetChar(',', "");
      GetString(ColorName);
    }
    GetChar(')', "");
  }
  else if (!ColorOnly) Error("Value Index expected for this displaytype");
}


void Scanner::GetSensorParameters(int           SensorType,
                                  char *        Buf1,
                                  char *        Buf2,
                                  PDEventType*& Ev1,
                                  PDEventType*& Ev2,
                                  double&       Interval)
{
  switch (SensorType)
  {
	
    case sGlobalSigFreq:
    case sGlobalReqFreq:
      break;
	  
    case sProcQLen:
    case sProcQLenFreq:
    case sProcNumber:
    case sProcStateFreq:
    case sProcInSigFreq:
    case sProcOutSigFreq:
    case sProcOutReqFreq: 	  
    case sMachQLen:
    case sMachQLenFreq:
    case sMachUtilization:  
    case sMachInReqFreq: 	  
      GetString(Buf1);
      break;  
    
    case sProcSigWaitTime:
    case sMachReqWaitTime:
    case sMachReqThruTime:  
      GetString(Buf1);
      GetChar(',', "after first parameter");
      GetString(Buf2);
    break;  
      
    case sEvent: 
      GetString(Buf1);
      GetEvent(Ev1);
    break;  
      
    case sActivity:
      GetString(Buf1);
      GetEvent(Ev1);
      GetEvent(Ev2);
    break;  
  }
  
  if (SensorHasInterval(SensorType) && CheckChar(',')) {
    GetChar(',', "");
    GetDbl(Interval);
  }
  GetChar(';', "after sensor definition");
}

