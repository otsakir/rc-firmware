#define FRONTBACK_PIN A0
#define LEFTRIGHT_PIN A1

struct PeriodicTask {

  /*
   * A periodic task executor.
   * 
   * You keep calling its check() method and if enough time has passed it will invokes the 
   * task handler.
   * 
   */
   
  // callback/handlers should follow this interface
  // dtMillis: how much time passed since last execution
  typedef void (*TaskHandler)(int dtMillis);

  unsigned int period; // in millis
  unsigned long last = 0; // the value returned from millis() last time this task was executed
  TaskHandler handler;

  // Create a task that will call 'phandler' every 'periodMillis'
  PeriodicTask(int periodMillis, TaskHandler phandler) {
    period = periodMillis;
    handler = phandler;
  }
  
  bool check() {
    unsigned long currentMillis = millis();
    unsigned long dt;
    if (last != 0) {
      if (currentMillis < last) {
        // looks like it wrapped around. Maybe we're getting a little forward but we'll trigger that task
        dt = period;
      } else {
        dt = currentMillis - last;
      }      
    }
    if (dt >= period || last == 0) {
      // looks like that we should trigger the handler. Enough time passed.
      last = currentMillis;
      handler(dt);
      return true;
    }
    return false;
  }
};


void setup() {
  Serial.begin(9600);
}

void taskHandler(int dt) {
  Serial.println("task executed!");
}
PeriodicTask simpleTask(1000, taskHandler);

void loop() {
  // put your main code here, to run repeatedly:
  int frontback_value = analogRead(FRONTBACK_PIN);
  int leftright_value = analogRead(LEFTRIGHT_PIN);
  //Serial.print("frontback: "); Serial.print(frontback_value); 
  //Serial.print("leftright: "); Serial.print(leftright_value);
  //Serial.println();

  simpleTask.check();
}
