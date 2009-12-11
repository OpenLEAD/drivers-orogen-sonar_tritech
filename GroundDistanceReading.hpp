#ifndef GROUNDDISTANCEREADING_H
#define GROUNDDISTANCEREADING_H

#include <dfki/time.h>

namespace sensorData
{
	struct GroundDistanceReading{
		DFKI::Time stamp;
		double depth;
	};


};


#endif 
