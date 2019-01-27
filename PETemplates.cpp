#ifdef __GNUC__

#include <SCL/SCList.tmpl.h>
#include <SCL/SCTransition.h>

#include "PDDataType.h"
#include "PEEventDispatcher.h"
#include "PESetup.h"

#if _SC_DMALLOC
  #include <dmalloc.h>
#endif

template class SCList<PDDataType>;
template class SCList<PESensor>;
template class SCList<PCUpdater>;
template class SCList<PVDisplay>;
template class SCList<PDCurve>;
template class SCList<PDFrequency>;
template class SCList<SensorTable::TEntry>;
template class SCList<DataTypeTable::TEntry>;
template class SCList<PDStateTable>;

template class SCListCons<PDDataType>;
template class SCListCons<PESensor>;
template class SCListCons<PCUpdater>;
template class SCListCons<PVDisplay>;
template class SCListCons<PDCurve>;
template class SCListCons<PDFrequency>;
template class SCListCons<SensorTable::TEntry>;
template class SCListCons<DataTypeTable::TEntry>;
template class SCListCons<PDStateTable>;

template class SCListIter<PDDataType>;
template class SCListIter<PESensor>;
template class SCListIter<PCUpdater>;
template class SCListIter<PVDisplay>;
template class SCListIter<PDCurve>;
template class SCListIter<PDFrequency>;
template class SCListIter<SensorTable::TEntry>;
template class SCListIter<DataTypeTable::TEntry>;
template class SCListIter<PDStateTable>;

#endif
