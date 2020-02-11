#include <iostream>
#include <chrono>
#include <algorithm>
#include "TrafficObject.h"

// init static variable
int TrafficObject::_idCnt = 0;

// Properly instantiate the static mutex called _mtxCout
// Static variables can only be initialised outside the class, in a source file
std::mutex TrafficObject::_mtxCout;

void TrafficObject::setPosition(double x, double y)
{
    _posX = x;
    _posY = y;
}

void TrafficObject::getPosition(double &x, double &y)
{
    x = _posX;
    y = _posY;
}

TrafficObject::TrafficObject()
{
    _type = ObjectType::noObject;
    _id = _idCnt++;
}

TrafficObject::~TrafficObject()
{
    // set up thread barrier before this object is destroyed
    std::for_each(threads.begin(), threads.end(), [](std::thread &t) {
        t.join();
    });
}
