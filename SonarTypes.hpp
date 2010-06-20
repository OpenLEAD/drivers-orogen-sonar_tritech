#ifndef SONARTYPES_H
#define SONARTYPES_H

#include <inttypes.h>
#include <base/time.h>

#ifndef __orogen
#include <vector>
#endif


namespace sensorData
{
	struct Sonar{
#ifndef __orogen
	double getScale(){
	  return  (((dataBytes*adInterval*640.0)/1000000000.0)*1500.0/2.0)/dataBytes;
	}

#endif
		base::Time stamp;
		uint16_t packedSize;
		uint8_t deviceType;
		uint8_t headStatus;
		uint8_t sweepCode;
		uint16_t headControl;
		uint16_t range;
		uint32_t txn;
		uint8_t gain;
		uint16_t slope;
		uint8_t adSpawn;
		uint8_t adLow;
		uint16_t headingOffset;
		uint16_t adInterval;
		uint16_t leftLimit;
		uint16_t rightLimit;
		uint8_t steps;
		uint16_t bearing;
		uint16_t dataBytes;
		std::vector<uint8_t> scanData;
	};


};


#endif 
