/**
 * Author: Matthias Goldhoorn (matthias.goldhoorn@dfki.de)
 * Company: Deutsches Forschungszentrum für Künstliche Intelligenz - Robotics Innovation Center (DFKI RIC)
 * Year 2010
 * Desc:
 *
*/
#ifndef SONARCONFIGTYPES_H
#define SONARCONFIGTYPES_H

#include <inttypes.h>
#include <base/time.h>
#include <Profiling.h>
#include <string.h>

namespace sensorConfig
{

struct ProfilingConfig{
	base::Time stamp;
	SeaNet::Profiling::headControl config;

#ifndef __orogen
	bool operator!=(const ProfilingConfig &other) const{
		if(stamp != other.stamp || config != other.config)
			return true;
		return false;
	}
#endif
};

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
    base::Angle leftLimit;
    base::Angle rightLimit;
    uint8_t adSpan;
    uint8_t adLow;
    double initialGain;
    uint8_t motorStepDelayTime;
    base::Angle motorStepAngleSize;
    double maximumDistance;
    double resolution;
    //uint16_t adInterval;
    //uint16_t numberOfBins;
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
		leftLimit(-M_PI),
		rightLimit(M_PI),
		adSpan(81),
		adLow(8),
		initialGain(0.2),
		motorStepDelayTime(25),
		motorStepAngleSize(5.0/180.0*M_PI),
		maximumDistance(15.0),
		resolution(0.1),
		adcSetpointCh(0)
	{};
	bool operator!=(const SonarConfig &other) const{
	    bool b = 
	    other.stamp==stamp &&
	    other.adc8on==adc8on &&
	    other.cont==cont &&
	    other.scanright==scanright &&
	    other.invert==invert &&
	    other.chan2==chan2 &&
	    other.applyoffset==applyoffset &&
	    other.pingpong==pingpong &&
	    other.rangeScale==rangeScale &&
	    other.leftLimit==leftLimit &&
	    other.rightLimit==rightLimit &&
	    other.adSpan==adSpan &&
	    other.adLow==adLow &&
	    other.initialGain==initialGain &&
	    other.motorStepDelayTime==motorStepDelayTime &&
	    other.motorStepAngleSize==motorStepAngleSize &&
	    other.maximumDistance==maximumDistance&&
	    other.resolution==resolution&&
	    other.adcSetpointCh==adcSetpointCh;
	    return !b;
	};
#endif
    };



};


#endif 
