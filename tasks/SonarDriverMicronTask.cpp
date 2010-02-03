#include "SonarDriverMicronTask.hpp"

#include <rtt/FileDescriptorActivity.hpp>

#include <SonarInterface.h>

using namespace sonar_driver;


RTT::FileDescriptorActivity* SonarDriverMicronTask::getFileDescriptorActivity()
{ return dynamic_cast< RTT::FileDescriptorActivity* >(getActivity().get()); }

std::string SonarDriverMicronTask::getLoggerFileName(const char *comment){
	char tmp[100];
	sprintf(tmp,"scan-data-%i-%s.txt",(int)time(0),comment);
	return std::string(tmp);
}

SonarDriverMicronTask::SonarDriverMicronTask(std::string const& name)
    : SonarDriverMicronTaskBase(name),
    stream(SonarDriverMicronTask::getLoggerFileName(_logComment.value().c_str()).c_str())
{
	sonar =0;
	depth = -999;
	doLogging=false;
}





/// The following lines are template definitions for the various state machine
// hooks defined by Orocos::RTT. See SonarDriverMicronTask.hpp for more detailed
// documentation about them.

bool SonarDriverMicronTask::configureHook()
{
	sonar = new SonarInterface();
	if(sonar->init(_port.value().c_str())){
		if(getFileDescriptorActivity() == 0){
	                fprintf(stderr,"Cannot use File Descriptor Activity, did you use periodic?\n");
	                return false;
            	}else{
			printf("Watching File descriptor for Sonar: %i\n",sonar->getReadFD());
			getFileDescriptorActivity()->watch(sonar->getReadFD());
		}
	}else{
		fprintf(stderr,"Cannot initialze Micron Driver on Port %s, going to error state...\n",_port.value().c_str());
		return false;
	}
	sonar->registerHandler(this);

	if(doLogging && !stream.is_open()){
		return false;
	}

	return true;
}
// bool SonarDriverMicronTask::startHook()
// {
//     return true;
// }

void SonarDriverMicronTask::updateHook(std::vector<RTT::PortInterface*> const& updated_ports)
{
	//Check Loggin System:
	if(doLogging != _doLogging){
		doLogging = _doLogging;
		if(!doLogging){
			stream.close();
		}else{
    		stream.open(SonarDriverMicronTask::getLoggerFileName(_logComment.value().c_str()).c_str());
		}
	}


	if(!updated_ports.empty()){
		if(isPortUpdated(_SonarConfig)){
			sensorConfig::SonarConfig data;
			if(!_SonarConfig.read(data)){
				fprintf(stderr,"Data not availible yet\n");	
			}else{
				printf("Got new HeadConfig\n");
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
		}else{
			printf("Sonar config is not updated, but what else?\n");
		}
	}
	sonar->processSerialData();	
}

void SonarDriverMicronTask::processDepth(const double value){
	depth = value;	
}

void SonarDriverMicronTask::processSonarScan(SonarScan *scan){
	sensorData::Sonar data;
	sensorData::GroundDistanceReading groundData;
	data.packedSize = scan->getpackedSize();
	data.deviceType = scan->getdeviceType();
	data.headStatus = scan->getheadStatus();
	data.sweepCode  = scan->getsweepCode();
	data.headControl= scan->getheadControl();
	data.range		= scan->getrange();
	data.txn		= scan->gettxn();
	data.gain		= scan->getgain();
	data.slope		= scan->getslope();
	data.adSpawn	= scan->getadSpawn();
	data.adLow		= scan->getadLow();
	data.headingOffset = scan->getheadingOffset();
	data.adInterval = scan->getadInterval();
	data.leftLimit	= scan->getleftLimit();
	data.rightLimit	= scan->getrightLimit();
	data.steps		= scan->getsteps();
	data.bearing	= scan->getbearing();
	data.dataBytes	= scan->getdataBytes();
	data.scanData.reserve(data.dataBytes);
	for(int i=0;i<data.dataBytes;i++){
		data.scanData.push_back(scan->getScanData()[i]);
	}
	
	groundData.depth	= depth;
	groundData.stamp.now();
	_CurrentGroundDistance.write(groundData);
	data.stamp.now();
	_SonarScan.write(data);
   	
	if(doLogging) 
		stream << *scan;
	
	delete scan;
}


// void SonarDriverMicronTask::errorHook()
// {
// }
// void SonarDriverMicronTask::stopHook()
// {
// }
// void SonarDriverMicronTask::cleanupHook()
// {
// }

