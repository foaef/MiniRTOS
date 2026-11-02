#include "Domain.h"
#include <cmath>
#include <algorithm>

namespace ebike {

NoiseSource::NoiseSource(std::uint64_t seed) : state_(seed) {}

double NoiseSource::next() {
    state_ = state_ * 6364136223846793005ULL + 1442695040888963407ULL;
    const double unit = static_cast<double>(state_ >> 11U) / 9007199254740992.0;
    return unit * 2.0 - 1.0;
}

std::ostream& operator<<(std::ostream& os, const NoiseSource& source) {
    return os << "NoiseSource{state=" << source.state_ << "}";
}

SensorSample::SensorSample(Tick tick, double speedKmh, double pedalPowerW, double batteryV, double currentA,
                           double motorTempC, bool brake)
    : tick_(tick),
      speedKmh_(speedKmh),
      pedalPowerW_(pedalPowerW),
      batteryV_(batteryV),
      currentA_(currentA),
      motorTempC_(motorTempC),
      brake_(brake) {}

std::ostream& operator<<(std::ostream& os, const SensorSample& s) {
    return os << "Sample{t=" << s.tick_ << ", " << Format::fixed(s.speedKmh_, 1) << " km/h, pedal "
              << Format::fixed(s.pedalPowerW_, 0) << " W, "
              << Format::fixed(s.batteryV_, 2) << " V, " << Format::fixed(s.currentA_, 2) << " A, motor "
              << Format::fixed(s.motorTempC_, 1) << " C" << (s.brake_ ? ", BRAKE" : "") << "}";
}

PidController::PidController(double kp, double ki, double kd, double outMin, double outMax)
    : kp_(kp), ki_(ki), kd_(kd), outMin_(outMin), outMax_(outMax) {
    if (outMin_ >= outMax_) {
        throw ConfigError("PID output range is empty");
    }
}

double PidController::update(double setpoint, double measured, double dtSeconds) {
    const double error = setpoint - measured;
    integral_ += error * dtSeconds;
    if (ki_ > 0.0) {
        integral_ = std::clamp(integral_, outMin_ / ki_, outMax_ / ki_);
    }
    const double derivative = primed_ && dtSeconds > 0.0 ? (error - previousError_) / dtSeconds : 0.0;
    previousError_ = error;
    primed_ = true;
    return std::clamp(kp_ * error + ki_ * integral_ + kd_ * derivative, outMin_, outMax_);
}

void PidController::reset() {
    integral_ = 0.0;
    previousError_ = 0.0;
    primed_ = false;
}

std::ostream& operator<<(std::ostream& os, const PidController& pid) {
    return os << "PID{kp=" << pid.kp_ << ", ki=" << pid.ki_ << ", kd=" << pid.kd_ << ", integral="
              << Format::fixed(pid.integral_, 3) << "}";
}

Battery::Battery(int cellsSeries, double capacityMah, double cellFullV, double cellEmptyV, double internalResistance,
                 double initialSoc)
    : cellsSeries_(cellsSeries),
      capacityAh_(capacityMah / 1000.0),
      cellFullV_(cellFullV),
      cellEmptyV_(cellEmptyV),
      internalResistance_(internalResistance),
      chargeAh_(capacityMah / 1000.0 * initialSoc) {
    if (cellsSeries_ <= 0 || capacityMah <= 0.0 || internalResistance_ <= 0.0) {
        throw ConfigError("battery needs positive cell count, capacity and resistance");
    }
    if (cellFullV_ <= cellEmptyV_) {
        throw ConfigError("battery full voltage must exceed empty voltage");
    }
    if (initialSoc < 0.0 || initialSoc > 1.0) {
        throw ConfigError("battery initial state of charge must be within [0, 1]");
    }
}

double Battery::openCircuitVoltage() const {
    const double cell = cellEmptyV_ + (cellFullV_ - cellEmptyV_) * stateOfCharge();
    return cell * cellsSeries_;
}

double Battery::terminalVoltage(double currentA) const {
    return openCircuitVoltage() - currentA * internalResistance_;
}

double Battery::remainingWh() const { return chargeAh_ * openCircuitVoltage(); }

double Battery::currentForPower(double powerW) const {
    if (powerW <= 0.0) {
        return 0.0;
    }
    const double v0 = openCircuitVoltage();
    const double discriminant = v0 * v0 - 4.0 * internalResistance_ * powerW;
    if (discriminant < 0.0) {
        return v0 / (2.0 * internalResistance_);
    }
    return (v0 - std::sqrt(discriminant)) / (2.0 * internalResistance_);
}

void Battery::discharge(double currentA, double dtSeconds) {
    chargeAh_ = std::max(0.0, chargeAh_ - currentA * dtSeconds / 3600.0);
}

double Battery::estimatedRangeKm(double whPerKm) const {
    return whPerKm > 0.0 ? remainingWh() / whPerKm : 0.0;
}

std::ostream& operator<<(std::ostream& os, const Battery& battery) {
    return os << "Battery{" << battery.cellsSeries_ << "S, " << Format::percent(battery.stateOfCharge()) << ", "
              << Format::fixed(battery.openCircuitVoltage(), 1) << " V, " << Format::fixed(battery.remainingWh(), 0)
              << " Wh}";
}

double RiderProfile::wave(Tick now, Tick period) {
    return std::sin(kTwoPi * static_cast<double>(now % period) / static_cast<double>(period));
}

RiderProfile::RiderProfile(double basePowerW, double swingW, Tick powerPeriod, double gradientAmplitudePct,
                           Tick gradientPeriod, Tick brakeEvery, Tick brakeDuration)
    : basePowerW_(basePowerW),
      swingW_(swingW),
      powerPeriod_(powerPeriod),
      gradientAmplitudePct_(gradientAmplitudePct),
      gradientPeriod_(gradientPeriod),
      brakeEvery_(brakeEvery),
      brakeDuration_(brakeDuration) {
    if (powerPeriod_ == 0 || gradientPeriod_ == 0) {
        throw ConfigError("rider periods must be positive");
    }
    if (basePowerW_ <= 0.0) {
        throw ConfigError("rider base power must be positive");
    }
}

double RiderProfile::pedalPowerAt(Tick now) const {
    return std::max(0.0, basePowerW_ + swingW_ * wave(now, powerPeriod_));
}

double RiderProfile::gradientAt(Tick now) const { return gradientAmplitudePct_ * wave(now, gradientPeriod_); }

bool RiderProfile::brakingAt(Tick now) const {
    return brakeEvery_ > 0 && now >= brakeEvery_ && now % brakeEvery_ < brakeDuration_;
}

std::ostream& operator<<(std::ostream& os, const RiderProfile& rider) {
    return os << "Rider{" << Format::fixed(rider.basePowerW_, 0) << " W +/- " << Format::fixed(rider.swingW_, 0)
              << ", brakes every " << rider.brakeEvery_ << " ticks}";
}

BikeDynamics::BikeDynamics(double massKg, double rollingResistance, double dragArea, double brakeDecel, double initialSpeedKmh)
    : massKg_(massKg),
      rollingResistance_(rollingResistance),
      dragArea_(dragArea),
      brakeDecel_(brakeDecel),
      speedMs_(initialSpeedKmh / 3.6) {
    if (massKg_ <= 0.0) {
        throw ConfigError("bike mass must be positive");
    }
}

void BikeDynamics::setRiderInput(double pwr, double gradientPct, bool braking) {
    pedalPowerW_ = pwr;
    gradientPct_ = gradientPct;
    braking_ = braking;
}

void BikeDynamics::step(double dtSeconds, double motorPowerW) {
    const double driveForce = (pedalPowerW_ + motorPowerW) / std::max(speedMs_, kMinSpeedMs);
    const double slope = std::atan(gradientPct_ / 100.0);
    const double resistance = massKg_ * kGravity * (rollingResistance_ * std::cos(slope) + std::sin(slope)) +
                              0.5 * kAirDensity * dragArea_ * speedMs_ * speedMs_;
    double acceleration = (driveForce - resistance) / massKg_;
    if (braking_) {
        acceleration -= brakeDecel_;
    }
    speedMs_ = std::max(0.0, speedMs_ + acceleration * dtSeconds);
}

std::ostream& operator<<(std::ostream& os, const BikeDynamics& bike) {
    return os << "Dynamics{" << Format::fixed(bike.speedKmh(), 1) << " km/h, grade " << Format::fixed(bike.gradientPct_, 1) << "%}";
}

AssistTable::AssistTable(const BikeConfig& config)
    : ratios_{config.number("assist.off"), config.number("assist.eco"), config.number("assist.tour"),
              config.number("assist.sport"), config.number("assist.turbo")} {}

AssistLevel AssistTable::next(AssistLevel level) {
    return static_cast<AssistLevel>((static_cast<std::size_t>(level) + 1) % kLevels);
}

AssistLevel AssistTable::fromIndex(Tick index) {
    if (index >= kLevels) {
        throw ConfigError("assist.initial must be between 0 and 4");
    }
    return static_cast<AssistLevel>(index);
}

const char* AssistTable::name(AssistLevel level) {
    switch (level) {
        case AssistLevel::Off: return "OFF";
        case AssistLevel::Eco: return "ECO";
        case AssistLevel::Tour: return "TOUR";
        case AssistLevel::Sport: return "SPORT";
        case AssistLevel::Turbo: return "TURBO";
    }
    return "?";
}

std::ostream& operator<<(std::ostream& os, const AssistTable& table) {
    os << "AssistTable{";
    for (std::size_t i = 0; i < AssistTable::kLevels; ++i) {
        os << (i == 0 ? "" : ", ") << AssistTable::name(static_cast<AssistLevel>(i)) << "=" << Format::fixed(table.ratios_[i], 2);
    }
    return os << "}";
}

MotorController::MotorController(PidController pid, double maxPowerW, double cutoffKmh, double fadeKmh, double derateStartC,
                                 double limitC, double heatRate, double coolRate, double responsePerS, double ambientC)
    : pid_(pid),
      maxPowerW_(maxPowerW),
      cutoffKmh_(cutoffKmh),
      fadeKmh_(fadeKmh),
      derateStartC_(derateStartC),
      limitC_(limitC),
      heatRate_(heatRate),
      coolRate_(coolRate),
      responsePerS_(responsePerS),
      ambientC_(ambientC),
      tempC_(ambientC) {
    if (maxPowerW_ <= 0.0 || fadeKmh_ <= 0.0) {
        throw ConfigError("motor max power and speed fade range must be positive");
    }
    if (derateStartC_ >= limitC_) {
        throw ConfigError("motor derate start must be below the temperature limit");
    }
}

double MotorController::speedFactor(double speedKmh) const {
    return std::clamp((cutoffKmh_ - speedKmh) / fadeKmh_, 0.0, 1.0);
}

double MotorController::thermalFactor() const {
    return std::clamp((limitC_ - tempC_) / (limitC_ - derateStartC_), 0.0, 1.0);
}

double MotorController::targetPower(const SensorSample& sample, double assistRatio, double batteryFactor) const {
    const double wanted = assistRatio * sample.pedalPowerW() * speedFactor(sample.speedKmh()) * thermalFactor() *
                          batteryFactor;
    return std::clamp(wanted, 0.0, maxPowerW_);
}

void MotorController::regulate(double targetPowerW, double dtSeconds) {
    duty_ = pid_.update(targetPowerW, powerW_, dtSeconds);
}

void MotorController::advance(double dtSeconds) {
    powerW_ += (duty_ * maxPowerW_ - powerW_) * std::min(1.0, responsePerS_ * dtSeconds);
    const double load = powerW_ / maxPowerW_;
    tempC_ += (heatRate_ * load * load - coolRate_ * (tempC_ - ambientC_)) * dtSeconds;
}

void MotorController::cutPower() {
    duty_ = 0.0;
    powerW_ = 0.0;
    pid_.reset();
}

std::ostream& operator<<(std::ostream& os, const MotorController& motor) {
    return os << "Motor{" << Format::fixed(motor.powerW_, 0) << "/" << Format::fixed(motor.maxPowerW_, 0) << " W, duty "
              << Format::percent(motor.duty_) << ", " << Format::fixed(motor.tempC_, 1) << " C, " << motor.pid_ << "}";
}

SensorHub::SensorHub(std::uint64_t seed, double speedNoiseKmh, double powerNoiseW, double voltageNoiseV)
    : noise_(seed), speedNoiseKmh_(speedNoiseKmh), powerNoiseW_(powerNoiseW), voltageNoiseV_(voltageNoiseV) {}

SensorSample SensorHub::read(Tick now, const BikeDynamics& bike, const Battery& battery,
                              const MotorController& motor, const RiderProfile& rider) {
    const double speed = std::max(0.0, bike.speedKmh() + speedNoiseKmh_ * noise_.next());
    const double pedalPower = std::max(0.0, bike.pedalPowerW() + powerNoiseW_ * noise_.next());
    const double current = battery.currentForPower(motor.powerW());
    const double voltage = battery.terminalVoltage(current) + voltageNoiseV_ * noise_.next();
    return {now, speed, pedalPower, voltage, current, motor.temperatureC(), rider.brakingAt(now)};
}

std::ostream& operator<<(std::ostream& os, const SensorHub& hub) {
    return os << "SensorHub{" << hub.noise_ << ", speed +/-" << hub.speedNoiseKmh_ << " km/h}";
}

void Display::showAlert(const std::string& text, Tick until) {
    alert_ = text;
    alertUntil_ = until;
}

void Display::render(const SensorSample& sample, double soc, AssistLevel level, double rangeKm, Tick now) {
    topLine_ = Format::fixed(sample.speedKmh(), 1) + " km/h  " + AssistTable::name(level);
    if (now < alertUntil_) {
        bottomLine_ = "! " + alert_;
    } else {
        bottomLine_ = "SOC " + Format::percent(soc) + "  RNG " + Format::fixed(rangeKm, 0) + " km";
    }
    ++frames_;
}

std::ostream& operator<<(std::ostream& os, const Display& display) {
    return os << "Display{frames=" << display.frames_ << " | " << display.topLine_ << " | " << display.bottomLine_
              << "}";
}

void RideStats::accumulate(double speedKmh, double batteryPowerW, double dtSeconds) {
    distanceKm_ += speedKmh * dtSeconds / 3600.0;
    elapsedS_ += dtSeconds;
    maxSpeedKmh_ = std::max(maxSpeedKmh_, speedKmh);
    batteryEnergyWh_ += batteryPowerW * dtSeconds / 3600.0;
}

double RideStats::averageSpeedKmh() const { return elapsedS_ > 0.0 ? distanceKm_ * 3600.0 / elapsedS_ : 0.0; }

double RideStats::whPerKm(double minimumKm) const {
    return distanceKm_ >= minimumKm ? batteryEnergyWh_ / distanceKm_ : 0.0;
}

std::ostream& operator<<(std::ostream& os, const RideStats& stats) {
    return os << "RideStats{" << Format::fixed(stats.distanceKm_, 3) << " km, avg "
              << Format::fixed(stats.averageSpeedKmh(), 1) << " km/h, max " << Format::fixed(stats.maxSpeedKmh_, 1)
              << " km/h, " << Format::fixed(stats.batteryEnergyWh_, 2) << " Wh}";
}

}
