
// 
class PID_Controller {
  public:
    
    PID_Controller(int kp, int ki, int kd, int sp){
      // Coeficientes PID
      this->kp = kp;
      this->ki = ki;
      this->kd = kd;
      this->setpoint = sp;

      // Variables de control
      this->last_error = 0;
      this->integral = 0;
      this->last_time = millis();
    }

    int update(int feedback){
      int error, dt, output;
      int proportional, derivative;
      long current_time;

      // Calcular el error actual
      error = this->setpoint - feedback;

      // Obtener el tiempo actual y dt
      current_time = millis;
      dt = current_time - this->last_time;

      // Calcular las tres partes del PID: P, I, y D
      proportional = this->kp * error;

      this->integral += error * dt;  // Acumular el error para la parte integral
      this->integral = this->ki * this->integral;

      derivative = this->kd * (error - this->last_error) / dt;

      // Salida del PID
      output = proportional + this->integral + derivative;

      // Guardar los valores del ciclo anterior
      this->last_error = error;
      this->last_time = current_time;

      return output;
    }

  private:
    int kp;
    int ki;
    int kd;
    int setpoint;
    int last_error;
    int integral;
    float last_time;
};