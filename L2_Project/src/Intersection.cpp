#include <iostream>
#include <thread>
#include <chrono>
#include <future>
#include <random>

#include "Street.h"
#include "Intersection.h"
#include "Vehicle.h"

/* Implementation of class "WaitingVehicles" */

int WaitingVehicles::getSize()
{
    return _vehicles.size();
}

void WaitingVehicles::pushBack(std::shared_ptr<Vehicle> vehicle, std::promise<void> &&promise)
{
    _vehicles.push_back(vehicle);
    _promises.push_back(std::move(promise));
}

void WaitingVehicles::permitEntryToFirstInQueue()
{
    // First, get the entries from the front of _promises and _vehicles.
    // vector::begin returns an iterator to the first element in the vector unlike vector::front which returns a direct reference
    auto promise = _promises.begin();
    auto vehicle = _vehicles.begin();
    // Then, fulfill promise and send signal back that permission to enter has been granted.
    // The set_value method on the promise sets the value that will be retrieved by the future
    promise->set_value(); // fulfill promise
    // Finally, remove the front elements from both queues.
    // vector::erase removes from the vector either a single element (iterator to position) or a range of elements ([first,last))
    _promises.erase(promise);
    _vehicles.erase(vehicle);
}

/* Implementation of class "Intersection" */

Intersection::Intersection()
{
    _type = ObjectType::objectIntersection;
    _isBlocked = false;
}

void Intersection::addStreet(std::shared_ptr<Street> street)
{
    _streets.push_back(street);
}

std::vector<std::shared_ptr<Street>> Intersection::queryStreets(std::shared_ptr<Street> incoming)
{
    // store all outgoing streets in a vector ...
    std::vector<std::shared_ptr<Street>> outgoings;
    for (auto it : _streets)
    {
        if (incoming->getID() != it->getID()) // ... except the street making the inquiry
        {
            outgoings.push_back(it);
        }
    }

    return outgoings;
}

// adds a new vehicle to the queue and returns once the vehicle is allowed to enter
void Intersection::addVehicleToQueue(std::shared_ptr<Vehicle> vehicle)
{
    std::cout << "Intersection #" << _id << "::addVehicleToQueue: thread id = " << std::this_thread::get_id() << std::endl;

    // First, add the new vehicle to the waiting line by creating a promise (and a corresponding future) and then adding both to _waitingVehicles.
    std::promise<void> vehiclePromise;
    std::future<void> vehicleResult = vehiclePromise.get_future();
    _waitingVehicles.pushBack(vehicle, std::move(vehiclePromise));
    // Then, wait until the vehicle has been granted entry.
    vehicleResult.wait();

    std::cout << "Intersection #" << _id << ": Vehicle #" << vehicle->getID() << " is granted entry." << std::endl;
}

void Intersection::vehicleHasLeft(std::shared_ptr<Vehicle> vehicle)
{
    //std::cout << "Intersection #" << _id << ": Vehicle #" << vehicle->getID() << " has left." << std::endl;

    // unblock queue processing
    this->setIsBlocked(false);
}

void Intersection::setIsBlocked(bool isBlocked)
{
    _isBlocked = isBlocked;
    //std::cout << "Intersection #" << _id << " isBlocked=" << isBlocked << std::endl;
}

// virtual function which is executed in a thread
void Intersection::simulate() // using threads + promises/futures + exceptions
{
    // launch vehicle queue processing in a thread
    threads.emplace_back(std::thread(&Intersection::processVehicleQueue, this));
}

void Intersection::processVehicleQueue()
{
    // print id of the current thread
    //std::cout << "Intersection #" << _id << "::processVehicleQueue: thread id = " << std::this_thread::get_id() << std::endl;

    // continuously process the vehicle queue
    while (true)
    {
        // sleep at every iteration to reduce CPU usage
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        // only proceed when at least one vehicle is waiting in the queue
        if (_waitingVehicles.getSize() > 0 && !_isBlocked)
        {
            // set intersection to "blocked" to prevent other vehicles from entering
            this->setIsBlocked(true);

            // permit entry to first vehicle in the queue (FIFO)
            _waitingVehicles.permitEntryToFirstInQueue();
        }
    }
}
