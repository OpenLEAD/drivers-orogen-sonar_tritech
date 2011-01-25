#include "Task.hpp"
#include <SonarInterface.h>

using namespace sonar_driver;


Task::Task(std::string const& name)
    : TaskBase(name)
    , sonar(0)
{
	configPhase=false;
	errorCnt=0;
	activity=0;
	_config.set(sensorConfig::SonarConfig()); 
}





/// The following lines are template definitions for the various state machine
// hooks defined by Orocos::RTT. See Task.hpp for more detailed
// documentation about them.

bool Task::configureHook()
{
	sonar = new SonarInterface();
	if (!sonar->init(_port.value().c_str()))
            return false;

	configureDevice();
	activity =  getActivity<RTT::extras::FileDescriptorActivity>();
        if (activity)
        {
            activity->watch(sonar->getFileDescriptor());
            activity->setTimeout(_timeout.get());
        }
	sonar->registerHandler(this);

	return true;
}

void Task::configureDevice()
{
    if (activity){
    	activity->setTimeout(5000);
    }
    configPhase=true;

    sensorConfig::SonarConfig data =  _config.get();
    sonar->sendHeadData(
        data.adc8on,
        data.cont,
        data.scanright,
        data.invert,
        data.chan2,
        data.applyoffset,
        data.pingpong,
        data.rangeScale,
        data.leftLimit,
        data.rightLimit,
        data.adSpan,
        data.adLow,
        data.initialGain,
        data.motorStepDelayTime,
        data.motorStepAngleSize,
        data.adInterval,
        data.numberOfBins,
        data.adcSetpointCh
    );
}

bool Task::startHook()
{
    // Start receiving data
    sonar->requestData();
    return true;
}

void Task::updateHook()
{
    if (activity && activity->hasError() && activity->hasTimeout()){
    	printf("Fatal error: activityError: %s, hasTimeout: %s\n",activity->hasError()?"true":"false",activity->hasTimeout()?"true":"false");
        return exception(IO_ERROR);
    }

    sensorConfig::SonarConfig config;
    while(_config_port.read(config,false) == RTT::NewData)
    {
    	_config.set(config);
	configureDevice();
    }

    scanUpdated = false;
    if (!sonar->processSerialData()){

    	if(!configPhase && errorCnt > 50){ //50 times requesting data before go to real ciritcal error
		fprintf(stderr,"Warning re-requesting data exceeded retry limit!\n");
	        return exception(IO_ERROR);
	}else{
		fprintf(stderr,"Warning re-requesting data!\n");
        	sonar->requestData();
		errorCnt++;
	}

    	if(configPhase && errorCnt > 5000/200){ //200ms timeout and maximum wait time is 5000 on configuration phase
		printf("FATAL errorCount = %i, ConfigPhase: %s\n",errorCnt,configPhase?"true":"false");
	        return exception(IO_ERROR);
	}else
		errorCnt++;
    }

    // Check if we got a new scan. If we did, ask for a new one
    if (scanUpdated)
        sonar->requestData();
}

void Task::processDepth(base::Time const& time, double value){
	sensorData::GroundDistanceReading groundData;
	groundData.stamp = time;
	groundData.depth = value;
	_CurrentGroundDistance.write(groundData);
}

void Task::processSonarScan(SonarScan const& scan){
	base::samples::SonarScan baseScan;

	baseScan.time 	   = scan.time;

	baseScan.time_beetween_bins    = ((scan.adInterval*640.0)*10e-9);

	baseScan.angle     = scan.bearing/6399.0*2.0*M_PI;

	baseScan.scanData  = scan.scanData;

        scanUpdated = true;
	_BaseScan.write(baseScan);
	if(configPhase){
    		if (activity){
	            	activity->setTimeout(_timeout.get());
		}
		configPhase=false;
		errorCnt=0;
	}
	errorCnt = 0;

}


// void Task::errorHook()
// {
// }
// void Task::stopHook()
// {
// }

void Task::cleanupHook()
{
        if (activity)
            activity->unwatch(sonar->getFileDescriptor());
        sonar->close();
}

