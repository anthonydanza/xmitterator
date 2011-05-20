import processing.serial.*;

int BUFFER_SIZE = 100;

Serial this_port;
int[] recv_buffer = new int[BUFFER_SIZE];


void setup() {
  println(Serial.list());
  println("hello world\n");
  this_port = new Serial(this, Serial.list()[0], 57600);
}

void draw() {
 while(this_port.available() > 0) {
  int recv_byte = this_port.read();
  println(recv_byte);
 } 
}

int x, y;
int recv_byte;

void serialEvent() {
 if(this_port.available()>0) recv_byte = this_port.read();

 stroke(127,34,255);
 line(x, y, x, y-recv_byte);
 if(x>width) {
  x = 0;
  background(0);
 } else x++;
 
}





