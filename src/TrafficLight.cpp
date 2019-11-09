#include <iostream>
#include <random>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */

template <typename T>
T MessageQueue<T>::receive()
{
    // DONE: FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function. 
    std::unique_lock<std::mutex> uLock(_mutex); //Acquire Lock 
    _cond.wait(uLock, [this](){ return !_queue.empty();}); // Wait until _queue is NOT empty

    T msg = std::move(_queue.back()); //get the message from the back of _queue
    _queue.pop_back(); //pop or remove the message from the _queue
    std::cout << "Retrieved message " << msg << "from back of queue" << std::endl;
    return msg;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // DONE: FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
    std::lock_guard<std::mutex> uLock(_mutex);

    std::cout << "Adding message " << msg << "to back of queue" << std::endl;
    _queue.push_back(std::move(msg));
    _cond.notify_one();
}


/* Implementation of class "TrafficLight" */
TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
    _msgQ = std::make_shared<MessageQueue<TrafficLightPhase>>();
}

void TrafficLight::waitForGreen()
{
    // DONE: FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
    while(true)
    {
        if(TrafficLightPhase::green == _msgQ->receive())
        {
            std::cout << "Got green!" << std::endl;
            return;
        }
        else
        {
            std::cout << "Waiting for green!" << std::endl;
        }        
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // DONE: FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method 
    // „simulate“ is called. To do this, use the thread queue in the base class. 
    // launch cycleThroughPhases function in a thread
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 
    
    std::random_device rd;  //Will be used to obtain a seed for the random number engine
    std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
    std::uniform_real_distribution<> dis(4.0, 6.0);
    //std::unique_lock<std::mutex> uLock(_mutex);
    //uLock.unlock();
    double cycleDuration = dis(gen); //initialize randomly
    auto lastUpdate = std::chrono::system_clock::now(); //current time

    while (true) //loop forever
    {
        //calculate time difference between loops
        std::chrono::duration<double> timeDifference = std::chrono::system_clock::now() - lastUpdate;
        double timeSinceLastUpdate = timeDifference.count();

        //if time elapsed is greater than cycleDuration
        if(timeSinceLastUpdate > cycleDuration)
        {
            //toggle light
            if(_currentPhase == TrafficLightPhase::red)
            {
                _currentPhase = TrafficLightPhase::green;                
            }
            else
            {
                _currentPhase = TrafficLightPhase::red;
            }
            _msgQ->send(std::move(_currentPhase));       
            lastUpdate = std::chrono::system_clock::now();    
        }
        cycleDuration = dis(gen); //randomly change cycle duration
    /*  //My first simple approach to toggle with random delay
        //Keeping it as I think it's better than above required implementation
        std::this_thread::sleep_for(std::chrono::milliseconds(1));//wait 1ms between two cycles
        //uLock.lock();
        _currentPhase = TrafficLightPhase::red;
        //uLock.unlock();
        _msgQ->send(std::move(_currentPhase));
        std::this_thread::sleep_for(std::chrono::duration<double>(dis(gen)));
        _currentPhase = TrafficLightPhase::green;
        _msgQ->send(std::move(_currentPhase));
    */
    }    
}

//overloading << for TrafficLightPhase enum
std::ostream& operator<<(std::ostream& os, TrafficLightPhase c)
{
    switch(c)
    {
        case red   : os << "red ";    break;
        case green : os << "green ";  break;
        default    : os.setstate(std::ios_base::failbit);
    }
    return os;
}