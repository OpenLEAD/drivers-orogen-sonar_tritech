#ifndef SONARCONFIGTYPES_H
#define SONARCONFIGTYPES_H

#include <inttypes.h>
#include <dfki/time.h>

namespace sensorConfig
{
	struct SonarConfig{
#ifndef __orogen

#endif
    DFKI::Time stamp;
    bool adc8on;
    bool cont;
    bool scanright;
    bool invert;
    bool chan2;
    bool applyoffset;
    bool pingpong;
    
	uint16_t rangeScale;
    uint16_t leftLimit;
    uint16_t rightLimit;
    uint8_t adSpan;
    uint8_t adLow;
    uint8_t initialGain;
    uint8_t motorStepDelayTime;
    uint8_t motorStepAngleSize;
    uint16_t adInterval;
    uint16_t numberOfBins;
    uint16_t adcSetpointCh;

	};


};


#endif 
