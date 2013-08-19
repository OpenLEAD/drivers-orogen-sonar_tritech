#include "Micron.hpp"

using namespace sonar_tritech;

Micron::Micron(std::string const& name)
: MicronBase(name),time_out_echo_sounder(0)
{
}


bool Micron::setConfig(::sea_net::MicronConfig const & value)
{
        if(!isRunning()){
            std::cout << "Got Configuration even the device is not running (yet) cacheing it!" << std::endl;
        }else{
            try
            {
                std::cout << "Reconfigure during operation!" << std::endl;
                micron.configure(_config.get(),_configure_timeout.get()*1000);
            }
            catch(std::runtime_error e)
            {
                std::cerr << "Cannot reconfigure the device!" << std::endl;
                std::cerr << e.what() << std::endl;
                return false;
    //            return exception(IO_ERROR);
            }
        }

  	//Call the base function, DO-NOT Remove
	return(sonar_tritech::MicronBase::setConfig(value));
}

bool Micron::configureHook()
{
    micron.setWriteTimeout(1000*_write_timeout.get());
    try
    {
        micron.openSerial(_port.value(), _baudrate.value());
        micron.configure(_config.get(),_configure_timeout.get()*1000);

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

    time_out_echo_sounder = new iodrivers_base::Timeout(_echo_sounder_timeout.get()*1000);
    return true;
}

void Micron::updateHook()
{
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
                        break;
                    }
                case sea_net::mtAuxData:
                    {
                        base::samples::RigidBodyState state;
                        micron.decodeEchoSounder(state);
                        state.sourceFrame = _ground_frame.get();
                        _ground_distance.write(state);
                        time_out_echo_sounder->restart();
                        break;
                    }
                default:
                    break;
                }
            }
            if(time_out.elapsed())
                throw std::runtime_error("Time for reading mtHeadData elapsed.");       //got to the catch block
            if(_echo_sounder_timeout.get() > 0 && time_out_echo_sounder->elapsed())
                throw std::runtime_error("Time for reading mtAuxData elapsed.");       //got to the catch block
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
    time_out_echo_sounder = 0;
    delete time_out_echo_sounder;
    time_out_echo_sounder = NULL;
}
