#include "SonarDriverMicronTask.hpp"

#include <rtt/FileDescriptorActivity.hpp>

#include <SonarInterface.h>

using namespace sonar_driver;


RTT::FileDescriptorActivity* SonarDriverMicronTask::getFileDescriptorActivity()
{ return dynamic_cast< RTT::FileDescriptorActivity* >(getActivity().get()); }

SonarDriverMicronTask::SonarDriverMicronTask(std::string const& name)
    : SonarDriverMicronTaskBase(name)
    , sonar(0)
{
}





/// The following lines are template definitions for the various state machine
// hooks defined by Orocos::RTT. See SonarDriverMicronTask.hpp for more detailed
// documentation about them.

bool SonarDriverMicronTask::configureHook()
{
	sonar = new SonarInterface();
	if (!sonar->init(_port.value().c_str()))
            return false;

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

        RTT::FileDescriptorActivity* activity = getFileDescriptorActivity();
        if (activity)
        {
            activity->watch(sonar->getFileDescriptor());
            activity->setTimeout(_timeout.get());
        }
	sonar->registerHandler(this);

	return true;
}

bool SonarDriverMicronTask::startHook()
{
    // Start receiving data
    sonar->requestData();
    return true;
}

void SonarDriverMicronTask::updateHook()
{
    RTT::FileDescriptorActivity* activity = getFileDescriptorActivity();
    if (activity && activity->hasError() && activity->hasTimeout())
        return fatal(IO_ERROR);

    scanUpdated = false;
    if (!sonar->processSerialData())
        return fatal(IO_ERROR);

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
}


// void SonarDriverMicronTask::errorHook()
// {
// }
// void SonarDriverMicronTask::stopHook()
// {
// }

void SonarDriverMicronTask::cleanupHook()
{
        RTT::FileDescriptorActivity* activity = getFileDescriptorActivity();
        if (activity)
            activity->unwatch(sonar->getFileDescriptor());
        sonar->close();
}

