import processing.serial.*;

#define BUFFER_SIZE 100

Serial this_port;
int recv_buffer[BUFFER_SIZE] = {0};


void setup() {
  println(Serial.list());
  this_port = new Serial(this, Serial.list()[0], 57600);
}





