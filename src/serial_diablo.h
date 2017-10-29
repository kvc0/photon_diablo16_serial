#pragma once

#include <application.h>
#include <vector>

namespace diablo
{
  /*
   * An implementation of the Diablo16 serial environment graphics command set 5.2:
   * http://www.4dsystems.com.au/productpages/DIABLO16/downloads/DIABLO16_serialcmdmanual_R_2_0.pdf
   *
   * blocking arguments specify whether you want to wait for the ack inline with the call
   *   or if you want to wait for it later, before the next command is sent.
   * This library is opinionated that you *generally* want to wait later, but you may want
   *   to use synchronous messages in some cases (you want to debug ACK timing maybe)
   */
  class Diablo
  {
  public:
    Diablo(Stream& serial) :
      serial(&serial),
      pending_ack(false),
      log("app.diablo")
    {}

    /**
      * The Clear Screen command clears the screen using the current background colour. This
      * command brings some of the settings back to default; such as,
      *  Transparency turned OFF
      *  Outline colour set to BLACK
      *  Opacity set to OPAQUE
      *  Pen set to OUTLINE
      *  Line patterns set to OFF
      *  Right text margin set to full width
      *  Text magnifications set to 1
      *  All origins set to 0:0
      * The alternative to maintain settings and clear screen is to draw a filled rectangle with the
      * required background colour.
      *
      * 5.2.1
      */
    void clear(bool blocking = false)
    {
      invoke_graphics<AckOnly>("clear", LOG_LEVEL_TRACE, blocking, {0xFF82});
    }

    /*
     * x, y = center of circle
     * 5.2.3
     */
    void draw_circle(uint16_t x, uint16_t y, uint16_t radius, uint16_t color = 0xFFFF, bool blocking = false)
    {
      invoke_graphics<AckOnly>("draw_circle", LOG_LEVEL_TRACE, blocking, {0xFF78,
        x, y, radius, color
      });
    }

    /*
     * x, y = center of circle
     * 5.2.4
     */
    void draw_circle_filled(uint16_t x, uint16_t y, uint16_t radius, uint16_t color = 0xFFFF, bool blocking = false)
    {
      invoke_graphics<AckOnly>("draw_circle_filled", LOG_LEVEL_TRACE, blocking, {0xFF77,
        x, y, radius, color
      });
    }

    /*
     * The Draw Line command draws a line from x1,y1 to x2,y2 using the specified colour.
     * line is drawn using the current object colour.
     * Current origin is not altered.
     * Line may be tessellated with the “Line Pattern” command.
     *
     * x1, y1 = start coordinates
     * x2, y2 = end coordinates
     * 5.2.5
     */
    void draw_line(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color = 0xFFFF, bool blocking = false)
    {
      invoke_graphics<AckOnly>("draw_line", LOG_LEVEL_TRACE, blocking, {0xFF7D,
        x1, y1, x2, y2, color
      });
    }

    /*
     * The Draw Rectangle command draws a rectangle from x1, y1 to x2, y2 using the specified colour.
     * Line may be tessellated with the “Line Pattern” command.
     *
     * x1, y1 = start coordinates
     * x2, y2 = end coordinates
     * 5.2.6
     */
    void draw_rectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color = 0xFFFF, bool blocking = false)
    {
      invoke_graphics<AckOnly>("draw_rectangle", LOG_LEVEL_TRACE, blocking, {0xFF7A,
        x1, y1, x2, y2, color
      });
    }

    /*
     * The Draw Filled Rectangle command draws a solid rectangle from x1, y1 to x2, y2 using the specified colour.
     * Line may be tessellated with the “Line Pattern” command.
     * Outline colour can be specified with the “Outline Colour” command.
     *   If “Outline Colour” is set to 0, no outline is drawn.
     *
     * x1, y1 = start coordinates
     * x2, y2 = end coordinates
     * 5.2.7
     */
    void draw_rectangle_filled(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color = 0xFFFF, bool blocking = false)
    {
      invoke_graphics<AckOnly>("draw_rectangle_filled", LOG_LEVEL_TRACE, blocking, {0xFF79,
        x1, y1, x2, y2, color
      });
    }

