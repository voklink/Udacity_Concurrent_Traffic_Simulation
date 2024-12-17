#include <chrono>
#include <future>
#include <iostream>
#include <random>
#include <sys/types.h>
#include <thread>
#include <queue>

#include "TrafficLight.h"
/* Implementation of class "MessageQueue" */

template <typename T>
T MessageQueue<T>::receive()
{
    // DONE
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function. 
    
    std::unique_lock<std::mutex> myUniqueLock(_mutex);
    _condition.wait(myUniqueLock, [this] { return !_queue.empty(); });

    T message = std::move(_queue.back());
    _queue.pop_back();
    
    return message;
    }

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // DONE    
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.

    std::lock_guard<std::mutex> myLockGuard(_mutex);
    _queue.clear( );
    _queue. emplace_back(std::move(msg));
    _condition.notify_one();
}


/* Implementation of class "TrafficLight" */


TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    // DONE
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
    
    while (true) 
    {
        TrafficLightPhase currentPhase = _messageQueue.receive();

        if (currentPhase == TrafficLightPhase::green)
        {
            break;
        }
    }
    return;   
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class. 
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));

}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    //DONE
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 

    // Random Float generation taken from ChatGpt!
    // ---- START -------
    // Create a random device and a Mersenne Twister random number generator
        std::random_device rd;
        std::mt19937 gen(rd());

        // Define the range [4, 6]
        std::uniform_real_distribution<> dis(4000, 6000);

        // Generate a random float number in the range [4, 6]
        float randomDuration = dis(gen);
        // std::cout << "Random time: " << randomDuration << std::endl;
    // ----END ------
    

    // Init of StopWatch
    constexpr uint SLEEP_DURATION_MS {100};     // I reduce the load a bit more...1ms seems like overkill
    std::chrono::time_point<std::chrono::system_clock> startTime{};
    std::chrono::time_point<std::chrono::system_clock> stopTime{};
    int elapsedTime{};
    
    startTime = std::chrono::system_clock::now();

    while (true) 
    {
        // Sleep to reduce CPU load
        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_DURATION_MS));
        stopTime    = std::chrono::system_clock::now();
        elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(stopTime - startTime).count();
        
        if (elapsedTime > randomDuration) 
        {  
            // toggle TrafficLightPhase
            _currentPhase = _currentPhase == TrafficLightPhase::red? TrafficLightPhase::green : TrafficLightPhase::red;
         
            // send update
            TrafficLightPhase message = TrafficLight::getCurrentPhase();
            _messageQueue.send(std::move(message)); 
            
            // reset the random duration of the traffic light to have some variance
            randomDuration  = dis(gen);
            startTime       = std::chrono::system_clock::now();
        }
    }
}


