#ifndef GROUNDDISTANCEREADING_H
#define GROUNDDISTANCEREADING_H

#include <base/time.h>

namespace sensorData
{
	struct GroundDistanceReading{
		base::Time stamp;
		double depth;
	};


};


#endif 
