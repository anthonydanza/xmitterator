import processing.serial.*;

int BUFFER_SIZE = 100;

Serial this_port;
int[] recv_buffer = new int[BUFFER_SIZE];

int x = 0;
int y = 0;
int recv_byte;

void setup() {
  size(800,300);
  background(0);
  println(Serial.list());
  //println("hello world\n");
  this_port = new Serial(this, Serial.list()[0], 57600);
}

int q = 0;

void draw() {
  if(q>300) q = 0;
  else q++;
  println(q);
  stroke(127,100,255);
  rect(100,100,100,100);
  line(x, 300, x, q);
  if(x>width) {
   x = 0;
   background(0);
  } else x++;
  
 while(this_port.available() > 0) {
  int recv_byte = this_port.readChar();
  println(recv_byte);

 } 
}










