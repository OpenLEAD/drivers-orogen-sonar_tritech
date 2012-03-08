#include "Micron.hpp"
#include "base/angle.h"

using namespace sonar_tritech;


Micron::Micron(std::string const& name)
    : MicronBase(name)
    , sonar(0)
{
	configPhase=false;
	errorCnt=0;
	activity=0;
	_config.set(sensorConfig::SonarConfig());
	currentConfig = _config.get();
}





/// The following lines are template definitions for the various state machine
// hooks defined by Orocos::RTT. See Micron.hpp for more detailed
// documentation about them.

bool Micron::configureHook()
{
	sonar = new SeaNet::Micron::Driver(false); 
        //Do not create PTS subdevice if no reader readds the data!
	
        if (!sonar->init(_port.value().c_str()))
            return false;

	activity =  getActivity<RTT::extras::FileDescriptorActivity>();
	sonar->registerHandler(this);
        fprintf(stderr,"Opened subdevice: %s\n",sonar->getSlavePTS());
        _pts_subdevice.write(std::string(sonar->getSlavePTS()));
	return true;
}

void Micron::configureDevice()
{
    if (activity){
    	activity->setTimeout(5000);
    }
    configPhase=true;
    sensorConfig::SonarConfig data =  _config.get();
    int bins	=  data.maximumDistance/data.resolution;

    //1500m/s sound veolicity in water
    //twice the time
    //to microsecounds
    //value of one means 640microsecounds for the sonar
    int ad	= (((data.resolution/1500.0)*2.0)*1e9)/640.0;

    if(
	data.initialGain < 0.0 || data.initialGain > 1.0 ||
	data.leftLimit.rad < -M_PI || data.leftLimit.rad > M_PI ||
	data.rightLimit.rad < -M_PI || data.rightLimit.rad > M_PI ||
	data.motorStepAngleSize.rad < 0 || data.motorStepAngleSize.rad / (0.05625/180.0*M_PI) > 255.0 ||
	ad < 0 || ad > 1500 ||
	bins < 0 || bins > 1500){
		return exception(INVALID_CONFIGURATION);
	}


    sonar->sendHeadData(
        data.adc8on,
        data.cont,
        data.scanright,
        data.invert,
        data.chan2,
        data.applyoffset,
        data.pingpong,
        data.rangeScale,
        ((M_PI-data.leftLimit.rad)/(M_PI*2.0))*6399.0,
        ((M_PI-data.rightLimit.rad)/(M_PI*2.0))*6399.0,
        data.adSpan,
        data.adLow,
        data.initialGain,
        data.motorStepDelayTime,
        (data.motorStepAngleSize.rad/(M_PI*2.0))*6399.0,
        ad,
        bins,
        data.adcSetpointCh
    );
}

bool Micron::startHook()
{
    // Start receiving data
    sonar->requestData();
    if (activity)
    {
            activity->watch(sonar->getFileDescriptor());
            activity->setTimeout(_timeout.get());
    }
    configureDevice();
    return true;
}

void Micron::updateHook()
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
    config = _config.get();
    if(currentConfig != config){
    	currentConfig = config;
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

void Micron::processDepth(base::Time const& time, double value){
	sensorData::GroundDistanceReading groundData;
	groundData.stamp = time;
	groundData.depth = value;
	_CurrentGroundDistance.write(groundData);
}

void Micron::processSonarScan(const SonarScan *s){
	const MicronScan *scan = dynamic_cast<const MicronScan*>(s);
	if(scan){	
		base::samples::SonarBeam baseScan;

		baseScan.time = scan->time;

		baseScan.sampling_interval  = ((scan->adInterval*640.0)*1e-9);

		baseScan.bearing     = base::Angle::fromRad(M_PI-(scan->bearing/6399.0*2.0*M_PI));

		baseScan.beam  = scan->scanData;
                baseScan.speed_of_sound = 1500;
                baseScan.beamwidth_vertical = 35.0/180.0*M_PI;
                baseScan.beamwidth_horizontal = 3.0/180.0*M_PI;
#if 0
		sensorConfig::SonarConfig debugConfig;
		debugConfig.rangeScale		= scan->range;
		debugConfig.leftLimit.rad	= scan->leftLimit;
		debugConfig.rightLimit.rad	= scan->rightLimit;
		debugConfig.adSpan		= scan->adSpawn;
		debugConfig.adLow		= scan->adLow; 
		debugConfig.initialGain		= scan->gain;
		debugConfig.motorStepDelayTime 	= 0;//scan->
		debugConfig.motorStepAngleSize 	= scan->steps;
		debugConfig.adInterval	 	= scan->adInterval;  
		debugConfig.numberOfBins 	= 0;//scan->dataBytes;        
		debugConfig.adcSetpointCh	= 0;//scan->        
		_debug_config.write(debugConfig);
#endif
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
    
	const GroundDistance *gd = dynamic_cast<const GroundDistance*>(s);
	if(gd){
            base::samples::RigidBodyState water_depth;
            water_depth.invalidate();
            water_depth.time = gd->time;
            water_depth.position[2] = gd->distance;
            water_depth.cov_position(2,2) = 0.2;
            _ground_distance.write(water_depth);
        }
}


// void Micron::errorHook()
// {
// }

void Micron::stopHook()
{
 if (activity)
 {
         activity->clearAllWatches();
 }
}

void Micron::cleanupHook()
{
        if (activity)
            activity->unwatch(sonar->getFileDescriptor());
        sonar->close();
}

