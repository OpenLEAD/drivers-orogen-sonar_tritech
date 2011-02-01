/* Generated from orogen/lib/orogen/templates/tasks/Task.cpp */

#include "Profiling.hpp"

using namespace sonar_driver;

Profiling::Profiling(std::string const& name)
    : ProfilingBase(name)
{
	activity=0;
	SeaNet::Profiling::headControl hc = SeaNet::Profiling::Driver::getDefaultHeadData();
	sensorConfig::ProfilingConfig pc;
	pc.config = hc;
	_config.set(pc);
	scanUpdated=false;
}



/// The following lines are template definitions for the various state machine
// hooks defined by Orocos::RTT. See Profiling.hpp for more detailed
// documentation about them.

bool Profiling::configureHook()
{
    	if (! ProfilingBase::configureHook())
        	return false;
	
	sonar = new SeaNet::Profiling::Driver();
	if (!sonar->init(_port.value().c_str()))
	    return false;

	activity =  getActivity<RTT::extras::FileDescriptorActivity>();
	//configureDevice();
	sonar->registerHandler(this);

	return true;
}

bool Profiling::startHook()
{
	if (! ProfilingBase::startHook())
        	return false;

	if (activity)
	{
	    activity->watch(sonar->getFileDescriptor());
	    activity->setTimeout(_timeout.get());
	}
    	sonar->requestData();
	return true;
}
void Profiling::updateHook()
{
    ProfilingBase::updateHook();
    if (activity && activity->hasError() && activity->hasTimeout()){
    	printf("Fatal error: activityError: %s, hasTimeout: %s\n",activity->hasError()?"true":"false",activity->hasTimeout()?"true":"false");
        return exception(IO_ERROR);
    }
    if (!sonar->processSerialData()){
       	sonar->requestData();
    }else{
	if(scanUpdated)
		sonar->requestData();
    }













}
// void Profiling::errorHook()
// {
//     ProfilingBase::errorHook();
// }
void Profiling::stopHook()
{
    ProfilingBase::stopHook();
}
// void Profiling::cleanupHook()
// {
//     ProfilingBase::cleanupHook();
// }


void Profiling::processSonarScan(const SonarScan *s){
	const ProfilerScan *scan = dynamic_cast<const ProfilerScan*>(s);
	if(scan){	
		base::samples::LaserScan baseScan;
		baseScan.time 	   = scan->time;
		baseScan.start_angle = scan->leftLimit/6399.0*2.0*M_PI;
		baseScan.angular_resolution = scan->stepSize/6399.0*2.0*M_PI;
		baseScan.speed = 0;
		for(unsigned int i=0;i<scan->scanData.size();i++){
			baseScan.ranges.push_back(scan->scanData[i]*1e-6*1500.0/2.0); //Time in microsecounds to secounds (1e-6) * time of water in speed (1500) / twice the way (2.0)
		}
		scanUpdated = true;
		_Scan.write(baseScan);
	}
}
