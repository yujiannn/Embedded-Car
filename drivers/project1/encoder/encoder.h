#ifndef ENCODER_H
#define ENCODER_H

void init_encoders();
int get_left_wheel_count();
int get_right_wheel_count();
float get_left_wheel_speed();
float get_right_wheel_speed();
void reset_encoders();

#endif // ENCODER_H
