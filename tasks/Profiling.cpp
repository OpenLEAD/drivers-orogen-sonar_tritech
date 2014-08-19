/* Generated from orogen/lib/orogen/templates/tasks/Task.cpp */

#include "Profiling.hpp"
#include <iodrivers_base/Timeout.hpp>

using namespace sonar_tritech;

Profiling::Profiling(std::string const& name)
    : ProfilingBase(name)
{
}


bool Profiling::setConfig(::sea_net::ProfilingConfig const & value)
{
    // Need to read the pending data packet first
    if (profiling.hasPendingData())
        profiling.receiveData(_io_read_timeout.get().toMilliseconds());

    profiling.configure(value, _configure_timeout.get()*1000);

    //Call the base function, DO-NOT Remove
    return(ProfilingBase::setConfig(value));
}

/// The following lines are template definitions for the various state machine
// hooks defined by Orocos::RTT. See Profiling.hpp for more detailed
// documentation about them.

bool Profiling::configureHook()
{
    if (!_port.value().empty())
        profiling.openSerial(_port.value(), _baudrate.value());
    else if (!_io_port.value().empty())
        profiling.openURI(_io_port);

    profiling.configure(_config.get(), _configure_timeout.get()*1000);
    setDriver(&profiling);
    return ProfilingBase::configureHook();
}

bool Profiling::startHook()
{
    // Do one ping to verify that the sonar is operational
    profiling.requestData();
    profiling.receiveData(1000);
    // And start pulling
    profiling.requestData();
    return ProfilingBase::startHook();
}
void Profiling::processIO()
{
    sea_net::PacketType packet_type = profiling.readPacket(_io_read_timeout.get().toMilliseconds());
    if (packet_type == sea_net::mtHeadData)
    {
        laser_scan.ranges.clear();
        profiling.decodeScan(laser_scan);
        _profiling_scan.write(laser_scan);
        profiling.requestData();
    }
}
void Profiling::stopHook()
{
    ProfilingBase::stopHook();
}

