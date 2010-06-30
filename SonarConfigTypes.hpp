#ifndef SONARCONFIGTYPES_H
#define SONARCONFIGTYPES_H

#include <inttypes.h>
#include <base/time.h>

namespace sensorConfig
{

struct SonarConfig{
    base::Time stamp;
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
#ifndef __orogen
	SonarConfig():
		adc8on(true),
		cont(true),
		scanright(false),
		invert(false),
		chan2(true),
		applyoffset(false),
		pingpong(false),
		rangeScale(30),
		leftLimit(1),
		rightLimit(6399),
		adSpan(81),
		adLow(8),
		initialGain(230),
		motorStepDelayTime(25),
		motorStepAngleSize(32),
		adInterval(200),
		numberOfBins(600),
		adcSetpointCh(0)
	{};
#endif
    };


};


#endif 
