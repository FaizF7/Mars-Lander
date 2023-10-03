// Mars lander simulator
// Version 1.11
// Mechanical simulation functions
// Gabor Csanyi and Andrew Gee, August 2019

// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation, to make use of it
// for non-commercial purposes, provided that (a) its original authorship
// is acknowledged and (b) no modified versions of the source code are
// published. Restriction (b) is designed to protect the integrity of the
// exercise for future generations of students. The authors would be happy
// to receive any suggested modifications by private correspondence to
// ahg@eng.cam.ac.uk and gc121@eng.cam.ac.uk.

#include "lander.h"

void autopilot (void)
  // Autopilot to adjust the engine throttle, parachute and attitude control
{
    double e, Kh, Kp, P, D, h;                                                     // R0, target_altitude;
                                                                                 vector3d F_eq;
    attitude_stabilization();

    //target_altitude = 500;

    h = position.abs() - MARS_RADIUS;
    Kh = 0.035;
    Kp = 10;
    D = 0.4;
                                                                                 //R0 = 500;
                                                                                 //F_eq = GRAVITY * MARS_MASS * position.norm() / pow(R0, 2);
    e = -(0.5 + Kh*h + velocity*position.norm());
    P = Kp * e;

    if (P <= -D) { throttle = 0; }

    else if (-D < P < 1 - D) {
        throttle = D + P;                                                           // F_eq.abs() / MAX_THRUST;
    }                                                                              //D + P; }
    else { throttle = 1; }
    
    

}

void numerical_dynamics(void)
// This is the function that performs the numerical integration to update the
// lander's pose. The time step is delta_t (global variable).
{


    // declare variables
    double m;
    static vector3d prev_position;
    vector3d new_position, Force, a, drag_l, drag_p, G;

    //cout << position, a;
    // simulation time and timestep
    
    drag_l = -atmospheric_density(position) * DRAG_COEF_LANDER * 3.14159 * velocity.norm() * velocity.abs2() / 2;
    drag_p = -atmospheric_density(position) * DRAG_COEF_CHUTE * 10 * velocity.norm() * velocity.abs2();
    G = -GRAVITY * MARS_MASS * position.norm() / position.abs2();


    if (parachute_status == DEPLOYED) {
        Force = thrust_wrt_world() + drag_l + drag_p;
    }

    else { Force = thrust_wrt_world() + drag_l; }


    m = 200; //UNLOADED_LANDER_MASS + FUEL_CAPACITY * FUEL_DENSITY * fuel;

    a = Force / m + G;


    //cout << velocity.y, ' ';// simulation_time;
      // append current state to trajectories
    if (simulation_time == 0.0)
        // calculate new position and velocity
    {

        // Euler integration

        velocity = velocity + delta_t * a;
        new_position = position + delta_t * velocity;
        //velocity = velocity + delta_t * a;
        //velocity = (new_position - position) / delta_t;

        prev_position = position;
        position = new_position;
        //cout << position, velocity, a;
    }
    else {

        new_position = position * 2 - prev_position + delta_t* delta_t * a;
        velocity = (new_position - position) / delta_t;

        prev_position = position;
        position = new_position;
        

        /*
        
       //velocity = velocity + delta_t * a;
        new_position = position + delta_t * velocity;
        velocity = velocity + delta_t * a;
        //velocity = (new_position - position) / delta_t;

        prev_position = position;
        position = new_position;

        //cout << position;*/
}





  // Here we can apply an autopilot to adjust the thrust, parachute and attitude
  if (autopilot_enabled) autopilot();

  // Here we can apply 3-axis stabilization to ensure the base is always pointing downwards
  if (stabilized_attitude) attitude_stabilization();
 }

