/*
g++ main.cpp -lboost_system -lboost_thread -lpthread -o main
*/

#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <iostream>
#include <unistd.h>


//pgiod message
typedef struct
{
   uint32_t cmd;
   uint32_t pin; //p1
   uint32_t value; //p2
   union
   {
      uint32_t p3;
      uint32_t ext_len;
      uint32_t res;
   };
} cmdCmd_t;


const std::string host = "192.168.1.123";
const int port = 8888;
const int steer_servo_gpio = 17;

const int pckt_size = sizeof(cmdCmd_t); //16


int main()
{
    boost::asio::io_service ios;
			
    boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::address::from_string(host), port);

    boost::asio::ip::tcp::socket socket(ios);

	socket.connect(endpoint);


    cmdCmd_t servo_message = {8, steer_servo_gpio, 550, 0};
    /*servo_message.cmd = 8; //SERVO
    servo_message.p1 = steer_servo_gpio; //GPIO
    servo_message.p2 = 550;//pulsewidth
    servo_message.p3 = 0;*/


	boost::array<char, pckt_size> buf;
    /* if you want to copy one part at a time, or change on part in the command
    std::memcpy(buf.begin(), &servo_message.cmd, 4);  //cmd
    std::memcpy(buf.begin()+4, &servo_message.p1, 4); //p1
    std::memcpy(buf.begin()+8, &servo_message.p2, 4); //p2
    std::memcpy(buf.begin()+12, &servo_message.p3, 4);//p3
    */

    std::memcpy(buf.begin(), &servo_message, pckt_size); //buffer elokeszitese

	boost::system::error_code ignore_error;

    //steering end-to-end

    int n_it = 2;

    uint32_t from = 530;
    uint32_t to = 900;

    while(n_it-- > 0){
        for(uint32_t i = from; i <= to; i+=10){
            std::cout << "send: " << i << std::endl; 
            std::memcpy(buf.begin()+8, &i, 4); //new p2
            socket.write_some(boost::asio::buffer(buf, pckt_size), ignore_error);

            usleep(5000); //0.005 sec
        }
        usleep(100000);
        for(uint32_t i = to; i >= from; i-=10){
            std::cout << "send: " << i << std::endl; 
            std::memcpy(buf.begin()+8, &i, 4); //new p2
            socket.write_some(boost::asio::buffer(buf, pckt_size), ignore_error);

            usleep(5000); //0.005 sec
        }
    }

    

    
    socket.close();

    
    return 0;
}