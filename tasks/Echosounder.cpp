/* Generated from orogen/lib/orogen/templates/tasks/Task.cpp */

#include "Echosounder.hpp"
#include "sonar_tritech/SeaNetEchosounder.hpp"

using namespace sonar_tritech;

Echosounder::Echosounder(std::string const& name)
    : EchosounderBase(name)
{
    driver = new sea_net::Echosounder();
}

Echosounder::Echosounder(std::string const& name, RTT::ExecutionEngine* engine)
    : EchosounderBase(name, engine)
{
    driver = new sea_net::Echosounder();
}

Echosounder::~Echosounder()
{
    delete driver;
}



/// The following lines are template definitions for the various state machine
// hooks defined by Orocos::RTT. See Echosounder.hpp for more detailed
// documentation about them.

bool Echosounder::configureHook()
{
    if (! EchosounderBase::configureHook())
        return false;
    driver->openSerial(_port.get(),_baudrate.get());
    return true;
}
bool Echosounder::startHook()
{
    if (! EchosounderBase::startHook())
        return false;
    return true;
}
void Echosounder::updateHook()
{
    EchosounderBase::updateHook();
    try{                   
    base::samples::RigidBodyState state;
    if(driver->getDistance(state,_echo_sounder_timeout.get())){
        state.sourceFrame = _ground_frame.get();
        _ground_distance.write(state);
    }
            
    }catch(std::runtime_error e){
            std::cerr << "Cannot read sonar beams!" << std::endl;
            std::cerr << e.what() << std::endl;
            return exception(IO_ERROR);
    }
    
    //trigger
    getActivity()->trigger();
}

// void Echosounder::errorHook()
// {
//     EchosounderBase::errorHook();
// }
// void Echosounder::stopHook()
// {
//     EchosounderBase::stopHook();
// }
// void Echosounder::cleanupHook()
// {
//     EchosounderBase::cleanupHook();
// }