    /*
     * The Draw Polyline command plots lines between points specified by a pair of arrays using the specified colour.
     * Lines may be tessellated with the “Line Pattern” command.
     * The “Draw Polyline” command can be used to create complex raster graphics by loading the arrays from serial input or from MEDIA with very little code requirement.
     *
     * ***NOTE*** the vertices argument is a little subtle.  Take a moment to ponder it.
     *
     * vertices:  x1, x2, [...], xn, y1, y2, [...], yn.
     * 5.2.8
     */
    void draw_polyline(std::vector<uint16_t> vertices, uint16_t color = 0xFFFF, bool blocking = false)
    {
      uint16_t n = vertices.size() / 2;
      invoke_graphics_compound_request<AckOnly>("draw_polyline", LOG_LEVEL_TRACE, blocking, {
        {0x0015, n}, vertices, {color}
      });
    }

    /*
     * The Draw Polygon command plots lines between points specified by a pair of arrays using the specified colour.
     * The last point is drawn back to the first point, completing the polygon.
     * The lines may be tessellated with “Line Pattern” command.
     * The Draw Polygon command can be used to create complex raster graphics by loading the arrays from serial input or from MEDIA with very little code requirement.
     *
     * ***NOTE*** the vertices argument is a little subtle.  Take a moment to ponder it.
     *
     * vertices:  x1, x2, [...], xn, y1, y2, [...], yn.
     * 5.2.9
     */
    void draw_polygon(std::vector<uint16_t> vertices, uint16_t color = 0xFFFF, bool blocking = false)
    {
      uint16_t n = vertices.size() / 2;
      invoke_graphics_compound_request<AckOnly>("draw_polygon", LOG_LEVEL_TRACE, blocking, {
        {0x0013, n}, vertices, {color}
      });
    }

    /*
     * The Draw Filled Polygon command draws a solid Polygon between specified vertices:
     *   x1, y1 x2, y2, .... , xn, yn using the specified colour.
     * The last point is drawn back to the first point, completing the polygon.
     * Vertices must be a minimum of 3 and can be specified in any fashion.
     *
     * ***NOTE*** the vertices argument is a little subtle.  Take a moment to ponder it.
     *
     * vertices:  x1, x2, [...], xn, y1, y2, [...], yn.
     * 5.2.10
     */
    void draw_polygon_filled(std::vector<uint16_t> vertices, uint16_t color = 0xFFFF, bool blocking = false)
    {
      uint16_t n = vertices.size() / 2;
      invoke_graphics_compound_request<AckOnly>("draw_polygon_filled", LOG_LEVEL_TRACE, blocking, {
        {0x0014, n}, vertices, {color}
      });
    }

