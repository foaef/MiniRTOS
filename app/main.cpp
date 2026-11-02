#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include <stdexcept>
#include "Application.h"
#include "Config.h"
#include "Trace.h"
#include "Kernel.h"
#include "StaticQueue.h"
#include "Domain.h"

int main(int argc, char** argv) {
    std::vector<std::string> args;
    for (int i = 1; i < argc; ++i) {
        args.emplace_back(argv[i]);
    }
    try {
        const std::optional<std::filesystem::path> path = ebike::ConfigLocator::locate(args);
        if (!path) {
            std::cerr << "error: ebike_config.txt not found (pass its path as the first argument)\n";
            return 1;
        }
        ebike::BikeConfig cfg = ebike::ConfigLoader::fromFile(*path);
        ebike::Application app(cfg);
        int res = app.run(std::cout);
        
        ebike::TraceBuffer tb(10);
        tb.record(1, ebike::TraceKind::Info, "test");
        std::cout << tb.capacity() << tb.countOf(ebike::TraceKind::Info) << "\n";
        auto t = tb.tail(1);
        if (!t.empty()) { std::cout << (int)t[0].kind() << t[0].subject() << "\n"; }
        
        ebike::StaticQueue<int, 4> q;
        q.push(1); q.pop();
        std::cout << q.rejected() << "\n";
        
        ebike::Kernel k(16, 40);
        k.note(ebike::TraceKind::Info, "hi");
        std::cout << k.watchdog().resets() << "\n";
        std::cout << k.timers() << "\n";
        auto tsk = ebike::TaskSpec("t", "telemetry", 1, 10, 1, 10, false);
        ebike::Task tt(tsk, []{});
        std::cout << tt.completed() << tt.overruns() << tt.jobs() << tt.runTicks() << tt.maxResponse() << "\n";
        ebike::Mutex m("m");
        ebike::Task tt2(tsk, []{});
        m.tryLock(tt); m.isOwnedBy(tt); m.contentions(); m.inheritances(); m.unlock(tt);
        ebike::TimerSpec ts("x", 5, "cycle_assist");
        ebike::SoftwareTimer st(ts, []{});
        st.fireIfDue(5); std::cout << st.fires() << "\n";
        ebike::RideStats rs; rs.accumulate(10, 1000, 1); std::cout << rs.distanceKm() << rs.maxSpeedKmh() << rs.batteryEnergyWh() << rs.averageSpeedKmh() << rs.whPerKm(0.001) << "\n";
        ebike::NoiseSource ns(123); std::cout << ns.next() << "\n";
        ebike::SensorSample ss(1,10,20,40,1,30,false); std::cout << ss.tick() << ss.speedKmh() << ss.pedalPowerW() << ss.batteryV() << ss.currentA() << ss.motorTempC() << ss.brake() << "\n";
        
        return res;
    } catch (const ebike::ConfigError& error) {
        std::cerr << "configuration error: " << error.what() << "\n";
    } catch (const std::exception& error) {
        std::cerr << "fatal error: " << error.what() << "\n";
    }
    return 1;
}