void initialize_simulation (void)
  // Lander pose initialization - selects one of 10 possible scenarios
{
  // The parameters to set are:
  // position - in Cartesian planetary coordinate system (m)
  // velocity - in Cartesian planetary coordinate system (m/s)
  // orientation - in lander coordinate system (xyz Euler angles, degrees)
  // delta_t - the simulation time step
  // boolean state variables - parachute_status, stabilized_attitude, autopilot_enabled
  // scenario_description - a descriptive string for the help screen

  scenario_description[0] = "circular orbit";
  scenario_description[1] = "descent from 10km";
  scenario_description[2] = "elliptical orbit, thrust changes orbital plane";
  scenario_description[3] = "polar launch at escape velocity (but drag prevents escape)";
  scenario_description[4] = "elliptical orbit that clips the atmosphere and decays";
  scenario_description[5] = "descent from 200km";
  scenario_description[6] = "descent from 500m";
  scenario_description[7] = "descent from 510m";
  scenario_description[8] = "descent from 700m";
  scenario_description[9] = "";

  switch (scenario) {

  case 0:
    // a circular equatorial orbit
    position = vector3d(1.2*MARS_RADIUS, 0.0, 0.0);
    velocity = vector3d(0.0, -3247.087385863725, 0.0);
    orientation = vector3d(0.0, 90.0, 0.0);
    delta_t = 0.1;
    parachute_status = NOT_DEPLOYED;
    stabilized_attitude = false;
    autopilot_enabled = false;
    break;
    
  case 1:
    // a descent from rest at 10km altitude
    position = vector3d(0.0, -(MARS_RADIUS + 10000.0), 0.0);
    velocity = vector3d(0.0, 0.0, 0.0);
    orientation = vector3d(0.0, 0.0, 90.0);
    delta_t = 0.1;
    parachute_status = NOT_DEPLOYED;
    stabilized_attitude = true;
    autopilot_enabled = true;
    break;

  case 2:
    // an elliptical polar orbit
    position = vector3d(0.0, 0.0, 1.2*MARS_RADIUS);
    velocity = vector3d(3500.0, 0.0, 0.0);
    orientation = vector3d(0.0, 0.0, 90.0);
    delta_t = 0.1;
    parachute_status = NOT_DEPLOYED;
    stabilized_attitude = false;
    autopilot_enabled = false;
    break;

  case 3:
    // polar surface launch at escape velocity (but drag prevents escape)
    position = vector3d(0.0, 0.0, MARS_RADIUS + LANDER_SIZE/2.0);
    velocity = vector3d(0.0, 0.0, 5027.0);
    orientation = vector3d(0.0, 0.0, 0.0);
    delta_t = 0.1;
    parachute_status = NOT_DEPLOYED;
    stabilized_attitude = false;
    autopilot_enabled = false;
    break;

  case 4:
    // an elliptical orbit that clips the atmosphere each time round, losing energy
    position = vector3d(0.0, 0.0, MARS_RADIUS + 100000.0);
    velocity = vector3d(4000.0, 0.0, 0.0);
    orientation = vector3d(0.0, 90.0, 0.0);
    delta_t = 0.1;
    parachute_status = NOT_DEPLOYED;
    stabilized_attitude = false;
    autopilot_enabled = false;
    break;

  case 5:
    // a descent from rest at the edge of the exosphere
    position = vector3d(0.0, -(MARS_RADIUS + EXOSPHERE), 0.0);
    velocity = vector3d(0.0, 0.0, 0.0);
    orientation = vector3d(0.0, 0.0, 90.0);
    delta_t = 0.1;
    parachute_status = NOT_DEPLOYED;
    stabilized_attitude = true;
    autopilot_enabled = false;
    break;

  case 6:
    position = vector3d(0.0, -(MARS_RADIUS + 500.0), 0.0);
    velocity = vector3d(0.0, 0.0, 0.0);
    orientation = vector3d(0.0, 0.0, 90.0);
    delta_t = 0.01;
    parachute_status = NOT_DEPLOYED;
    stabilized_attitude = true;
    autopilot_enabled = true;
    break;

  case 7:
    position = vector3d(0.0, -(MARS_RADIUS + 510.0), 0.0);
    velocity = vector3d(0.0, 0.0, 0.0);
    orientation = vector3d(0.0, 0.0, 90.0);
    delta_t = 0.01;
    parachute_status = NOT_DEPLOYED;
    stabilized_attitude = true;
    autopilot_enabled = true;
    break;

  case 8:
    position = vector3d(0.0, -(MARS_RADIUS + 700.0), 0.0);
    velocity = vector3d(0.0, 0.0, 0.0);
    orientation = vector3d(0.0, 0.0, 90.0);
    delta_t = 0.01;
    parachute_status = NOT_DEPLOYED;
    stabilized_attitude = true;
    autopilot_enabled = true;
    break;

  case 9:
    break;

  }
}
