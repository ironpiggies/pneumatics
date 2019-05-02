//define pump motor things
#define PUMPDIR 12 //pump is on terminals A
#define PUMPPWM 3 //can use to moderate pressure
#define PUMPCURRENT A0 //will help to tell torque and therefore pressure output
#define PUMPBRAKE 9 //potentially useful? not sure. if the current is too high maybe.
//define solenoid motor 
#define SOLENOIDDIR 13 //solenoid is on terminals B
#define SOLENOIDPWM 11 //this will be set to 255 or 0 always, nothing in between.
#define SOLENOIDCURRENT A1 //not that useful, but we will keep it anyways
#define SOLENOIDBRAKE 8 //also not useful

//comment out which one you are using or not; i.e: if you want to test by hand comment out SERIAL_TALKING
//#define TESTING
#define SERIAL_TALKING


//these govern times for both testing and for actual implementation. this means we can test then 
//when we find something that works just uncomment/comment above and leave these values the same
int blow_time= 4000; //milliseconds 
int suck_time=7000;//milliseconds
int hold_time=2000;//milliseconds

/*
penumatic connections:

solenoid set up:        
                              |
                             solenoid 1
|------------|  suck line     |
|            |----------------|-------solenoid 2 ----|
|   motor    |                                       -------output
|            |----------------|-------solenoid 3 ----|
|------------|  blow line    solenoid 4
                              |

blow is activated (inflating baloon) should be solenoid 4 + 2 are closed, 1+3 are energized. This corresponds to B- currently 
(direction is LOW)
suck is solenoid 2+4 are activated, which is B+ (direction HIGH)

all solenoid black wires should go together on common ground (currently hooked up to the power supply)
2+4 red go to B+
1+3 red go to B-
*/
int pumping; //0 is nothing, 1 is suck, 2 is inflate, 3 is hold

//these need to be set each loop, called in drive() function. set here to be able to store values for controls purposes (i think?)
int pump_pwm;
bool solenoid_pwm;//only ever all-on or all-off. no in between.
bool solenoid_dir;

float pump_current; //used to tell how hard the pump is driving

//used to track how long pumping has been happening relative to what our goal is
int current_cmd_start_time=0;
//if millis() wraparound ends up being a problem (once it gets too high it becomes 0 again), then we can have the ros computer side of things take care of 
//timing and the arduino just recieves and executes commands. i could see this being an issue
//ok we are going with computer taking care of timing and the arduino just does what it is told!

void setup() {
  // put your setup code here, to run once:
  //Setup pinmodes for our pins
  pinMode(PUMPDIR,OUTPUT);
  pinMode(PUMPPWM,OUTPUT);
  pinMode(PUMPCURRENT,INPUT);

  pinMode(SOLENOIDDIR,OUTPUT);
  pinMode(SOLENOIDPWM,OUTPUT);
  pinMode(SOLENOIDCURRENT,INPUT);
  //

  digitalWrite(PUMPDIR,LOW);  //pump goes in the same direction reagrdless of polarity, so only PWM matters for it.

  #ifdef SERIAL_TALKING
    Serial.begin(9600);//used for talking with a computer
  #endif


  #ifdef TESTING
      //begin serial communication
      Serial.begin(9600); 
  #endif

}

void loop() {
  //Serial.println(pumping);
  #ifdef SERIAL_TALKING
  get_pumping_serial();
  #endif

  #ifdef TESTING
  get_pumping_test(); //use this to read value of pumping from serial connection. also will use to test :P
  #endif

  //somehow read either a desired pressure or PWM or just being told to inflate
  //ill assume that we are just being told to inflate, and that how much will be experimentally 
  //decided and implemented by the arduino

  //read values
  //pump_current = 1/1.65*map(analogRead(PUMPCURRENT),0,1023,0,3.3);// Vout= 1.65V/A * current, Vout is 0->1023 but actually is 0 to 3.3V (or 5V?)
  
  //Serial.println(analogRead(SOLENOIDCURRENT)/1.65);
  //somehow recieve value for pumping
  if (pumping==1){ //suck
    solenoid_dir=HIGH;
    solenoid_pwm=HIGH;
    pump_pwm=255;
    //Serial.println("suck");
  }else if (pumping ==2){ //blow
    solenoid_dir=LOW;
    solenoid_pwm=HIGH;
    pump_pwm=255;
    //Serial.println("blow");
  }else if (pumping==3){ //hold
    solenoid_dir=LOW;
    solenoid_pwm=LOW;
    pump_pwm=0;
    //Serial.println("hold");
  }else if (pumping==0){//no info
    solenoid_dir=LOW;
    solenoid_pwm=LOW;
    pump_pwm=0;
  } //if else, should do what was being done before. might have to change 0 to this, we shall see :P

  drive(); //send commands to motors based on pumping
  writeSerialData(); //send updates back to ros
}

void drive(){
  //code to apply motor outputs to the actual motors/actuators
  analogWrite(PUMPPWM,pump_pwm);
  digitalWrite(SOLENOIDPWM,solenoid_pwm);
  digitalWrite(SOLENOIDDIR,solenoid_dir);
}

void get_pumping_test(){ 
  //eventually this should read off of the serial port or ros topic or however that thing works
  //Serial.println(pump_current);
  //below is the test code :P it alternates between sucking and blowing for testing purposes
  int timed=millis();
  //pumping=1;
  //triple square wave/step wave:
  if ((timed%(blow_time+suck_time+hold_time))<blow_time){
    pumping=2; //blow
  }else if ((timed%(blow_time+suck_time+hold_time))<(hold_time+blow_time)){
    pumping=3; //hold
  }else{
    pumping=1;//suck
  }
}

void get_pumping_serial(){
    int new_pumping=readSerialData();
    if (new_pumping!=0){
      pumping=new_pumping;
    }
}

int readSerialData(){
  
  //if (Serial.available()>0){
    //Serial.println("ooh");
    String commandString=Serial.readStringUntil('\n');//read a whole line
    return commandString.toInt();
  //}else{
    //return 0;
  //}
}
