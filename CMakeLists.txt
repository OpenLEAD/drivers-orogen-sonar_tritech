PROJECT(sonar_driver)
cmake_minimum_required(VERSION 2.6)

INCLUDE(FindPkgConfig)
PKG_CHECK_MODULES(MOICRON REQUIRED "micron")

SET (CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/.orogen/config")
INCLUDE(sonar_driverBase)

# FIND_PACKAGE(KDL)
# FIND_PACKAGE(OCL)

