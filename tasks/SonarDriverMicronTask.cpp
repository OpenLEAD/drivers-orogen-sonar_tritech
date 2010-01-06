#include "SonarDriverMicronTask.hpp"

#include <rtt/NonPeriodicActivity.hpp>

//#include <QObject>
#include <SonarInterface.h>

using namespace sonar_driver;


RTT::NonPeriodicActivity* SonarDriverMicronTask::getNonPeriodicActivity()
{ return dynamic_cast< RTT::NonPeriodicActivity* >(getActivity().get()); }

char* SonarDriverMicronTask::getLoggerFileName(){
	char tmp[100];
	sprintf(tmp,"scan-data-%i.txt",(int)time(0));
	return tmp;
}

SonarDriverMicronTask::SonarDriverMicronTask(std::string const& name, TaskCore::TaskState initial_state)
    : SonarDriverMicronTaskBase(name, initial_state),
    //file(QString("scan-data-%1.txt").arg(time(0))),
    stream(SonarDriverMicronTask::getLoggerFileName())
{
//    file.open(QIODevice::WriteOnly | QIODevice::Unbuffered);
 //   stream.setDevice(&file);
	sonar =0;
//	app=0;
	depth = -999;
}





/// The following lines are template definitions for the various state machine
// hooks defined by Orocos::RTT. See SonarDriverMicronTask.hpp for more detailed
// documentation about them.

bool SonarDriverMicronTask::configureHook()
{
	sonar = new SonarInterface(_port.value().c_str());
	sonar->start();
	return true;
}
// bool SonarDriverMicronTask::startHook()
// {
//     return true;
// }

void SonarDriverMicronTask::updateHook(std::vector<RTT::PortInterface*> const& updated_ports)
{
	
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
    
	stream << *scan;
	//file.waitForBytesWritten(0);
 	//file.flush();
	
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

