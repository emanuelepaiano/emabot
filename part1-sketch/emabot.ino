/* 
 * Emabot Arduino sketch
 * (C) 2015 - Emanuele Paiano
 *      nixw0rm@gmail.com
 *      
 * Comandi:  un singolo carattere per la gestione dei comandi
 * 
 *          Comandi per Pilotare Servo camera
 *          X: Muovi di una unità il servo X della camera incrementando le ascisse (Camera X+)
 *          x: Muovi di una unità il servo X della camera decrementando le ascisse (Camera X-)
 *          Y: Muovi di una unità il servo Y della camera incrementando le ordinate (Camera Y+)
 *          y: Muovi di una unità il servo X della camera decrementando le ordinate (Camera Y-)
 *          R: Sposta il servo X della camera a 90° (Camera X Reset)
 *          r: Sposta il servo Y della camera a 90° (Camera Y Reset)
 *          B: Sposta entrambi i servo della camera a 90° (Reset Both)
 *          p: Ritorna la posizione del servo X (X current value)
 *          P: Ritorna la posizione del servo Y (Y current value)
 *          
 * 
 *          Comandi per pilotare Luci
 *          0: Spegni luci
 *          1: Accendi luci
 *          2: Trigger Luci
 *          3: Modalita parlante (lampeggia velocemente)
 *          
 *          
 *          Comandi per Pilotare Modulo L298N
 *          A: Muovi in Avanti motore di marcia (Motore A)
 *          a: Muovi all'indietro motore di marcia (Motore A)
 *          s: Ferma motore di marcia (Motore A)
 *          L: Muovi in avanti motore sterzo (gira a destra con Motore B)
 *          l: Muovi all'indietro motore sterzo (gira a sinistra con Motore B)
 *          S: Ferma motore sterzo (sterzo in posizione centrale con Motore B)
 *          +: Incrementa numero di marcia corrente (aumenta velocità tramite PWM)
 *          -: Decrementa numero di marcia corrente (riduci velocità tramite PWM)
 *          z: Imposta a zero la marcia di velocità (Reset Speed)
 *          Q: Stampa lo stato dei motori
 *          q: Stampa la marcia corrente seguita dal valore massimo di marcia
 *          
 *
*/

#include <Servo.h>

/* Uscita a cui sarà collegato il motorino Servo X */
const int KPinServoX=2;

/* Uscita a cui sarà collegato il motorino Servo Y */
const int KPinServoY=4;

/* Motorino di rotazione asse X */
Servo cameraMotorX;


/* Motorino di rotazione asse Y */
Servo cameraMotorY;

/* Variabile di stato del motorino X */

int servoAngle=90;


/* Variabile di stato del motorino Y */

int tiltAngle=90;

/* valore unità di incremento Servo della camera */
const int KUnitServo=1;


/* Uscita a cui sarà collegato il polo positivo dell'impianto luci */
const int KPinLightSystem=13;


/* Stato attuale delle luci 0: spento, 1:acceso */
int lightSystemStatus=0;


/* Opzioni modulo L298N; i nomi dei pin in alcuni casi sono invertiti (es EnA è il EnB) */

/* Primo motore */
const int enA = 11;
const int in1 = 9;
const int in2 = 8;

/* Secondo motore */
const int enB = 5;
const int in3 = 7;
const int in4 = 6;


/* Variabili modulo L298N */

/* SpeedVal attuale (in gergo marcia) */
int currentSpeed=4;

/* SpeedVal Max */
const int kMaxSpeed=5;

/* Stato motore Marcia 
 *
 * 0 : Motore Fermo
 * 1 : Motore in Avanti
 * 2 : Motore all'indietro
 */
int statusMotorA=0;


/* Stato motore Sterzo
 *
 * 0 : Sterzo dritto (Motore B stop)
 * 1 : Destra (Motore B in Avanti) 
 * 2 : Sinistra (Motore B all'indietro)
 */
int statusMotorB=0;

/* Massimo valore PWM motorino di Marcia*/
const int kMaxPwm=255;


