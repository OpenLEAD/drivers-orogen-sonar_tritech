#include "Micron.hpp"

using namespace sonar_tritech;

Micron::Micron(std::string const& name)
: MicronBase(name),hasEchoSounderTimeout(false),timeoutEchoSounder(0)
{
}


bool Micron::setConfig(::sea_net::MicronConfig const & value)
{
    // Need to read the pending data packet first
    if (micron.hasPendingData())
        micron.receiveData(_io_read_timeout.get().toMilliseconds());

    micron.configure(value, _configure_timeout.get()*1000);

    //Call the base function, DO-NOT Remove
    return(sonar_tritech::MicronBase::setConfig(value));
}

bool Micron::configureHook()
{
    if (!_port.value().empty())
        micron.openSerial(_port.value(), _baudrate.value());
    else if (!_io_port.value().empty())
        micron.openURI(_io_port);
    setDriver(&micron);

    if (!MicronBase::configureHook())
        return false;

    micron.configure(_config.get(), _configure_timeout.get()*1000);

    //check if full duplex is set
    //if not the user has to set it via tritech software
    if(!micron.isFullDuplex(1000))
        std::cout << "WARNING: Micron is not using Full Duplex" << std::endl;

    return true;
}

bool Micron::startHook()
{
    //Wait up to one second. This is needed because the
    //motor of the sonar is powering down after a while
    //and it needs some time to send HeadData again
    micron.requestData();
    micron.receiveData(1000);

    int echoSounderTimeout = _echo_sounder_timeout.get();
    hasEchoSounderTimeout = (echoSounderTimeout != 0);
    timeoutEchoSounder =
        iodrivers_base::Timeout(_echo_sounder_timeout.get()*1000);

    base::Time acquisitionTimeout = _acquisition_timeout.get();
    hasAcquisitionTimeout = !acquisitionTimeout.isNull();
    timeoutAcquisition =
        iodrivers_base::Timeout(acquisitionTimeout.toMilliseconds());

    // Start pulling
    micron.requestData();
    return MicronBase::startHook();
}

void Micron::processIO()
{
    sea_net::PacketType packet_type = micron.readPacket(_io_read_timeout.get().toMilliseconds());
    if (packet_type == sea_net::mtHeadData)
    {
        base::samples::SonarBeam sonar_beam;
        micron.decodeSonarBeam(sonar_beam);
        _sonar_beam.write(sonar_beam);
        micron.requestData();
        timeoutAcquisition.restart();
    }
    else if (packet_type == sea_net::mtAuxData)
    {
        processEchoSounderPacket();
    }
    else if (hasAcquisitionTimeout && timeoutAcquisition.elapsed())
    {
        micron.requestData();
        timeoutAcquisition.restart();
    }

    if (hasEchoSounderTimeout && timeoutEchoSounder.elapsed())
        exception(ECHO_SOUNDER_TIMEOUT);
}

void Micron::processEchoSounderPacket()
{
    base::samples::RigidBodyState state;
    micron.decodeEchoSounder(state);
    state.sourceFrame = _ground_frame.get();
    _ground_distance.write(state);
    timeoutEchoSounder.restart();
}

void Micron::updateHook()
{
    MicronBase::updateHook();
}

void Micron::stopHook()
{
    MicronBase::stopHook();
}

void Micron::cleanupHook()
{
    MicronBase::cleanupHook();
}

