#include "SonarDriverMicronTask.hpp"
#include <SonarInterface.h>

using namespace sonar_driver;


SonarDriverMicronTask::SonarDriverMicronTask(std::string const& name)
    : SonarDriverMicronTaskBase(name)
    , sonar(0)
{
	configPhase=false;
	errorCnt=0;
	activity=0;
	_config.set(sensorConfig::SonarConfig()); 
}





/// The following lines are template definitions for the various state machine
// hooks defined by Orocos::RTT. See SonarDriverMicronTask.hpp for more detailed
// documentation about them.

bool SonarDriverMicronTask::configureHook()
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

void SonarDriverMicronTask::configureDevice()
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

bool SonarDriverMicronTask::startHook()
{
    // Start receiving data
    sonar->requestData();
    return true;
}

void SonarDriverMicronTask::updateHook()
{
    if (activity && activity->hasError() && activity->hasTimeout()){
    	printf("Fatal error: activityError: %s, hasTimeout: %s\n",activity->hasError()?"true":"false",activity->hasTimeout()?"true":"false");
        return exception(IO_ERROR);
    }

    sensorConfig::SonarConfig config;
    if (_config_port.read(config) == RTT::NewData)
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

void SonarDriverMicronTask::processDepth(base::Time const& time, double value){
	sensorData::GroundDistanceReading groundData;
	groundData.stamp = time;
	groundData.depth = value;
	_CurrentGroundDistance.write(groundData);
}

void SonarDriverMicronTask::processSonarScan(SonarScan const& scan){
	sensorData::Sonar data;
	data.stamp = scan.time;
	data.packedSize    = scan.packedSize;
	data.deviceType    = scan.deviceType;
	data.headStatus    = scan.headStatus;
	data.sweepCode     = scan.sweepCode;
	data.headControl   = scan.headControl;
	data.range         = scan.range;
	data.txn           = scan.txn;
	data.gain          = scan.gain;
	data.slope         = scan.slope;
	data.adSpawn       = scan.adSpawn;
	data.adLow         = scan.adLow;
	data.headingOffset = scan.headingOffset;
	data.adInterval    = scan.adInterval;
	data.leftLimit     = scan.leftLimit;
	data.rightLimit    = scan.rightLimit;
	data.steps         = scan.steps;
	data.bearing       = scan.bearing;
	data.scanData      = scan.scanData;
        scanUpdated = true;
	_SonarScan.write(data);
	printf("Got SonarScan..\n");
	if(configPhase){
    		if (activity){
	            	activity->setTimeout(_timeout.get());
		}
		configPhase=false;
		errorCnt=0;
	}
	errorCnt = 0;

}


// void SonarDriverMicronTask::errorHook()
// {
// }
// void SonarDriverMicronTask::stopHook()
// {
// }

void SonarDriverMicronTask::cleanupHook()
{
        if (activity)
            activity->unwatch(sonar->getFileDescriptor());
        sonar->close();
}

