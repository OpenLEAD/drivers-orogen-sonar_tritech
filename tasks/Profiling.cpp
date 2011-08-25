/* Generated from orogen/lib/orogen/templates/tasks/Task.cpp */

#include "Profiling.hpp"

using namespace sonar_tritech;

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
	sonar->sendHeadData(_config.get().config);
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
    sensorConfig::ProfilingConfig newConfig = _config.get(); 
    if(newConfig != currentConfig){
	sonar->sendHeadData(_config.get().config);
	currentConfig = _config.get();
	//printf("reconfiguring\n");	
    }
	
    if (!sonar->processSerialData(_timeout.get())){
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
		baseScan.minRange = 20; //Hardcoded 2 cm
		baseScan.maxRange = 100000; //Hardcoded 100meter
		baseScan.angular_resolution = scan->stepSize/6399.0*2.0*M_PI;
		baseScan.speed = 0;
		printf("Stant angle: %f, resolution: %f\nScans:",baseScan.start_angle,baseScan.angular_resolution);
		for(unsigned int i=0;i<scan->scanData.size();i++){
			double distance = scan->scanData[i]*1e-6*1500.0/2.0;//Time in microsecounds to secounds (1e-6) * time of water in speed (1500) / twice the way (2.0)
			baseScan.ranges.push_back(distance*1000); //To millimeters 
			printf(" %f",distance);
		}
		printf("\n");
		scanUpdated = true;
		_Scan.write(baseScan);
	}
}
