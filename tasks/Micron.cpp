#include "Micron.hpp"
#include <iodrivers_base/Timeout.hpp>

using namespace sonar_tritech;

Micron::Micron(std::string const& name)
: MicronBase(name)
{
}

bool Micron::configureHook()
{
    micron.setWriteTimeout(1000*_write_timeout.get());
    try
    {
        micron.openSerial(_port.value());
        micron.configure(_config.get(),_configure_timeout.get()*1000);
        current_config = _config.get();

        //check if full duplex is set
        //if not the user has to set it via tritech software
        if(!micron.isFullDublex(1000))
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

bool Micron::startHook()
{
    micron.start();
    try
    {
        //Wait up to one second. This is needed because the
        //motor of the sonar is powering down after a while
        //and it needs some time to send HeadData again
        micron.waitForPacket(sea_net::mtHeadData,1000);
    }
    catch(std::runtime_error e)
    {
        std::cerr << "Cannot start the device!" << std::endl;
        std::cerr << e.what() << std::endl;
        return false;
    }
    return true;
}

void Micron::updateHook()
{
    //check if the configuration has changed 
    if(current_config != _config.get())
    {
        try
        {
            std::cout << "Reconfigure during operation!" << std::endl;
            micron.configure(_config.get(),_configure_timeout.get()*1000);
            current_config = _config.get();
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
            iodrivers_base::Timeout time_out(_read_timeout.get()*1000);
            sea_net::PacketType packet_type = sea_net::mtNull;
            while(packet_type != sea_net::mtHeadData)
            {
                packet_type = micron.readPacket(time_out.timeLeft());
                switch(packet_type)
                {
                case sea_net::mtHeadData:
                    {
                        base::samples::SonarBeam sonar_beam;
                        micron.decodeSonarBeam(sonar_beam);
                        _sonar_beam.write(sonar_beam);
                    }
                default:
                    break;
                }
            }
            if(time_out.elapsed())
                throw std::runtime_error("");       //got to the catch block
        }
        catch(std::runtime_error e)
        {
            std::cerr << "Cannot read sonar beams!" << std::endl;
            std::cerr << e.what() << std::endl;
            return exception(IO_ERROR);
        }
    }

    //trigger 
    getActivity()->trigger();
}


void Micron::stopHook()
{
    micron.stop();
}

void Micron::cleanupHook()
{
    micron.close();
}

// void Micron::errorHook()
// {
// }


//void Micron::processDepth(base::Time const& time, double value){
//	sensorData::GroundDistanceReading groundData;
//	groundData.stamp = time;
//	groundData.depth = value;
//	_CurrentGroundDistance.write(groundData);
//}

//	const GroundDistance *gd = dynamic_cast<const GroundDistance*>(s);
//	if(gd){
//            base::samples::RigidBodyState water_depth;
//            water_depth.invalidate();
//            water_depth.time = gd->time;
//            water_depth.position[2] = gd->distance;
//            water_depth.cov_position(2,2) = 0.2;
//            _ground_distance.write(water_depth);
//        }
//}