/* valore PWM Sterzo */
const int kWheelPwm=150;



/* Variabili di gestione errori e flusso istruzioni */

/* Se True, si è verificato un errore */
bool error=0;

/* Se True, è appena stato eseguito un comando valido */
bool wasCommand=0;



/* Notifiche tramite Led  */

/* Boot Arduino Completato */

const int KBootTimesBlink=5; /* Numero di volte in cui viene eseguito il trigger */

const int KBootDelayBlink=100; /* Ritardo tra accensione e spegnimento del led */



void setup() {
  /* Inizializza motorino camera X */
  cameraMotorX.attach(KPinServoX);

  /* Inizializza motorino camera Y */
  cameraMotorY.attach(KPinServoY);

  /* inizializza ingressi e uscite */

  /* Uscite Servo Camera */
  pinMode(KPinServoX, OUTPUT);
  pinMode(KPinServoY, OUTPUT);

  /* Uscite Modulo L298N */
  pinMode(enA, OUTPUT);
  pinMode(enB, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);

  
   /* Inizializzo la comunicazione seriale usb a 9600 bit/sec */
  Serial.begin(9600);

  /* Svuoto il buffer della seriale */
  Serial.flush();


  /* Notifica avvio completato (Led blink) */
  blinkLight(KBootTimesBlink, KBootDelayBlink);
  
}




void loop() {
  
  if (Serial.available()) {
    char c = Serial.read();
    switch(c)
    {
      /* Gestione comandi del servo camera X e Y */
      
      
      case 'X': 
                if (servoAngle<=(180-KUnitServo) && servoAngle>=0)
                {
                  servoAngle+=KUnitServo;
                  printServoPosXY();
                }else{
                  error=1;
                  servoAngle=constrain(servoAngle, 0, 180);
                }
                wasCommand=1;
                break;
                
      case 'x': 
                if (servoAngle<=180 && servoAngle>=KUnitServo)
                {
                  servoAngle-=KUnitServo;
                  printServoPosXY();
                }else{
                  error=1;
                  servoAngle=constrain(servoAngle, 0, 180);
                }
                wasCommand=1;
                break;
                
      case 'Y': if (tiltAngle<=(180-KUnitServo) && tiltAngle>=0)
                {
                  tiltAngle+=KUnitServo;
                  printServoPosXY();
                }else{
                  error=1;
                  tiltAngle=constrain(tiltAngle, 0, 180);
                }
                wasCommand=1;
                break;
                
      case 'y': if (tiltAngle<=180 && tiltAngle>=KUnitServo)
                {
                  tiltAngle-=KUnitServo;
                  printServoPosXY();
                }else{
                  error=1;
                  tiltAngle=constrain(tiltAngle, 0, 180);
                }
                wasCommand=1;
                break;

      case 'R': servoAngle=90;
                wasCommand=1;
                break;

      case 'r': 
                tiltAngle=90;
                printServoPosXY();
                wasCommand=1;
                break;
                
      case 'B': 
                tiltAngle=90;
                servoAngle=90;
                printServoPosXY();
                wasCommand=1;
                break;

      case 'p': 
                printServoPosXY();
                wasCommand=1;
                break;

      case '1': setLightStatus(1);
                wasCommand=1;
                break;

      case '0': setLightStatus(0);
                wasCommand=1;
                break;

      case '2': triggerLightSystem();
                wasCommand=1;
                break;


      case 'Q': 
                printMotorsStatus();
                wasCommand=1;
                break;

      case 'q': 
                printCurrentSpeed();
                wasCommand=1;
                break;
                
                
      case '+': 
                currentSpeed++;
                currentSpeed=constrain(currentSpeed, 1, kMaxSpeed);
                wasCommand=1;
                break;
                
      case '-': 
                currentSpeed--;
                currentSpeed=constrain(currentSpeed, 1, kMaxSpeed);
                wasCommand=1;
                break;

      case 'z': 
                currentSpeed=0;
                currentSpeed=constrain(currentSpeed, 1, kMaxSpeed);
                wasCommand=1;
                break;

      
      
    
    
    
    /* Gestione comandi dei motori di marcia e terzo */
    
     case 'A': 
                forward();
                wasCommand=1;
                break;
    
     case 'a': 
                reverse();
                wasCommand=1;
                break;
                
     case 's': 
                stopMotorA();
                wasCommand=1;
                break;
                
     case 'L': 
                turnRight();
                wasCommand=1;
                break;
                
     case 'l': 
                turnLeft();
                wasCommand=1;
                break;
    
     case 'S': 
                straight();
                wasCommand=1;
                break;
    
    }

    /* Usato per evitare comandi inutili o non riconosciuti */
    
    if (wasCommand)
    {
      if (!error) Serial.println(":OK");
      else Serial.println(":ERROR");
      error=0;
      wasCommand=0;
      cameraMotorX.write(servoAngle);
      cameraMotorY.write(tiltAngle);
    }
  }

  
  
  //delay(50);
  
}

