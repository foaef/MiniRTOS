#include "ReadSensorsAction.hpp"

#include "EBike.hpp"

void ReadSensorsAction::run(EBike& bike) const { bike.readSensors(); }

std::string ReadSensorsAction::name() const { return "read_sensors"; }
