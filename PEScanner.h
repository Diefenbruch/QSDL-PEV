#include <fstream>

#include <SCL/SCBasicTypes.h>

#include "PDDataType.h"

// Ein primitiver Scanner für die Konfigurationsdatei
// -----------------------------------

class Scanner 
{
  public:
    Scanner(const char * Config);

    void GetChar(char C, const char * Msg);
    SCBoolean CheckChar(char c);
    void GetString(char * Buffer);
    void GetInt(int& Buffer);
    void GetDbl(double& Buffer);
    void GetKeyWord(char * KeyWord);
    void GetKeyBlock(const char * KeyWord);
    void GetKeyString(const char * KeyWord, char * Buffer);
    void GetKeyInt(const char * KeyWord, int& Buffer);
    void GetKeyDbl(const char * KeyWord, double& Buffer);
    SCBoolean GetKeyWordIndex(const char * KeyWords[], int& Index); 
    void GetEvent(PDEventType*& Event);
    void GetConParas(int& ValIndex, char * ColorName, 
                     int DispType, int SensorType); 
    void GetSensorParameters(int SensorType,
                             char * Buf1, char * Buf2,
                             PDEventType*& Ev1, PDEventType*& Ev2,
                             double& Interval);  

    void Error(const char * Msg);

private:
  enum {maxCol = 128};

  std::ifstream in;
  char     buf[maxCol];
  int      line;
  int      col;

  void SkipIt();
};

