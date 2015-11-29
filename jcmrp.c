/*
 * jcmrp.c
 * jcmrp - Joystick Control for stepper Motor on Raspberry Pi
 *
 * Copyright 2015 Valentin Corfu <corfuvalentin@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 *

    compile with on your PI: gcc -o jcmrp jcmrp.c -l wiringPi

 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/joystick.h>
#include <wiringPi.h>

#define SLEEP 1000

// MAP GPIO PINS
// Motor 1 (m1)
#define PIN1 17     // 1
#define PIN2 18     // 2
#define PIN3 27     // 3
#define PIN4 22     // 4

// Motor 2 (m2)
#define PIN5 9      // 1
#define PIN6 11     // 2
#define PIN7 25     // 3
#define PIN8 8      // 4

// table for forward move.
int ftable[8][4] = {
    {1, 0, 0, 1} ,
    {1, 0, 0, 0} ,
    {1, 1, 0, 0} ,
    {0, 1, 0, 0} ,
    {0, 1, 1, 0} ,
    {0, 0, 1, 0} ,
    {0, 0, 1, 1} ,
    {0, 0, 0, 1}
};

// table for backward move.
int btable[8][4] = {
    {0, 0, 0, 1} ,
    {0, 0, 1, 1} ,
    {0, 0, 1, 0} ,
    {0, 1, 1, 0} ,
    {0, 1, 0, 0} ,
    {1, 1, 0, 0} ,
    {1, 0, 0, 0} ,
    {1, 0, 0, 1} ,
};

#define JOY_DEV "/dev/input/js0"

//
//  set_pins: we can set here GPIO OUTPUT/LOW eatch pin.
//      a, b, c, d - are "how" to set (1/0 = OUTPUS/LOW)
//      p1, p2, p3, p4 - are "which" pins to set
//
void set_pins(int a, int b, int c, int d, int p1, int p2, int p3, int p4) {
    if (a)
        digitalWrite(p1, OUTPUT);
    else
        digitalWrite(p1, LOW);

    if (b)
        digitalWrite(p2, OUTPUT);
    else
        digitalWrite(p2, LOW);

    if (c)
        digitalWrite(p3, OUTPUT);
    else
        digitalWrite(p3, LOW);

    if (d)
        digitalWrite(p4, OUTPUT);
    else
        digitalWrite(p4, LOW);
}

//
//  move function:
//      - move(0, 1) -> for LEFT
//      - move(1, 0) -> for RIGHT
//      - move(1, 1) -> for UP
//      - move(0, 0) -> for DOWN
//  m1 - motor 1
//  m2 - motor 2
//
void move (int m1, int m2) {
    int i = 0;

    if ((m1 == 1) && (m2 == 1)) {
        for (i = 0; i < 8; i++) {
            set_pins(btable[i][0], btable[i][1], btable[i][2], btable[i][3], PIN1, PIN2, PIN3, PIN4);
            set_pins(ftable[i][0], ftable[i][1], ftable[i][2], ftable[i][3], PIN5, PIN6, PIN7, PIN8);
            usleep(SLEEP);
        }
    }
    else if ((m1 == 1)) {
        for (i = 0; i < 8; i++) {
            set_pins(btable[i][0], btable[i][1], btable[i][2], btable[i][3], PIN1, PIN2, PIN3, PIN4);
            set_pins(btable[i][0], btable[i][1], btable[i][2], btable[i][3], PIN5, PIN6, PIN7, PIN8);
            usleep(SLEEP);
        }
    } else if((m2 == 1)) {
        for (i = 0; i < 8; i++) {
            set_pins(ftable[i][0], ftable[i][1], ftable[i][2], ftable[i][3], PIN1, PIN2, PIN3, PIN4);
            set_pins(ftable[i][0], ftable[i][1], ftable[i][2], ftable[i][3], PIN5, PIN6, PIN7, PIN8);
            usleep(SLEEP);
        }
    } else {
        for (i = 0; i < 8; i++) {
            set_pins(ftable[i][0], ftable[i][1], ftable[i][2], ftable[i][3], PIN1, PIN2, PIN3, PIN4);
            set_pins(btable[i][0], btable[i][1], btable[i][2], btable[i][3], PIN5, PIN6, PIN7, PIN8);
            usleep(SLEEP);
        }
    }
}

int main()
{
    int joy_fd, *axis=NULL, num_of_axis=0, num_of_buttons=0;
    char *button=NULL, name_of_joystick[80];
    struct js_event js;
    int i = 0;

    wiringPiSetupGpio();    // Initialize wiringPi -- using Broadcom pin numbers

    // Set pins as output (M1)
    pinMode(PIN1, OUTPUT);
    pinMode(PIN2, OUTPUT);
    pinMode(PIN3, OUTPUT);
    pinMode(PIN4, OUTPUT);

    // Set pins as output (M2)
    pinMode(PIN5, OUTPUT);
    pinMode(PIN6, OUTPUT);
    pinMode(PIN7, OUTPUT);
    pinMode(PIN8, OUTPUT);

    if( ( joy_fd = open( JOY_DEV , O_RDONLY)) == -1 )
    {
        printf( "Couldn't open joystick\n" );
        return -1;
    }

    ioctl( joy_fd, JSIOCGAXES, &num_of_axis );
    ioctl( joy_fd, JSIOCGBUTTONS, &num_of_buttons );
    ioctl( joy_fd, JSIOCGNAME(80), &name_of_joystick );

    axis = (int *) calloc( num_of_axis, sizeof( int ) );
    button = (char *) calloc( num_of_buttons, sizeof( char ) );

    printf("Joystick detected: %s\n\t%d axis\n\t%d buttons\n\n"
        , name_of_joystick
        , num_of_axis
        , num_of_buttons );

    fcntl( joy_fd, F_SETFL, O_NONBLOCK );   /* use non-blocking mode */

    while( 1 )  /* infinite loop */
    {
        /* read the joystick state */
        read(joy_fd, &js, sizeof(struct js_event));

            /* see what to do with the event */
        switch (js.type & ~JS_EVENT_INIT)
        {
            case JS_EVENT_AXIS:
                axis   [ js.number ] = js.value;
                break;
            case JS_EVENT_BUTTON:
                button [ js.number ] = js.value;
                break;
        }

        if (button[0] == 1) {
            move(0, 1);
            printf("LEFT!");
        }

        if (button[1] == 1) {
            move(0, 0);
            printf("DOWN!");
        }

        if (button[2] == 1) {
            move(1, 0);
            //backward(1, 1);
            printf("RIGHT!");
        }

        if (button[3] == 1) {
            move(1, 1);
            printf("UP!");
        }

        printf("      \r");
        fflush(stdout);

        // Set all pin to LOW
        digitalWrite(PIN1, LOW);
        digitalWrite(PIN2, LOW);
        digitalWrite(PIN3, LOW);
        digitalWrite(PIN4, LOW);
        digitalWrite(PIN5, LOW);
        digitalWrite(PIN6, LOW);
        digitalWrite(PIN7, LOW);
        digitalWrite(PIN8, LOW);
    }

    close( joy_fd );    /* too bad we never get here */
    return 0;
}
