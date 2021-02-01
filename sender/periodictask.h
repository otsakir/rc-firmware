#ifndef _PERIODICTASK_H_
#define _PERIODICTASK_H_

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

  // to avoid callins millisnoumerous times, it is called externally and passed as a parameter
  bool check(unsigned long currentMillis) {
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


#endif
