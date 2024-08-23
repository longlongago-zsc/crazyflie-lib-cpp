#include "Crazyflie.h"

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <iomanip>
#include <boost/asio/ts/buffer.hpp>
#include <boost/asio/ts/internet.hpp>
#include "CrazyfileUdp.h"
using namespace bitcraze::crazyflieLinkCpp;

using boost::asio::ip::udp;

enum { max_length = 1024 };

std::string toHex(const unsigned char* data, int len)
{
    std::stringstream ss;
    ss << std::uppercase << std::hex << std::setfill('0');
    for (int i = 0; i < len; i++) {
        ss << std::setw(2) << static_cast<unsigned>(data[i]);
    }
    return ss.str();
}

int main1()
{
    while (1)
    {
        try
        {

            //boost::asio::io_context io_context;

            boost::asio::io_context io;
            CrazyfileUdp cf(io, "192.168.43.42", 2390);

            //udp::socket s(io_context, udp::endpoint(udp::v4(), 2399));

            udp::resolver resolver(io);
            udp::endpoint endpoint =
                *resolver.resolve(udp::v4(), "192.168.43.42", "2390").begin();

            std::cout << "Enter message: ";
            uint8_t request[max_length] = { 0x5c,0x03, '\0' };
            //std::cin.getline(request, max_length);
            size_t request_length = std::strlen((const char*)request);
            //s.send_to(boost::asio::buffer(request, request_length), endpoint);
            for (;;)
            {
                cf.send(request, request_length);

                uint8_t reply[max_length] = { 0 };
                //udp::endpoint sender_endpoint = udp::endpoint(boost::asio::ip::address::from_string("192.168.43.42"), 2390);
                //size_t reply_length = s.receive_from(
                //    boost::asio::buffer(reply, max_length), sender_endpoint);

                size_t reply_length = cf.recv(reply, max_length, 0);
                std::cout << "Reply is: " << toHex(reply, reply_length);
                //std::cout.write(reply, reply_length-1);
                std::cout << "\n";
            }
        }
        catch (std::exception& e)
        {
            std::cerr << "Exception: " << e.what() << "\n";
            break;
        }
    }

    return 0;
}

int main()
{
    Crazyflie cf("udp://192.168.43.42");
    cf.init();
    int res = cf.createLogBlock({
        // {".tof_mxRj", "bucket0"},
        {"stabilizer", "yaw"},
        {"stabilizer", "roll"},
        {"stabilizer","pitch"},
        },
        "test");

    if (res < 0)
        std::cout << "creation Error: " << res << std::endl;
    res = cf.startLogBlock(10, "test");
    if (res < 0)
        std::cout << "starting Error: " << res << std::endl;
   /* res = cf.createLogBlock({
                                    {"stabilizer", "yaw"},
                                    {"stabilizer","roll"},
        },
        "test2");

    if (res < 0)
        std::cout << "creation Error: " << res << std::endl;
    res = cf.startLogBlock(10, "test2");
    if (res < 0)
        std::cout << "starting Error: " << res << std::endl;*/

    std::mutex mu;
    std::unique_lock<std::mutex> lock(mu);
    std::mutex* muPtr = &mu;
    std::condition_variable waitTillFinished;
    std::condition_variable* waitTillFinishedPtr = &waitTillFinished;
    std::atomic<bool> isFinished(false);
    std::atomic<bool>* isFinishedPtr = &isFinished;
    std::atomic<bool> isCallbackFinished(false);
    std::atomic<bool>* isCallbackFinishedPtr = &isCallbackFinished;
    std::cout << "pass " << res << std::endl;
    cf.addLogCallback([isFinishedPtr, muPtr, waitTillFinishedPtr, isCallbackFinishedPtr]
    (const std::map<TocItem, boost::spirit::hold_any>& tocItemsAndValues, uint32_t period)
        {
            std::cout << "  period:  " << period << "  val=  ";
            for (auto element : tocItemsAndValues)
            {

                 /*if(to_string(element.first._type).find("uint")!=std::string::npos)
                 {
                     uint32_t res = 0;
                     std::memcpy(&res, (void *)element.second,element.first._type.size());
                     std::cout << res;
                 }
                 else if (element.first._type == "int8_t")
                 {
                     std::cout << (int)*(int8_t*)element.second;
                 }
                  else if (element.first._type == "int16_t")
                 {
                     std::cout << *(int16_t*)element.second;
                 }
                  else if(element.first._type == "int32_t")
                 {
                     std::cout << *(int32_t*)element.second;
                 }
                 else if (element.first._type == "float")
                 {
                     std::cout << *(float*)element.second;
                 }*/
                std::cout << element.second << "  ";
            }
            std::cout << std::endl;

            if ((bool)*isFinishedPtr)
            {
                *isCallbackFinishedPtr = true;
                waitTillFinishedPtr->notify_all();
                return false;
            }
            return true;
        }, "test");

    std::cout << "Press enter to stop receiving" << std::endl;
    lock.unlock();
    char ch[256] = { 0 };
    std::cin.getline(ch, 1, '\n');
    lock.lock();
    isFinished = true;
    waitTillFinished.wait(lock, [isCallbackFinishedPtr]()
        { return (bool)*isCallbackFinishedPtr; });
    return 0;
}