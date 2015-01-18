/* Generated from orogen/lib/orogen/templates/tasks/Task.hpp */

#ifndef SONAR_DRIVER_PROFILING_TASK_HPP
#define SONAR_DRIVER_PROFILING_TASK_HPP

#include "sonar_tritech/ProfilingBase.hpp"
#include <sonar_tritech/SeaNetProfiling.hpp>
#include <iodrivers_base/Timeout.hpp>

namespace sonar_tritech {
    class Profiling : public ProfilingBase
    {
    protected:
        sea_net::Profiling profiling;
        base::samples::LaserScan laser_scan;
        
        /* Dynamic Property setter of config
         */
        virtual bool setAcquisition_config(::sea_net::ProfilingAcquisitionConfig const & value);

        /** Push the last received scan out of the driver onto the laser scan
         * port
         *
         * It assumes that the driver just received a scan (i.e. this is usually
         * called just after receiveData()
         */
        void pushProfile();
        
    public:
        Profiling(std::string const& name = "sonar_tritech::Profiling");

        /** This hook is called by Orocos when the state machine transitions
         * from PreOperational to Stopped. If it returns false, then the
         * component will stay in PreOperational. Otherwise, it goes into
         * Stopped.
         *
         * It is meaningful only if the #needs_configuration has been specified
         * in the task context definition with (for example):
         *
         *   task_context "TaskName" do
         *     needs_configuration
         *     ...
         *   end
         */
        bool configureHook();

        /** This hook is called by Orocos when the state machine transitions
         * from Stopped to Running. If it returns false, then the component will
         * stay in Stopped. Otherwise, it goes into Running and updateHook()
         * will be called.
         */
        bool startHook();

        void processIO();

        /** This hook is called by Orocos when the component is in the
         * RunTimeError state, at each activity step. See the discussion in
         * updateHook() about triggering options.
         *
         * Call recovered() to go back in the Runtime state.
         */
        // void errorHook();

        /** This hook is called by Orocos when the state machine transitions
         * from Running to Stopped after stop() has been called.
         */
        void stopHook();

        /** This hook is called by Orocos when the state machine transitions
         * from Stopped to PreOperational, requiring the call to configureHook()
         * before calling start() again.
         */
        // void cleanupHook();

    private:
        bool hasAcquisitionTimeout;
        iodrivers_base::Timeout timeoutAcquisition;
    };
}

#endif

