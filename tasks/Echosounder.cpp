/* Generated from orogen/lib/orogen/templates/tasks/Task.cpp */

#include "Echosounder.hpp"

using namespace sonar_tritech;

Echosounder::Echosounder(std::string const& name)
    : EchosounderBase(name)
{
}

Echosounder::Echosounder(std::string const& name, RTT::ExecutionEngine* engine)
    : EchosounderBase(name, engine)
{
}

Echosounder::~Echosounder()
{
}



/// The following lines are template definitions for the various state machine
// hooks defined by Orocos::RTT. See Echosounder.hpp for more detailed
// documentation about them.

// bool Echosounder::configureHook()
// {
//     if (! EchosounderBase::configureHook())
//         return false;
//     return true;
// }
// bool Echosounder::startHook()
// {
//     if (! EchosounderBase::startHook())
//         return false;
//     return true;
// }
// void Echosounder::updateHook()
// {
//     EchosounderBase::updateHook();
// }
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

