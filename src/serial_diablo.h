#pragma once

#include <application.h>
#include <vector>

namespace diablo
{
  /*
   * An implementation of the Diablo16 serial environment command set:
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
    typedef std::function<void()> Runnable;
    Diablo(Stream &serial) :
        log("app.diablo"),
        pending_ack(false),
        outstanding_words(0),
        serial(&serial)
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
    void clear(LogLevel log_level = LOG_LEVEL_TRACE, bool blocking = false)
    {
      std::vector<uint16_t> words = {
          0xFF82
      };
      invoke_graphics<AckOnly>("clear", log_level, blocking, words);
    }

    /*
     * x, y = center of circle
     * 5.2.3
     */
    void draw_circle(uint16_t x,
                     uint16_t y,
                     uint16_t radius,
                     uint16_t color = 0xFFFF,
                     LogLevel log_level = LOG_LEVEL_TRACE,
                     bool blocking = false)
    {
      std::vector<uint16_t> words = {
          0xFF78,
          x, y, radius, color
      };
      invoke_graphics<AckOnly>("draw_circle", log_level, blocking, words);
    }

    /*
     * x, y = center of circle
     * 5.2.4
     */
    void draw_circle_filled(uint16_t x,
                            uint16_t y,
                            uint16_t radius,
                            uint16_t color = 0xFFFF,
                            LogLevel log_level = LOG_LEVEL_TRACE,
                            bool blocking = false)
    {
      std::vector<uint16_t> words = {
          0xFF77,
          x, y, radius, color
      };
      invoke_graphics<AckOnly>("draw_circle_filled", log_level, blocking, words);
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
    void draw_line(uint16_t x1,
                   uint16_t y1,
                   uint16_t x2,
                   uint16_t y2,
                   uint16_t color = 0xFFFF,
                   LogLevel log_level = LOG_LEVEL_TRACE,
                   bool blocking = false)
    {
      std::vector<uint16_t> words = {
          0xFF7D,
          x1, y1, x2, y2, color
      };
      invoke_graphics<AckOnly>("draw_line", log_level, blocking, words);
    }

    /*
     * The Draw Rectangle command draws a rectangle from x1, y1 to x2, y2 using the specified colour.
     * Line may be tessellated with the “Line Pattern” command.
     *
     * x1, y1 = start coordinates
     * x2, y2 = end coordinates
     * 5.2.6
     */
    void draw_rectangle(uint16_t x1,
                        uint16_t y1,
                        uint16_t x2,
                        uint16_t y2,
                        uint16_t color = 0xFFFF,
                        LogLevel log_level = LOG_LEVEL_TRACE,
                        bool blocking = false)
    {
      std::vector<uint16_t> words = {
          0xFF7A,
          x1, y1, x2, y2, color
      };
      invoke_graphics<AckOnly>("draw_rectangle", log_level, blocking, words);
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
    void draw_rectangle_filled(uint16_t x1,
                               uint16_t y1,
                               uint16_t x2,
                               uint16_t y2,
                               uint16_t color = 0xFFFF,
                               LogLevel log_level = LOG_LEVEL_TRACE,
                               bool blocking = false)
    {
      std::vector<uint16_t> words = {
          0xFF79,
          x1, y1, x2, y2, color
      };
      invoke_graphics<AckOnly>("draw_rectangle_filled", log_level, blocking, words);
    }

    /*
     * The Draw Polyline command plots lines between points specified by a pair of arrays using the specified colour.
     * Lines may be tessellated with the “Line Pattern” command.
     * The “Draw Polyline” command can be used to create complex raster graphics by loading the arrays from serial input or from MEDIA with very little code requirement.
     *
     * ***NOTE*** the vertices argument is a little subtle.  Take a moment to ponder it.
     * ***NOTE*** See poly_points() if you prefer an {x,y},{x,y} expression style (generally more natural).
     *
     * vertices:  x1, x2, [...], xn, y1, y2, [...], yn.
     * 5.2.8
     */
    void draw_polyline(std::vector <uint16_t> vertices,
                       uint16_t color = 0xFFFF,
                       LogLevel log_level = LOG_LEVEL_TRACE,
                       bool blocking = false)
    {
      uint16_t n = vertices.size() / 2;
      std::vector<std::vector<uint16_t>> compound_words = {
          {0x0015, n}, vertices, {color}
      };
      invoke_graphics_compound_request<AckOnly>("draw_polyline", log_level, blocking, compound_words);
    }

    /*
     * The Draw Polygon command plots lines between points specified by a pair of arrays using the specified colour.
     * The last point is drawn back to the first point, completing the polygon.
     * The lines may be tessellated with “Line Pattern” command.
     * The Draw Polygon command can be used to create complex raster graphics by loading the arrays from serial input or from MEDIA with very little code requirement.
     *
     * ***NOTE*** the vertices argument is a little subtle.  Take a moment to ponder it.
     * ***NOTE*** See poly_points() if you prefer an {x,y},{x,y} expression style (generally more natural).
     *
     * vertices:  x1, x2, [...], xn, y1, y2, [...], yn.
     * 5.2.9
     */
    void draw_polygon(std::vector <uint16_t> vertices,
                      uint16_t color = 0xFFFF,
                      LogLevel log_level = LOG_LEVEL_TRACE,
                      bool blocking = false)
    {
      uint16_t n = vertices.size() / 2;
      std::vector<std::vector<uint16_t>> compound_words = {
          {0x0013, n}, vertices, {color}
      };
      invoke_graphics_compound_request<AckOnly>("draw_polygon", log_level, blocking, compound_words);
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
    void draw_polygon_filled(std::vector <uint16_t> vertices,
                             uint16_t color = 0xFFFF,
                             LogLevel log_level = LOG_LEVEL_TRACE,
                             bool blocking = false)
    {
      uint16_t n = vertices.size() / 2;
      std::vector<std::vector<uint16_t>> compound_words = {
          {0x0014, n}, vertices, {color}
      };
      invoke_graphics_compound_request<AckOnly>("draw_polygon_filled", log_level, blocking, compound_words);
    }

    /*
     * The Draw Triangle command draws a triangle outline between vertices x1,y1 , x2,y2 and x3,y3 using the specified colour.
     * Line may be tessellated with the “Line Pattern” command.
     *
     * 5.2.11
     */
    void draw_triangle(uint16_t x1,
                       uint16_t y1,
                       uint16_t x2,
                       uint16_t y2,
                       uint16_t x3,
                       uint16_t y3,
                       uint16_t color = 0xFFFF,
                       LogLevel log_level = LOG_LEVEL_TRACE,
                       bool blocking = false)
    {
      std::vector<uint16_t> words = {
          0xFF74,
          x1, y1, x2, y2, x3, y3, color
      };
      invoke_graphics<AckOnly>("draw_triangle", log_level, blocking, words);
    }

    /*
     * The Draw Filled Triangle command draws a solid triangle between vertices x1, y1, x2, y2 and x3, y3 using the specified colour.
     *
     * 5.2.12
     */
    void draw_triangle_filled(uint16_t x1,
                              uint16_t y1,
                              uint16_t x2,
                              uint16_t y2,
                              uint16_t x3,
                              uint16_t y3,
                              uint16_t color = 0xFFFF,
                              LogLevel log_level = LOG_LEVEL_TRACE,
                              bool blocking = false)
    {
      std::vector<uint16_t> words = {
          0xFF59,
          x1, y1, x2, y2, x3, y3, color
      };
      invoke_graphics<AckOnly>("draw_triangle_filled", log_level, blocking, words);
    }

    /*
     * The Move Origin command moves the origin to a new position,
     *   suitable for specifying the location for both graphics and text.
     *
     * 5.2.16
     */
    void move_origin(uint16_t x, uint16_t y, LogLevel log_level = LOG_LEVEL_TRACE, bool blocking = false)
    {
      std::vector<uint16_t> words = {
          0xFF81,
          x, y
      };
      invoke_graphics<AckOnly>("move_origin", log_level, blocking, words);
    }

    /*
     * The Outline Colour command sets the outline colour for rectangles and circles
     *
     * 5.2.30
     * Returns previous setting.
     */
    uint16_t outline_color(uint16_t setting, LogLevel log_level = LOG_LEVEL_INFO)
    {
      std::vector<uint16_t> words = {
          0xFF41,
          setting
      };
      return invoke_graphics<uint16_t>("outline_color", log_level, true, words,
                                       [this]() -> uint16_t { return read_word(); }, 1);
    }

    /*
     * 0 - 15 values
     * 5.2.31
     * Returns previous setting.
     */
    uint16_t contrast(uint16_t setting, LogLevel log_level = LOG_LEVEL_INFO)
    {
      std::vector<uint16_t> words = {
          0xFF40,
          setting
      };
      return invoke_graphics<uint16_t>("contrast", log_level, true, words,
                                       [this]() -> uint16_t { return read_word(); }, 1);
    }

    /*
     * The Line Pattern command sets the line draw pattern for line drawing.
     * If set to zero, lines are solid, else each '1' bit represents a pixel that is turned off.
     *
     * 5.2.33
     * Returns previous setting.
     */
    uint16_t line_pattern(uint16_t pattern, LogLevel log_level = LOG_LEVEL_INFO)
    {
      std::vector<uint16_t> words = {
          0xFF3F,
          pattern
      };
      return invoke_graphics<uint16_t>("line_pattern", log_level, true, words,
                                       [this]() -> uint16_t { return read_word(); }, 1);
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
    uint16_t screen_mode(uint16_t setting, LogLevel log_level = LOG_LEVEL_INFO)
    {
      std::vector<uint16_t> words = {
          0xFF42,
          setting
      };
      return invoke_graphics<uint16_t>("screen_mode", log_level, true, words,
                                       [this]() -> uint16_t { return read_word(); }, 1);
    }

    /*
     * The Transparency command turns the transparency ON or OFF.
     * Transparency is automatically turned OFF after the next image or video command.
     *
     * 5.2.35
     * Returns previous setting.
     */
    uint16_t transparency(bool enabled, LogLevel log_level = LOG_LEVEL_INFO)
    {
      static uint8_t response_words = 1;
      std::vector<uint16_t> words = {
          0xFF44,
          enabled ? (uint16_t) 1 : (uint16_t) 0
      };
      return invoke_graphics<uint16_t>("transparency", log_level, true, words,
                                       [this]() -> uint16_t { return read_word(); },
                                       response_words);
    }

    /*
     * The Transparent Colour command alters the colour that needs to be made transparent.
     *
     * 5.2.36
     * Returns previous setting.
     */
    uint16_t transparent_color(uint16_t color, LogLevel log_level = LOG_LEVEL_INFO)
    {
      std::vector<uint16_t> words = {
          0xFF45,
          color
      };
      return invoke_graphics<uint16_t>("transparent_color", log_level, true, words,
                                       [this]() -> uint16_t { return read_word(); }, 1);
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
    uint16_t set_graphics_parameters(uint16_t function, uint16_t value, LogLevel log_level = LOG_LEVEL_INFO)
    {
      std::vector<uint16_t> words = {
          0xFF83,
          function, value
      };
      return invoke_graphics<uint16_t>("set_graphics_parameters", log_level, true, words,
                                       [this]() -> uint16_t { return read_word(); }, 1);
    }

    /////////////////////////////////////    5.3 Media Commands    /////////////////////////////////////

    /*
     * The Media Init command initialises a uSD/SD/SDHC memory card for further operations.
     * The SD card is connected to the SPI (serial peripheral interface) of the Diablo16 Processor.
     *
     * 5.3.1
     * True if init successful.
     */
    bool media_init(LogLevel log_level = LOG_LEVEL_INFO)
    {
      std::vector<uint16_t> words = {
          0xFF25
      };
      return invoke_graphics<bool>("media_init", log_level, true, words,
                                   [this]() -> bool { return 1 == read_word(); }, 1);
    }

    /*
     * The Set Byte Address command sets the media memory internal Address pointer for access
     *   at a non-sector aligned byte address
     *
     * 5.3.2
     */
    void media_set_byte(uint32_t address, LogLevel log_level = LOG_LEVEL_TRACE, bool blocking = false)
    {
      std::vector<uint16_t> words = {
          0xFF2F,
          (uint16_t)(address >> 16),
          (uint16_t)(address & 0xFFFF)
      };
      invoke_graphics<AckOnly>("media_set_byte", log_level, blocking, words);
    }

    /*
     * The Set Sector Address command sets the media memory internal Address pointer for sector access.
     *
     * 5.3.2
     */
    void media_set_sector(uint32_t address, LogLevel log_level = LOG_LEVEL_TRACE, bool blocking = false)
    {
      std::vector<uint16_t> words = {
          0xFF2E,
          (uint16_t)(address >> 16),
          (uint16_t)(address & 0xFFFF)
      };
      invoke_graphics<AckOnly>("media_set_sector", log_level, blocking, words);
    }

    /*
     * The Write Sector command writes 512 bytes (256 words) from a source memory block into the uSD card.
     * After the write the Sect pointer is automatically incremented by 1
     *
     * 5.3.5
     */
    bool media_write_sector(std::vector<uint8_t> &sector, LogLevel log_level = LOG_LEVEL_TRACE, bool blocking = false)
    {
      std::function<void ()> request = [&sector, this]()->void {
        write_word(0x0017);
        write_bytes(sector);
      };
      bool success;
      int attempt = 0;
      do
      {
        success = invoke<bool>("media_set_sector", log_level, true, request,
                               [this]() -> bool { return 1 == read_word(); }, 1);
        attempt ++;
      } while(!success && attempt < 10);
      return success;
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
    void media_image_raw(uint16_t x, uint16_t y, LogLevel log_level = LOG_LEVEL_TRACE, bool blocking = false)
    {
      std::vector<uint16_t> words = {0xFF27,
                                     x, y
      };
      invoke_graphics<AckOnly>("media_image_raw", log_level, blocking, words);
    }

    /**
      * Convenience function to wrap up setting transparency and displaying an image from a sector.
      */
    void media_image_raw(uint16_t x,
                         uint16_t y,
                         uint16_t transparency_color,
                         uint32_t sector,
                         LogLevel log_level = LOG_LEVEL_TRACE,
                         bool blocking = false)
    {
      media_set_sector(sector, log_level);
      transparency(true, log_level);
      transparent_color(transparency_color, log_level);
      media_image_raw(x, y, log_level, blocking);
    }

    /**
      * Convenience function to wrap up displaying an image from a sector.
      */
    void media_image_raw(uint16_t x,
                         uint16_t y,
                         uint32_t sector,
                         LogLevel log_level = LOG_LEVEL_TRACE,
                         bool blocking = false)
    {
      media_set_sector(sector, log_level);
      media_image_raw(x, y, log_level, blocking);
    }

    

  private:
    typedef uint8_t AckOnly;

    const Logger log;

    bool pending_ack;
    uint8_t outstanding_words;
    const char *previous_command = "";
    Stream *serial;
    std::vector<std::pair<String, Runnable>> deduping_requests;

    static AckOnly no_response()
    { return 0; }

    // Emits a log message for how long the function took at the indicated log level.
    // Handles fetching the ack for a previous command if necessary.
    template<typename Response, typename Responder = std::function < Response()>>
    Response invoke_graphics_compound_request(const char *name,
                                              LogLevel level,
                                              bool blocking,
                                              std::vector <std::vector<uint16_t>> &compound_body,
                                              Responder responder = no_response,
                                              uint8_t response_words = 0)
    {
      std::function<void ()> request = [&compound_body, this]() -> void { write_compound_words(compound_body); };
      return invoke<Response>(name, level, blocking, request, responder, response_words);
    }

    // Emits a log message for how long the function took at the indicated log level.
    // Handles fetching the ack for a previous command if necessary.
    template<typename Response, typename Responder = std::function < Response()>>
    Response invoke(const char *name,
                    LogLevel level,
                    bool blocking,
                    std::function<void ()> &request,
                    Responder responder = no_response,
                    uint8_t response_words = 0)
    {
      log.trace("Invoking: %s", name);
      unsigned long start = millis();

      // Handle leftover state
      if (pending_ack)
      {
        if (!ack())
        { return Response(); }
        pending_ack = false;
        log.trace("Previous command ack. Command: %s, %dms", previous_command, (int) (millis() - start));
        start = millis();
      }
      while (outstanding_words > 0)
      {
        uint16_t garbage = read_word();
        if (garbage == 0xDEAD)
        {
          log.error("Error waiting for response from: %s", previous_command);
          return Response();
        }
        outstanding_words--;
      }


      log.trace("Writing request");
      request();

      if (blocking)
      {
        log.trace("Blocking for ACK");
        if (!ack())
        {
          pending_ack = true;
        }
      } else
      {
        pending_ack = true;
        previous_command = name;
      }

      // Get the response.
      // If we need to ack first, we can't get the response & it'll be ignored.
      Response r;
      if (pending_ack)
      {
        outstanding_words += response_words;
        r = Response();
      } else
      {
        log.trace("Getting response");
        r = responder();
      }
      log(level, "Latency %s: %dms", name, (int) (millis() - start));
      return r;
    }

    template<typename Response, typename Responder = std::function < Response()>>
    Response invoke_graphics(const char *name,
                             LogLevel level,
                             bool blocking,
                             std::vector <uint16_t> &request,
                             Responder responder = no_response,
                             uint8_t response_words = 0)
    {
      std::vector<std::vector<uint16_t>> compound_request = {request};
      return invoke_graphics_compound_request < Response >
             (name, level, blocking, compound_request, responder, response_words);
    }

    // Block for ACK byte.
    bool ack()
    {
      static uint8_t timeout_length = 100;
      static uint16_t give_up_length = 1000;
      unsigned long timeout = millis() + timeout_length;
      unsigned long give_up = millis() + give_up_length;
      int response = -1;
      do
      {
        if (millis() > timeout)
        {
          if (millis() > give_up)
          { break; }
          log.warn("Timing out waiting for ACK :-(");
          timeout = millis() + timeout_length;
        }
        if (serial->available() > 0)
        { response = serial->read(); }
      } while (response == -1);
      if (response == 0x06)
      {
        log.trace("Successful ack");
        return true;
      } else
      {
        log.error("Failed ack: %d", response);
        return false;
      }
    }

    void write_bytes(std::vector<uint8_t> &raw_request)
    {
      for(uint8_t b : raw_request) serial->write(b);
    }

    void write_compound_words(std::vector<std::vector <uint16_t>> &compound_request)
    {
      for (std::vector <uint16_t> &portion : compound_request)
      { write_words(portion); }
    }

    // MostSignificantByte, LeastSignificantByte for each 2 byte word.
    void write_words(std::vector <uint16_t> &words)
    {
      for (const auto &word : words)
      {
        write_word(word);
      }
    }

    inline void write_word(uint16_t word)
    {
      serial->write((uint8_t)(word >> 8));
      serial->write((uint8_t)(word & 0xFF));
    }

    uint16_t read_word()
    {
      static uint8_t timeout_length = 100;
      static uint16_t give_up_length = 1000;
      unsigned long timeout = millis() + timeout_length;
      unsigned long give_up = millis() + give_up_length;
      while (serial->available() < 2)
      {
        if (millis() > timeout)
        {
          if (millis() > give_up)
          {
            return 0xDEAD;
          }
          log.warn("Timing out waiting for response :-(");
          timeout = millis() + timeout_length;
        }
      }
      return ((uint16_t) serial->read() << 8) | (uint16_t) serial->read();
    }
  };
}