void printServoPosXY()
{
    Serial.print(servoAngle);
    Serial.print(",");
    Serial.print(tiltAngle);
}


void printCurrentSpeed()
{
  Serial.print(currentSpeed);
  Serial.print(",");
  Serial.print(kMaxSpeed);
}



void printMotorsStatus()
{
 
  switch(statusMotorA)
  {
    case 0: Serial.print("STOP");
            break;
    
    case 1: Serial.print("FORWARD");
            break;
    
    case 2: Serial.print("REVERSE");
            break;        
    
  }
  
  Serial.print(",");
  
  switch(statusMotorB)
  {
    case 0: Serial.print("STRAIGHT");
            break;
    
    case 1: Serial.print("RIGHT");
            break;
    
    case 2: Serial.print("LEFT");
            break;        
  }
  
}


int forward()
{
 
 stopMotorA();

 statusMotorA=1;
 
 digitalWrite(in1, HIGH);
 
 digitalWrite(in2, LOW);

 analogWrite(enA, getSpeedVal(currentSpeed));
 
 return 1;
}


int getSpeedVal(int value)
{
  int val=0, mult=kMaxPwm/kMaxSpeed;
  
  value=constrain(value, 0, kMaxSpeed);
  
  val=value*mult;  
  
  val=constrain(val, 0, kMaxPwm);
 
  return val;
}



int reverse()
{
  
 stopMotorA();

 statusMotorA=2;

 digitalWrite(in2, HIGH);
 
 digitalWrite(in1, LOW);

 analogWrite(enA, getSpeedVal(currentSpeed));
 
 return 2;
}


int stopMotorA()
{
 statusMotorA=0;
 
 digitalWrite(in1, LOW);
 
 digitalWrite(in2, LOW);

 analogWrite(enA, 0);
 
 return 0;
}


int turnRight()
{
  
  straight();

  statusMotorB=1;

  delay(100);

  analogWrite(enB, kWheelPwm);
 
  digitalWrite(in3, HIGH);
 
  digitalWrite(in4, LOW);
  
  return 1;
}


int turnLeft()
{
  straight();

  statusMotorB=2;

  delay(100);
  
  analogWrite(enB, kWheelPwm);
 
  digitalWrite(in3, LOW);
 
  digitalWrite(in4, HIGH);
  
  return 2;
}

int straight()
{
 statusMotorB=0;
 
 digitalWrite(in3, LOW);
 
 digitalWrite(in4, LOW);

 analogWrite(enB, 0);
 
}


int setLightStatus(int value)
{
    if (value==0) {
      lightSystemStatus=0;
      digitalWrite(KPinLightSystem, LOW);
      }else{
       lightSystemStatus=1;
       digitalWrite(KPinLightSystem, HIGH);
      }
}



int triggerLightSystem()
{
    if (lightSystemStatus==1) {
        setLightStatus(0);
      }else{
        setLightStatus(1);
      }
} 


int blinkLight(int times, float delayVal)
{
  for (int i=0; i<times; i++)
  {
    triggerLightSystem();
    delay(delayVal);
    triggerLightSystem();
    delay(delayVal);
  }
}
