# photon_diablo16_serial
A partial implementation of the Diablo16 serial environment command set:
http://www.4dsystems.com.au/productpages/DIABLO16/downloads/DIABLO16_serialcmdmanual_R_2_0.pdf

## Notes
* As usual, kick off your Serial1 at the baud rate the screen expects (9600 by default, 200k works great with Photon).
* It logs!  Set up your Particle logging handler appropriately.
* Not all the commands are implemented.  It's trivial to add the missing ones, I just inserted the ones that were useful to me for now.

```
#include "serial_diablo.h"

SerialLogHandler logHandler(LOG_LEVEL_WARN, { // Logging level for non-application messages
    { "app",        LOG_LEVEL_INFO },
    { "app.diablo", LOG_LEVEL_TRACE }
});

// Just holds a pointer, doesn't actually eagerly do anything with Serial1
diablo::Diablo diablo16(Serial1);
// I like pin 4, but do your own thing.
#define SCREEN_RESETPIN 4

void setup()
{
  // Per the Particle docs, this is how it's done.
  // If you just unwrapped your nice shiny 4d systems display, you need to use 9600.
  //   Just go into Workshop4 and go File->Options->Serial and crank it up to 200000 for lulz.
  //   Or just use 9600 if you don't have high expectations, it's honestly quick enough usually.
  Serial1.begin(200000);
  
  pinMode(SCREEN_RESETPIN, OUTPUT);
  Log.info("Resetting screen");
  digitalWrite(SCREEN_RESETPIN, LOW);
  delay(100);
  digitalWrite(SCREEN_RESETPIN, HIGH);
  delay (3500); //let the display start up after the reset (This is important)
  // For uLCD-70D, use 0-15 for Brightness Control, where 0 = Display OFF, though to 15 = Max Brightness ON.
  diablo16.contrast(7);
}

// Just a toy for demonstration.
bool state = false;

void loop()
{
  ///////////////////////////       RED    GREEN BLUE   5 6 5 bits
  static const uint16_t red       = 0b1111100000000000;
  static const uint16_t green     = 0b0000010000000000;
  static const uint16_t blue      = 0b0000000000011111;
  // Toggle state every 750ms.
  bool curr_state = millis() / 750 % 2 == 0;
  
  if(state != curr_state)
  {
    // Only write to the screen if the state is changed.
    // Diablo16 only supports 1.22 million pixels per second.
    // On an 800x480 uLCD-70D that means if you want to paint the whole screen, you get 3.1 FPS.
    // Blue is "on" red is "off."
    diablo16.draw_circle_filled(100, 100, 100, state ? blue : red);
    state = curr_state;
  }
}
```

## Anatomy of a log
Timing can be a struggle with the 4d Systems displays.
They let you express some expensive operations really easily.  To help, I've added some trace messages to help you.

Here's one draw message from the above application with the default async strategy (final parameter in draw_circle_filled omitted, defaulted to false):
```
// Command which was invoked through Diablo.
0000036000 [app.diablo] TRACE: Invoking: draw_circle_filled
// Breadcrumb to indicate whether an ACK was successful.  Note that this is before we even made the request!
0000036000 [app.diablo] TRACE: Successful ack
// Previous command was sent & not ack'd.  It took 0ms to get its ack off the serial bus and be ready to send a new command.
0000036000 [app.diablo] TRACE: Previous command ack: 0ms
// Writing the command bits onto the serial bus for the Diablo.
0000036001 [app.diablo] TRACE: Writing request
// No-op for most commands.  If the  method has a return value, it will block for the response.
0000036001 [app.diablo] TRACE: Getting response
// Latency for this operation - This is partially a lie, as it only bills the portion of the library invocation
//   that was related to _this_ command.  The previous command had its _extra_ latency, if any, spelled out above.
0000036001 [app.diablo] TRACE: Latency draw_circle_filled: 0ms
```

Here's one with the synchronous ACK strategy (final parameter in draw_circle_filled specified true):
```
0000035250 [app.diablo] TRACE: Invoking: draw_circle_filled
0000035250 [app.diablo] TRACE: Writing request
0000035250 [app.diablo] TRACE: Blocking for ACK
0000035258 [app.diablo] TRACE: Successful ack
0000035258 [app.diablo] TRACE: Getting response
0000035259 [app.diablo] TRACE: Latency draw_circle_filled: 9ms
```
There's no previous message to ack, but we have to pay the time waiting up front for the Diablo16 to ACK.


## Examples
### Polygon
#### Hardest possible way to draw a square
```
diablo16.draw_polygon_filled({
  100,200,200,100, // X values
  100,100,200,200  // Y values
}, state ? blue : red);
```
#### Square with poly_points
Remember, this convenience costs O(n) compute _and_ memory.
Doesn't matter for the odd square but it can add up.
```
diablo->draw_polygon_filled(diablo::poly_points({
  {100,100},{200,100},
  {200,200},{100,200}
}), state ? on_color : off_color);
```