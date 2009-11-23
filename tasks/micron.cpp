#include "micron.hpp"

#include <rtt/NonPeriodicActivity.hpp>
#include <QObject>
#include <SonarInterface.h>



using namespace sonar;
        
		

RTT::NonPeriodicActivity* micron::getNonPeriodicActivity()
{ return dynamic_cast< RTT::NonPeriodicActivity* >(getActivity().get()); }


micron::micron(std::string const& name, TaskCore::TaskState initial_state)
    : micronBase(name, initial_state),
    file(QString("scan-data-%1.txt").arg(time(0))),
    stream(&file)
{
    file.open(QIODevice::WriteOnly | QIODevice::Unbuffered);
    stream.setDevice(&file);
	sonar =0;
	app=0;
	depth = -999;
}





/// The following lines are template definitions for the various state machine
// hooks defined by Orocos::RTT. See micron.hpp for more detailed
// documentation about them.

// bool micron::configureHook()
// {
//     return true;
// }
bool micron::startHook()
{
     printf("Starting...\n");
     return true;
 }

void micron::updateHook(std::vector<RTT::PortInterface*> const& updated_ports)
{
	if(app==0){
		char **b;
		b = new char*[1];
		b[0] = new char[2];
		b[0][0]='T';
		b[0][1]=0;
		int *i= new int;
		i[0] = 1;
		app = new QCoreApplication(*i,b);
		sonar = new SonarInterface(_port.value().c_str());
		sonar->start();
		connect(sonar,SIGNAL(scanComplete(SonarScan*)),this,SLOT(scanFinished(SonarScan*)),Qt::DirectConnection);
		connect(sonar,SIGNAL(newDepth(float)),this,SLOT(newDepthReady(float)),Qt::DirectConnection);
	}
	
	app->processEvents();
	
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

	app->processEvents();

}
void micron::newDepthReady(float value){
	depth = value;	
}

void micron::scanFinished(SonarScan *scan){
	sensorData::Sonar data;
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
	data.depth	= depth;
	for(int i=0;i<data.dataBytes;i++){
		data.scanData.push_back(scan->getScanData()[i]);
	}

	_SonarScan.write(data);
    
	stream << *scan;
	file.waitForBytesWritten(0);
 	file.flush();
	
	delete scan;
}


// void micron::errorHook()
// {
// }
void micron::stopHook()
{
     printf("Stopping...\n");
}
/// void micron::cleanupHook()
// {
// }

