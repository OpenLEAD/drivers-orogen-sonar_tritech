/* Generated from orogen/lib/orogen/templates/tasks/Task.cpp */

#include "Profiling.hpp"
#include <iodrivers_base/Timeout.hpp>

using namespace sonar_tritech;

Profiling::Profiling(std::string const& name)
    : ProfilingBase(name)
{
}

void Profiling::pushProfile()
{
    laser_scan.ranges.clear();
    profiling.decodeScan(laser_scan);
    _profiling_scan.write(laser_scan);
    timeoutAcquisition.restart();
}

bool Profiling::setAcquisition_config(::sea_net::ProfilingAcquisitionConfig const & value)
{
    // Need to read the pending data packet first
    if (profiling.hasPendingData())
    {
        profiling.receiveData(_io_read_timeout.get().toMilliseconds());
        pushProfile();
    }

    profiling.configureAcquisition(value, _configure_timeout.get()*1000);
    profiling.requestData();
    return ProfilingBase::setAcquisition_config(value);
}

/// The following lines are template definitions for the various state machine
// hooks defined by Orocos::RTT. See Profiling.hpp for more detailed
// documentation about them.

bool Profiling::configureHook()
{
    profiling.clear();

    if (!_port.value().empty())
        profiling.openSerial(_port.value(), _baudrate.value());
    else if (!_io_port.value().empty())
        profiling.openURI(_io_port);
    setDriver(&profiling);
    if (!ProfilingBase::configureHook())
        return false;

    profiling.configure(_config.get(), _configure_timeout.get()*1000);
    profiling.configureAcquisition(_acquisition_config.get(), _configure_timeout.get()*1000);
    return true;
}

bool Profiling::startHook()
{
    // Do one ping to verify that the sonar is operational
    //
    // Wait up to one second. This is needed because the
    // motor of the sonar is powering down after a while
    // and it needs some time to send HeadData again
    profiling.requestData();
    profiling.receiveData(_configure_timeout.get()*1000);
    pushProfile();

    base::Time acquisitionTimeout = _acquisition_timeout.get();
    hasAcquisitionTimeout = !acquisitionTimeout.isNull();
    timeoutAcquisition =
        iodrivers_base::Timeout(acquisitionTimeout.toMilliseconds());

    // And start pulling
    profiling.requestData();
    return ProfilingBase::startHook();
}
void Profiling::processIO()
{
    sea_net::PacketType packet_type = profiling.readPacket(_io_read_timeout.get().toMilliseconds());
    if (packet_type == sea_net::mtHeadData)
    {
        pushProfile();
        profiling.requestData();
    }
    if (hasAcquisitionTimeout && timeoutAcquisition.elapsed())
    {
        profiling.requestData();
        timeoutAcquisition.restart();
    }
}
void Profiling::stopHook()
{
    ProfilingBase::stopHook();
}