    /*
     * The Draw Triangle command draws a triangle outline between vertices x1,y1 , x2,y2 and x3,y3 using the specified colour.
     * Line may be tessellated with the “Line Pattern” command.
     *
     * 5.2.11
     */
    void draw_triangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t x3, uint16_t y3, uint16_t color = 0xFFFF, bool blocking = false)
    {
      invoke_graphics<AckOnly>("draw_triangle", LOG_LEVEL_TRACE, blocking, {0xFF74,
        x1, y1, x2, y2, x3, y3, color
      });
    }

    /*
     * The Draw Filled Triangle command draws a solid triangle between vertices x1, y1, x2, y2 and x3, y3 using the specified colour.
     *
     * 5.2.12
     */
    void draw_triangle_filled(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t x3, uint16_t y3, uint16_t color = 0xFFFF, bool blocking = false)
    {
      invoke_graphics<AckOnly>("draw_triangle_filled", LOG_LEVEL_TRACE, blocking, {0xFF59,
        x1, y1, x2, y2, x3, y3, color
      });
    }

    /*
     * The Move Origin command moves the origin to a new position,
     *   suitable for specifying the location for both graphics and text.
     *
     * 5.2.16
     */
    void move_origin(uint16_t x, uint16_t y, bool blocking = false)
    {
      invoke_graphics<AckOnly>("move_origin", LOG_LEVEL_TRACE, blocking, {0xFF81,
        x, y
      });
    }

    /*
     * The Outline Colour command sets the outline colour for rectangles and circles
     *
     * 5.2.30
     * Returns previous setting.
     */
    uint16_t outline_color(uint16_t setting)
    {
      return invoke_graphics<uint16_t>("outline_color", LOG_LEVEL_INFO, true, {0xFF41,
        setting
      }, [this]()->uint16_t{return read_word();});
    }

    /*
     * 0 - 15 values
     * 5.2.31
     * Returns previous setting.
     */
    uint16_t contrast(uint16_t setting)
    {
      return invoke_graphics<uint16_t>("contrast", LOG_LEVEL_INFO, true, {0xFF40,
        setting
      }, [this]()->uint16_t{return read_word();});
    }

    /*
     * The Line Pattern command sets the line draw pattern for line drawing.
     * If set to zero, lines are solid, else each '1' bit represents a pixel that is turned off.
     *
     * 5.2.33
     * Returns previous setting.
     */
    uint16_t line_pattern(uint16_t pattern)
    {
      return invoke_graphics<uint16_t>("line_pattern", LOG_LEVEL_INFO, true, {0xFF3F,
        pattern
      }, [this]()->uint16_t{return read_word();});
    }

    /*
     * The Screen Mode command alters the graphics orientation.
     * 0 = LANDSCAPE
     * 1 = LANDSCAPE REVERSE
     * 2 = PORTRAIT
     * 3 = PORTRAIT REVERSE
     *
     * 5.2.34
     * Returns previous setting.
     */
    uint16_t screen_mode(uint16_t setting)
    {
      return invoke_graphics<uint16_t>("screen_mode", LOG_LEVEL_INFO, true, {0xFF42,
        setting
      }, [this]()->uint16_t{return read_word();});
    }

    /*
     * The Transparency command turns the transparency ON or OFF.
     * Transparency is automatically turned OFF after the next image or video command.
     *
     * 5.2.35
     * Returns previous setting.
     */
    uint16_t transparency(bool enabled)
    {
      return invoke_graphics<uint16_t>("transparency", LOG_LEVEL_INFO, true, {0xFF44,
        enabled ? 1 : 0
      }, [this]()->uint16_t{return read_word();});
    }

    /*
     * The Transparent Colour command alters the colour that needs to be made transparent.
     *
     * 5.2.36
     * Returns previous setting.
     */
    uint16_t transparent_color(uint16_t color)
    {
      return invoke_graphics<uint16_t>("transparent_color", LOG_LEVEL_INFO, true, {0xFF45,
        color
      }, [this]()->uint16_t{return read_word();});
    }

    /*
     * Returns various graphics parameters to the caller.
     *
     * Function = 18 Object Colour
     *   Sets the Object colour used in various functions such as Draw Slider and
     *   Draw Line & Move Origin
     *
     * 5.2.37
     * Returns previous setting.
     */
    uint16_t set_graphics_parameters(uint16_t function, uint16_t value)
    {
      return invoke_graphics<uint16_t>("set_graphics_parameters", LOG_LEVEL_INFO, true, {0xFF83,
        function, value
      }, [this]()->uint16_t{return read_word();});
    }

    /////////////////////////////////////    5.3 Media Commands    /////////////////////////////////////

    /*
     * The Media Init command initialises a uSD/SD/SDHC memory card for further operations.
     * The SD card is connected to the SPI (serial peripheral interface) of the Diablo16 Processor.
     *
     * 5.3.1
     * True if init successful.
     */
    bool media_init()
    {
      return invoke_graphics<bool>("media_init", LOG_LEVEL_INFO, true, {0xFF25
      }, [this]()->bool{return 1 == read_word();});
    }

    /*
     * The Set Byte Address command sets the media memory internal Address pointer for access
     *   at a non-sector aligned byte address
     *
     * 5.3.2
     */
    void media_set_byte(uint32_t address, bool blocking = false)
    {
      invoke_graphics<AckOnly>("media_set_byte", LOG_LEVEL_TRACE, blocking, {0xFF2F,
        (uint16_t)(address >> 16), (uint16_t)(address & 0xFFFF)
      });
    }

    /*
     * Displays an image from the media storage at the specified co-ordinates.
     * The image address is previously specified with the “Set Byte Address” command or “Set Sector Address” command.
     *
     * If the image is shown partially off screen, it may not be displayed correctly.
     *
     * x, y => top left corner where the image is to be drawn.
     * 5.3.11
     */
    void media_image_raw(uint16_t x, uint16_t y, bool blocking = false)
    {
      invoke_graphics<AckOnly>("media_image_raw", LOG_LEVEL_TRACE, blocking, {0xFF27,
        x, y
      });
    }

  private:
    typedef std::function<void ()> Callable;
    typedef uint8_t AckOnly;

    const Logger log;

    bool pending_ack;
    Stream *serial;

    static AckOnly no_response(){return 0;}

    // Emits a log message for how long the function took at the indicated log level.
    // Handles fetching the ack for a previous command if necessary.
    template<typename Response, typename Responder = std::function<Response ()>>
    Response invoke_graphics_compound_request(const char *name, LogLevel level, bool blocking, std::vector<std::vector<uint16_t>> request, Responder responder = no_response)
    {
      log.trace("Invoking: %s", name);
      unsigned long start = millis();
      if(pending_ack)
      {
        ack();
        pending_ack = false;
        log.trace("Previous command ack: %dms", millis() - start);
        start = millis();
      }
      log.trace("Writing request");
      for(std::vector<uint16_t> &portion : request) write_words(portion);
      if(blocking)
      {
        log.trace("Blocking for ACK");
        ack();
      } else
      {
        pending_ack = true;
      }
      log.trace("Getting response");
      Response r = responder();
      log(level, "Latency %s: %dms", name, millis() - start);
      return r;
    }

    template<typename Response, typename Responder = std::function<Response ()>>
    Response invoke_graphics(const char *name, LogLevel level, bool blocking, std::vector<uint16_t> request, Responder responder = no_response)
    {
      return invoke_graphics_compound_request<Response>(name, level, blocking, {request}, responder);
    }

    // Block for ACK byte.
    void ack()
    {
      static uint8_t timeout_length = 100;
      static uint8_t give_up_length = 4000;
      unsigned long timeout = millis() + timeout_length;
      unsigned long give_up = millis() + give_up_length;
      int response;
      do
      {
        if(millis() > timeout)
        {
          if(millis() > give_up) break;
          log.warn("Timing out waiting for ACK :-(");
          timeout = millis() + timeout_length;
        }
        response = serial->read();
      } while(response == -1);
      if(response == 0x06) log.trace("Successful ack");
      else log.error("Failed ack: %d", response);
    }

    // MostSignificantByte, LeastSignificantByte for each 2 byte word.
    void write_words(std::vector<uint16_t> &words)
    {
      for(const auto &word : words)
      {
        serial->write((uint8_t)(word >> 8));
        serial->write((uint8_t)(word & 0xFF));
      }
    }

    uint16_t read_word()
    {
      static uint8_t timeout_length = 100;
      static uint8_t give_up_length = 1000;
      unsigned long timeout = millis() + timeout_length;
      unsigned long give_up = millis() + give_up_length;
      while(serial->available() < 2)
      {
        if(millis() > timeout)
        {
          if(millis() > give_up) return 0xDEAD;
          log.warn("Timing out waiting for response :-(");
          timeout = millis() + timeout_length;
        }
      }
      return ((uint16_t)serial->read() << 8) | (uint16_t)serial->read();
    }
  };
}