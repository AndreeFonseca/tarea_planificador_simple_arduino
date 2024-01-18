#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>
#include "planificador.c"
// Definición de pines y constantes
#define POT_PIN A0
#define SERVO_PIN 9
#define LCD_COLS 16
#define LCD_ROWS 2
#define TICK 50
#define CHARACTER 500

//Variables para el tiempo
tTime now;
tTime LastScheduleTime;

// Crear instancia de Servo y de controlador de LCD
Servo motor;
LiquidCrystal_I2C display(0x27, LCD_COLS, LCD_ROWS);

// Declaración de tareas
struct Task *blinkTask;
struct Task *potTask;
struct Task *updateLcdTask;
struct Task *servoTask;

// Declaración global para el estado del parpadeo
int blinkingState = 0;



// Función para alternar el estado del parpadeo, cada 500ms el led debe cambiar de estado
void BlinkLcd(void *args)
{
  display.setCursor(0, 1);
    if (blinkingState)
    display.write(byte(0)); // Mostrar el icono personalizado
  else
    display.write(byte(1)); // Mostrar el icono vacío
  blinkingState ^= 1;
}

// Posicion inicial del servo
int servoPosition = 0;

// Función para actualizar la posición del servo basada en el valor del potenciómetro
void UpdateServoPosition(int potenciometer)
{
  int potValue = analogRead(POT_PIN);
  //Se realiza un mapeo general
  servoPosition = map(potValue, 0, 1023, 0, 180);
  //Se pone el motor en la posicion indicada
  motor.write(servoPosition);
}

// Función para actualizar las barras en el LCD basadas en la posición del servo
void UpdateBars(void *args)
{
  int bars = map(servoPosition, 0, 180, 0, LCD_COLS);
  display.setCursor(0, 0);
  for (int i = 0; i < LCD_COLS; i++) {
    if (i < bars) {
      display.write(255); // Caracter especial de barra
    } else {
      display.write(' '); // Espacio en blanco
    }
  }
  display.setCursor(1,1);
  display.print("Servo :");
  display.setCursor(0,0);

}

// Definición matriz de bits
byte text[8] = {
B01110,
B01110,
B00100,
B11111,
B00100,
B01010,
B10001,
B00000,
};

byte empty[8] = {
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000
};
void setup()
{
  // Inicialización del LCD y del Servo
  display.init();
  display.backlight();
  display.createChar(0, text); // Cargar el icono

  motor.attach(SERVO_PIN);
  motor.write(0); // Inicializar el servo en la posición 0

  // Obtener el tiempo actual y agregar tareas al planificador
  tTime now = TimeNow() + 1;

  potTask = SchedulerAddTask(now, 0, TICK, UpdateServoPosition);
  servoTask = SchedulerAddTask(now, 0, TICK, UpdateServoPosition);
  blinkTask = SchedulerAddTask(now, 0, CHARACTER, BlinkLcd);
  //updateLcdTask = SchedulerAddTask(now, 0, TICK, UpdateLcd);

  // Agregar nuevas tareas
  struct Task *updateBarsTask = SchedulerAddTask(now, 0, 100, UpdateBars);

  // Habilitar las tareas en el planificador
  TaskEnable(potTask);
  TaskEnable(servoTask);
  TaskEnable(blinkTask);
  TaskEnable(updateBarsTask);

  //Actualizar el tiempo
  LastScheduleTime = now;
}

void loop()
{
  if (TimePassed(LastScheduleTime) > TICK) {
    //Ejecuta el planificador
    SchedulerRun();
    LastScheduleTime = TimeNow();
  }
}
