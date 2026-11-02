#include <iostream>
#include <stdexcept>
#include "Example.h"
#include "Task.h"
#include "TaskQueue.h"
#include "Timer.h"
#include "Scheduler.h"

int main() {
    try {
        std::cout << "=== MiniRTOS for Raspberry Pi Pico 2 Simulation ===\n\n";
        
        
        std::cout << "--- Example Demo ---\n";
        Example e1;
        e1.add(3);
        e1.g();
        std::cout << "\n";
        
        
        std::cout << "--- Task Creation ---\n";
        Task sensorTask(1, "SensorRead", 150, 512);
        Task controlTask(2, "PIDControl", 200, 1024);
        Task displayTask(3, "DisplayUpdate", 100, 256);
        Task idleTask(4, "IdleTask", 50, 128);
        
        std::cout << sensorTask << "\n";
        std::cout << controlTask << "\n";
        std::cout << displayTask << "\n";
        std::cout << idleTask << "\n\n";
        
        
        std::cout << "--- Task Operations ---\n";
        std::cout << "sensorTask priority: " << sensorTask.getPriority() << "\n";
        std::cout << "sensorTask can preempt idleTask? " << (sensorTask.canPreempt(idleTask) ? "Yes" : "No") << "\n";
        std::cout << "controlTask has higher priority than sensorTask? " << (controlTask.hasHigherPriorityThan(sensorTask) ? "Yes" : "No") << "\n";
        sensorTask.executeTick();
        std::cout << "After executeTick: " << sensorTask << "\n";
        sensorTask.yield();
        std::cout << "After yield: " << sensorTask << "\n\n";
        
        
        std::cout << "--- TaskQueue Demo ---\n";
        TaskQueue readyQ("HighPriorityQueue");
        readyQ.enqueue(sensorTask);
        readyQ.enqueue(controlTask);
        readyQ.enqueue(displayTask);
        readyQ.enqueue(idleTask);
        
        std::cout << readyQ << "\n";
        readyQ.sortByPriority();
        std::cout << "After sorting by priority:\n" << readyQ << "\n";
        
        std::cout << "Queue contains task with ID 2? " << (readyQ.containsTask(2) ? "Yes" : "No") << "\n";
        std::cout << "Promoting task 3: " << (readyQ.findAndPromoteTask(3, 180) ? "Success" : "Failed") << "\n";
        readyQ.sortByPriority();
        std::cout << "After promotion:\n" << readyQ << "\n\n";
        
        
        std::cout << "--- Timer Demo ---\n";
        Timer sysTimer(10);
        sysTimer.start();
        std::cout << sysTimer << "\n";
        sysTimer.scheduleCallback(50);
        sysTimer.scheduleCallback(30);
        std::cout << "After scheduling callbacks:\n" << sysTimer << "\n";
        for (int i = 0; i < 6; ++i) {
            sysTimer.tick();
        }
        sysTimer.processCallbacks();
        std::cout << "After ticking 6 times:\n" << sysTimer << "\n";
        
        
        Timer timerCopy = sysTimer; // copy constructor
        Timer timerAssign;
        timerAssign = sysTimer; // copy assignment
        std::cout << "Timer copy: " << timerCopy << "\n";
        std::cout << "Timer assigned: " << timerAssign << "\n\n";
        
        
        std::cout << "--- Scheduler Demo ---\n";
        Scheduler rtos(SchedulerMode::PRIORITY_BASED, 16);
        rtos.attachTimer(&sysTimer);
        rtos.start();
        
        rtos.addTask(Task(10, "UARTHandler", 175, 768));
        rtos.addTask(Task(11, "ADCSampler", 190, 512));
        rtos.addTask(Task(12, "NetworkStack", 120, 2048));
        
        std::cout << rtos << "\n";
        
        std::cout << "Running scheduler for 15 ticks...\n";
        rtos.runForTicks(15);
        std::cout << rtos << "\n";
        
        rtos.printStatistics();
        rtos.stop();
        
        
        std::cout << "--- Round Robin Scheduler Demo ---\n";
        Scheduler rrScheduler(SchedulerMode::PREEMPTIVE_ROUND_ROBIN);
        rrScheduler.start();
        rrScheduler.addTask(Task(20, "TaskA", 100, 256));
        rrScheduler.addTask(Task(21, "TaskB", 100, 256));
        rrScheduler.addTask(Task(22, "TaskC", 100, 256));
        rrScheduler.runForTicks(12);
        std::cout << rrScheduler << "\n";
        rrScheduler.printStatistics();
        
        
        std::cout << "\n--- Keyboard Input Demo (from tastatura.txt) ---\n";
        int nrTasks;
        std::cout << "Enter number of custom tasks to simulate: ";
        std::cin >> nrTasks;
        std::cout << "Simulating " << nrTasks << " additional task operations...\n";
        for (int i = 0; i < nrTasks && i < 5; ++i) {
            int prio;
            std::cout << "Enter priority for task " << (30 + i) << ": ";
            std::cin >> prio;
            Task customTask(30 + i, "CustomTask", static_cast<unsigned int>(prio), 512);
            std::cout << "  Created: " << customTask.getName() 
                      << " with priority " << customTask.getPriority() << "\n";
        }
        
        std::cout << "\n=== MiniRTOS Simulation Complete ===\n";
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    
    return 0;
}
