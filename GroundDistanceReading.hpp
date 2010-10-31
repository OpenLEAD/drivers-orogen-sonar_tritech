/**
 * Author: Matthias Goldhoorn (matthias.goldhoorn@dfki.de)
 * Company: Deutsches Forschungszentrum für Künstliche Intelligenz - Robotics Innovation Center (DFKI RIC)
 * Year 2010
 * Desc:
 *
*/
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
