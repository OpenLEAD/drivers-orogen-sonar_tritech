/* Generated from orogen/lib/orogen/templates/tasks/Task.cpp */

#include "Profiling.hpp"
#include <iodrivers_base/Timeout.hpp>

using namespace sonar_tritech;

Profiling::Profiling(std::string const& name)
    : ProfilingBase(name)
{
}



/// The following lines are template definitions for the various state machine
// hooks defined by Orocos::RTT. See Profiling.hpp for more detailed
// documentation about them.

bool Profiling::configureHook()
{
    profiling.setWriteTimeout(1000*_write_timeout.get());
    try
    {
        profiling.openSerial(_port.value(), _baudrate.value());
        profiling.configure(_config.get(),_configure_timeout.get()*1000);
        profiling_config = _config.get();

        //check if full duplex is set
        //if not the user has to set it via tritech software
        if(!profiling.isFullDublex(1000))
                std::cout << "WARNING: Micron is not using Full Dublex" << std::endl;
    }
    catch(std::runtime_error e)
    {
        std::cerr << "Cannot open port and configure the device!" << std::endl;
        std::cerr << e.what() << std::endl;
        return false;
    }
    return true;
}

bool Profiling::startHook()
{
    profiling.start();
    try
    {
        //Wait up to one second. This is needed because the
        //motor of the sonar is powering down after a while
        //and it needs some time to send HeadData again
        profiling.waitForPacket(sea_net::mtHeadData,10000);
    }
    catch(std::runtime_error e)
    {
        std::cerr << "Cannot start the device!" << std::endl;
        std::cerr << e.what() << std::endl;
        return false;
    }
    return true;
}
void Profiling::updateHook()
{
    //check if the configuration has changed 
    if(profiling_config != _config.get())
    {
        try
        {
            std::cout << "Reconfigure during operation!" << std::endl;
            profiling.configure(_config.get(),_configure_timeout.get()*1000);
            profiling_config = _config.get();
        }
        catch(std::runtime_error e)
        {
            std::cerr << "Cannot reconfigure the device!" << std::endl;
            std::cerr << e.what() << std::endl;
            return exception(IO_ERROR);
        }
    }
    else
    {
        try
        {
            iodrivers_base::Timeout time_out(_read_timeout.get()*10000);
            sea_net::PacketType packet_type = sea_net::mtNull;
            while(packet_type != sea_net::mtHeadData)
            {
                packet_type = profiling.readPacket(time_out.timeLeft());
                if(packet_type == sea_net::mtHeadData)
                {
                    base::samples::LaserScan laser_scan;
                    profiling.decodeScan(laser_scan);
                    _profiling_scan.write(laser_scan);
                }
            }
            if(time_out.elapsed())
                throw std::runtime_error("Time for reading mtHeadData elapsed.");       //got to the catch block
        }
        catch(std::runtime_error e)
        {
            std::cerr << "Cannot read profiling scans!" << std::endl;
            std::cerr << e.what() << std::endl;
            return exception(IO_ERROR);
        }
    }

    //trigger
    getActivity()->trigger(); //TODO use fd to trigger the task
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
