#include "main.hpp"

#include "pfmgl.h"
#include "pfm.hpp"
#include "unit-ctrl.hpp"
#include "com/unit/input-ctrl.hpp"
#include <emscripten.h>
#include <emscripten/html5.h>
#include <cstdio>
#include <cstdarg>
#include <cmath>
#include <cstring>


class emst_file_impl : public pfm_impl::pfm_file_impl
{
public:
   emst_file_impl(const std::string& ifilename, const std::string& iroot_dir) : pfm_impl::pfm_file_impl(ifilename, iroot_dir) {}
   virtual ~emst_file_impl() {}

   virtual uint64 length()
   {
      uint64 size = 0;

      if (!file)
      {
         open("rb");

         if (file)
         {
            FILE* f = (FILE*)file;

            fseek(f, 0, SEEK_END);
            size = ftell(f);
            close();
         }
      }
      else
      {
         FILE* f = (FILE*)file;
         long crt_pos = ftell(f);

         fseek(f, 0, SEEK_END);
         size = ftell(f);
         fseek(f, crt_pos, SEEK_SET);
      }

      return size;
   }

   virtual uint64 creation_time()const
   {
      return 0;
   }

   virtual uint64 last_write_time()const
   {
      return 0;
   }

   virtual void* open_impl(std::string iopen_mode)
   {
      std::string path = ppath.get_full_path();
      FILE* file = fopen(path.c_str(), iopen_mode.c_str());
      //vprint("open_impl: opening external file %s\n", path.c_str());

      return file;
   }

   virtual void close_impl()
   {
      fclose((FILE*)file);
      file = nullptr;
   }

   virtual void seek_impl(uint64 ipos, int iseek_pos)
   {
      fseek((FILE*)file, ipos, iseek_pos);
   }

   virtual uint64 tell_impl()
   {
      return ftell((FILE*)file);
   }

   virtual int read_impl(uint8* ibuffer, int isize)
   {
      return fread(ibuffer, 1, isize, (FILE*)file);
   }

   virtual int write_impl(const uint8* ibuffer, int isize)
   {
      return fwrite(ibuffer, 1, isize, (FILE*)file);
   }
};


std::shared_ptr<emst_main> emst_main::instance;

emst_main::~emst_main()
{
}

std::shared_ptr<emst_main> emst_main::get_instance()
{
   if (!instance)
   {
      instance = std::shared_ptr<emst_main>(new emst_main());
   }

   return instance;
}

std::shared_ptr<pfm_impl::pfm_file_impl> emst_main::new_pfm_file_impl(const std::string& ifilename, const std::string& iroot_dir)
{
   return std::make_shared<emst_file_impl>(ifilename, iroot_dir);
}

void emst_main::init()
{
   pfm_main::init();

   setup_callbacks();
   unit_ctrl::inst()->pre_init_app();
   unit_ctrl::inst()->set_gfx_available(true);
   unit_ctrl::inst()->init_app();

   is_started = true;
}

void emst_main::start()
{
   pfm_main::start();

   unit_ctrl::inst()->start_app();
}

void emst_main::run()
{
   unit_ctrl::inst()->update();
}

int emst_main::get_screen_dpi()const
{
   return 127;
}

void emst_main::write_text(const char* text)const
{
   printf("%s", text);
}

void emst_main::write_text_nl(const char* text)const
{
   write_text(text);
   write_text("\n");
}

void emst_main::write_text(const wchar_t* text)const
{
   printf("wstring not supported");
}

void emst_main::write_text_nl(const wchar_t* text)const
{
   write_text(text);
   write_text(L"\n");
}

void emst_main::write_text_v(const char* iformat, ...)const
{
   static char dest[1024 * 16];
   va_list arg_ptr;

   va_start(arg_ptr, iformat);
   vsnprintf(dest, 1024 * 16 - 1, iformat, arg_ptr);
   va_end(arg_ptr);
   printf("%s", dest);
}

std::string emst_main::get_writable_path()const
{
   return "";
}

umf_list emst_main::get_directory_listing(const std::string& idirectory, umf_list iplist, bool is_recursive)
{
   umf_r& list = *iplist;

   if (list.find("dummy-runtime-file") == list.end())
   {
      std::string key;
      key = "dummy-runtime-file"; list[key] = pfm_file::get_inst(std::make_shared<emst_file_impl>(key, ""));
      //key = "hello_world_file.txt"; list[key] = pfm_file::get_inst(std::make_shared<emst_file_impl>(key, ""));
      //key = "missing.png"; list[key] = pfm_file::get_inst(std::make_shared<emst_file_impl>(key, ""));
      key = "res-file"; list[key] = pfm_file::get_inst(std::make_shared<emst_file_impl>(key, ""));
      key = "vera.ttf"; list[key] = pfm_file::get_inst(std::make_shared<emst_file_impl>(key, ""));
      key = "3d-globe-outline.fsh"; list[key] = pfm_file::get_inst(std::make_shared<emst_file_impl>(key, "shaders"));
      key = "3d-globe-outline.vsh"; list[key] = pfm_file::get_inst(std::make_shared<emst_file_impl>(key, "shaders"));
      key = "basic_tex.fsh"; list[key] = pfm_file::get_inst(std::make_shared<emst_file_impl>(key, "shaders"));
      key = "basic_tex.vsh"; list[key] = pfm_file::get_inst(std::make_shared<emst_file_impl>(key, "shaders"));
      key = "c_o.fsh"; list[key] = pfm_file::get_inst(std::make_shared<emst_file_impl>(key, "shaders"));
      key = "c_o.vsh"; list[key] = pfm_file::get_inst(std::make_shared<emst_file_impl>(key, "shaders"));
      key = "fxaa.fsh"; list[key] = pfm_file::get_inst(std::make_shared<emst_file_impl>(key, "shaders"));
      key = "fxaa.vsh"; list[key] = pfm_file::get_inst(std::make_shared<emst_file_impl>(key, "shaders"));
      key = "globe-border.fsh"; list[key] = pfm_file::get_inst(std::make_shared<emst_file_impl>(key, "shaders"));
      key = "globe-border.vsh"; list[key] = pfm_file::get_inst(std::make_shared<emst_file_impl>(key, "shaders"));
      key = "globe-dot.fsh"; list[key] = pfm_file::get_inst(std::make_shared<emst_file_impl>(key, "shaders"));
      key = "globe-dot.vsh"; list[key] = pfm_file::get_inst(std::make_shared<emst_file_impl>(key, "shaders"));
      key = "globe-spike.fsh"; list[key] = pfm_file::get_inst(std::make_shared<emst_file_impl>(key, "shaders"));
      key = "globe-spike.vsh"; list[key] = pfm_file::get_inst(std::make_shared<emst_file_impl>(key, "shaders"));
      key = "globe-spike-base.fsh"; list[key] = pfm_file::get_inst(std::make_shared<emst_file_impl>(key, "shaders"));
      key = "globe-spike-base.vsh"; list[key] = pfm_file::get_inst(std::make_shared<emst_file_impl>(key, "shaders"));
      key = "hot-spot-lines.fsh"; list[key] = pfm_file::get_inst(std::make_shared<emst_file_impl>(key, "shaders"));
      key = "hot-spot-lines.vsh"; list[key] = pfm_file::get_inst(std::make_shared<emst_file_impl>(key, "shaders"));
      key = "sky-box.fsh"; list[key] = pfm_file::get_inst(std::make_shared<emst_file_impl>(key, "shaders"));
      key = "sky-box.vsh"; list[key] = pfm_file::get_inst(std::make_shared<emst_file_impl>(key, "shaders"));
      key = "line.png"; list[key] = pfm_file::get_inst(std::make_shared<emst_file_impl>(key, "tex"));
      key = "trail.png"; list[key] = pfm_file::get_inst(std::make_shared<emst_file_impl>(key, "tex"));
      key = "skybx-negx.png"; list[key] = pfm_file::get_inst(std::make_shared<emst_file_impl>(key, "tex/skybx"));
      key = "skybx-negy.png"; list[key] = pfm_file::get_inst(std::make_shared<emst_file_impl>(key, "tex/skybx"));
      key = "skybx-negz.png"; list[key] = pfm_file::get_inst(std::make_shared<emst_file_impl>(key, "tex/skybx"));
      key = "skybx-posx.png"; list[key] = pfm_file::get_inst(std::make_shared<emst_file_impl>(key, "tex/skybx"));
      key = "skybx-posy.png"; list[key] = pfm_file::get_inst(std::make_shared<emst_file_impl>(key, "tex/skybx"));
      key = "skybx-posz.png"; list[key] = pfm_file::get_inst(std::make_shared<emst_file_impl>(key, "tex/skybx"));
   }

   return iplist;
}

bool emst_main::is_full_screen_mode()
{
   return true;
}

void emst_main::set_full_screen_mode(bool ienabled)
{
}

emst_main::emst_main()
{
   is_started = false;
}

EM_BOOL emst_key_down(int event_type, const EmscriptenKeyboardEvent* e, void* user_data)
{
   return false;
}

EM_BOOL emst_key_up(int event_type, const EmscriptenKeyboardEvent* e, void* user_data)
{
   return false;
}

EM_BOOL emst_key_press(int event_type, const EmscriptenKeyboardEvent* e, void* user_data)
{
   return true;
}

EM_BOOL emst_mouse_down(int event_type, const EmscriptenMouseEvent* e, void* user_data)
{
   auto pfm_te = std::make_shared<pointer_evt>();
   pointer_evt::touch_point& te = pfm_te->points[0];

   te.identifier = 0;
   te.is_changed = false;
   te.x = (float)e->canvasX;
   te.y = (float)e->canvasY;
   pfm_te->time = pfm::time::get_time_millis();
   pfm_te->touch_count = 1;
   pfm_te->type = pointer_evt::touch_began;

   unit_ctrl::inst()->pointer_action(pfm_te);

   return true;
}

EM_BOOL emst_mouse_up(int event_type, const EmscriptenMouseEvent* e, void* user_data)
{
   auto pfm_te = std::make_shared<pointer_evt>();
   pointer_evt::touch_point& te = pfm_te->points[0];

   te.identifier = 0;
   te.is_changed = false;
   te.x = (float)e->canvasX;
   te.y = (float)e->canvasY;
   pfm_te->time = pfm::time::get_time_millis();
   pfm_te->touch_count = 1;
   pfm_te->type = pointer_evt::touch_ended;

   unit_ctrl::inst()->pointer_action(pfm_te);

   return true;
}

EM_BOOL emst_mouse_move(int event_type, const EmscriptenMouseEvent* e, void* user_data)
{
   auto pfm_te = std::make_shared<pointer_evt>();
   pointer_evt::touch_point& te = pfm_te->points[0];

   te.identifier = 0;
   te.is_changed = false;
   te.x = (float)e->canvasX;
   te.y = (float)e->canvasY;
   pfm_te->time = pfm::time::get_time_millis();
   pfm_te->touch_count = 1;
   pfm_te->type = pointer_evt::touch_moved;

   unit_ctrl::inst()->pointer_action(pfm_te);

   return true;
}

EM_BOOL emst_mouse_wheel(int event_type, const EmscriptenWheelEvent* e, void* user_data)
{
   auto pfm_te = std::make_shared<pointer_evt>();
   pointer_evt::touch_point& te = pfm_te->points[0];

   te.identifier = 0;
   te.is_changed = false;
   te.x = 0.f;
   te.y = 0.f;
   pfm_te->time = pfm::time::get_time_millis();
   pfm_te->touch_count = 1;
   pfm_te->type = pointer_evt::mouse_wheel;
   pfm_te->mouse_wheel_delta = float(e->deltaY * -0.09f);

   unit_ctrl::inst()->pointer_action(pfm_te);
   //trx("mouse wheel %1% %2% %3%") % wheel_delta % pointer_coord.x % pointer_coord.y;

   return true;
}

EM_BOOL emst_touch(int event_type, const EmscriptenTouchEvent* e, void* user_data)
{
   return true;
}

void emst_main::setup_callbacks()
{
   emscripten_set_keydown_callback(0, this, true, emst_key_down);
   emscripten_set_keyup_callback(0, this, true, emst_key_up);
   emscripten_set_keypress_callback(0, this, true, emst_key_press);
   emscripten_set_mousedown_callback("#canvas", this, true, emst_mouse_down);
   emscripten_set_mouseup_callback("#canvas", this, true, emst_mouse_up);
   emscripten_set_mousemove_callback("#canvas", this, true, emst_mouse_move);
   emscripten_set_wheel_callback("#canvas", this, false, emst_mouse_wheel);
   emscripten_set_touchstart_callback("#canvas", this, true, emst_touch);
   emscripten_set_touchend_callback("#canvas", this, true, emst_touch);
   emscripten_set_touchmove_callback("#canvas", this, true, emst_touch);
   emscripten_set_touchcancel_callback("#canvas", this, true, emst_touch);
   //emscripten_set_devicemotion_callback(this, true, emst_device_motion);
   //emscripten_set_deviceorientation_callback(this, true, emst_device_orientation);
}


void report_result(int result)
{
   if (result == 0) {
      printf("Test successful!\n");
   }
   else {
      printf("Test failed!\n");
   }
#ifdef REPORT_RESULT
   REPORT_RESULT(result);
#endif
}

static inline const char *emscripten_event_type_to_string(int event_type) {
   const char *events[] = { "(invalid)", "(none)", "keypress", "keydown", "keyup", "click", "mousedown", "mouseup", "dblclick", "mousemove", "wheel", "resize",
     "scroll", "blur", "focus", "focusin", "focusout", "deviceorientation", "devicemotion", "orientationchange", "fullscreenchange", "pointerlockchange",
     "visibilitychange", "touchstart", "touchend", "touchmove", "touchcancel", "gamepadconnected", "gamepaddisconnected", "beforeunload",
     "batterychargingchange", "batterylevelchange", "webglcontextlost", "webglcontextrestored", "(invalid)" };
   ++event_type;
   if (event_type < 0) event_type = 0;
   if (event_type >= sizeof(events) / sizeof(events[0])) event_type = sizeof(events) / sizeof(events[0]) - 1;
   return events[event_type];
}

const char *emscripten_result_to_string(EMSCRIPTEN_RESULT result) {
   if (result == EMSCRIPTEN_RESULT_SUCCESS) return "EMSCRIPTEN_RESULT_SUCCESS";
   if (result == EMSCRIPTEN_RESULT_DEFERRED) return "EMSCRIPTEN_RESULT_DEFERRED";
   if (result == EMSCRIPTEN_RESULT_NOT_SUPPORTED) return "EMSCRIPTEN_RESULT_NOT_SUPPORTED";
   if (result == EMSCRIPTEN_RESULT_FAILED_NOT_DEFERRED) return "EMSCRIPTEN_RESULT_FAILED_NOT_DEFERRED";
   if (result == EMSCRIPTEN_RESULT_INVALID_TARGET) return "EMSCRIPTEN_RESULT_INVALID_TARGET";
   if (result == EMSCRIPTEN_RESULT_UNKNOWN_TARGET) return "EMSCRIPTEN_RESULT_UNKNOWN_TARGET";
   if (result == EMSCRIPTEN_RESULT_INVALID_PARAM) return "EMSCRIPTEN_RESULT_INVALID_PARAM";
   if (result == EMSCRIPTEN_RESULT_FAILED) return "EMSCRIPTEN_RESULT_FAILED";
   if (result == EMSCRIPTEN_RESULT_NO_DATA) return "EMSCRIPTEN_RESULT_NO_DATA";
   return "Unknown EMSCRIPTEN_RESULT!";
}

#define TEST_RESULT(x) if (ret != EMSCRIPTEN_RESULT_SUCCESS) printf("%s returned %s.\n", #x, emscripten_result_to_string(ret));

// The event handler functions can return 1 to suppress the event and disable the default action. That calls event.preventDefault();
// Returning 0 signals that the event was not consumed by the code, and will allow the event to pass on and bubble up normally.
EM_BOOL key_callback(int event_type, const EmscriptenKeyboardEvent *e, void *user_data)
{
   printf("key_callback\n");
   if (event_type == EMSCRIPTEN_EVENT_KEYPRESS && (!strcmp(e->key, "f") || e->which == 102)) {
      EmscriptenFullscreenChangeEvent fsce;
      EMSCRIPTEN_RESULT ret = emscripten_get_fullscreen_status(&fsce);
      TEST_RESULT(emscripten_get_fullscreen_status);
      if (!fsce.isFullscreen) {
         printf("Requesting fullscreen..\n");
         ret = emscripten_request_fullscreen(0, 1);
         TEST_RESULT(emscripten_request_fullscreen);
      }
      else {
         printf("Exiting fullscreen..\n");
         ret = emscripten_exit_fullscreen();
         TEST_RESULT(emscripten_exit_fullscreen);
         ret = emscripten_get_fullscreen_status(&fsce);
         TEST_RESULT(emscripten_get_fullscreen_status);
         if (fsce.isFullscreen) {
            fprintf(stderr, "Fullscreen exit did not work!\n");
         }
      }
   }
   else if (event_type == EMSCRIPTEN_EVENT_KEYPRESS && (!strcmp(e->key, "Esc") || !strcmp(e->key, "Escape") || e->which == 27)) {
      emscripten_exit_soft_fullscreen();
   }
   return 0;
}

int callCount = 0;

EM_BOOL fullscreenchange_callback(int event_type, const EmscriptenFullscreenChangeEvent *e, void *user_data)
{
   printf("%s, isFullscreen: %d, fullscreenEnabled: %d, is_full_screen element nodeName: \"%s\", is_full_screen element id: \"%s\". New size: %dx%d pixels. Screen size: %dx%d pixels.\n",
      emscripten_event_type_to_string(event_type), e->isFullscreen, e->fullscreenEnabled, e->nodeName, e->id, e->elementWidth, e->elementHeight, e->screenWidth, e->screenHeight);

   ++callCount;
   if (callCount == 1) { // Transitioned to fullscreen.
      if (!e->isFullscreen) {
         report_result(1);
      }
   }
   else if (callCount == 2) { // Transitioned to windowed, we must be back to the default pixel size 300x150.
      if (e->isFullscreen || e->elementWidth != 300 || e->elementHeight != 150) {
         report_result(1);
      }
      else {
         report_result(0);
      }
   }
   return 0;
}

EM_BOOL mouse_callback(int event_type, const EmscriptenMouseEvent *e, void *user_data)
{
   //printf("mouse_callback\n");
   return 0;
}

//GLuint program;

void mws_get_canvas_size(int* width, int* height, int* is_full_screen)
{
   EMSCRIPTEN_RESULT r = emscripten_get_canvas_element_size("#canvas", width, height);
   EmscriptenFullscreenChangeEvent e;

   if (r != EMSCRIPTEN_RESULT_SUCCESS)
   {
      printf("mws_get_canvas_size: failed getting canvas size\n");
      return;
   }

   r = emscripten_get_fullscreen_status(&e);

   if (r != EMSCRIPTEN_RESULT_SUCCESS)
   {
      printf("mws_get_canvas_size: failed getting fullscreen status\n");
      return;
   }

   *is_full_screen = e.isFullscreen;
}

EM_BOOL on_canvassize_changed(int event_type, const void* reserved, void* user_data)
{
   int width, height, fs;
   mws_get_canvas_size(&width, &height, &fs);
   double css_width, css_height;
   emscripten_get_element_css_size(0, &css_width, &css_height);
   height = (height > 0) ? height : 1;
   unit_ctrl::inst()->resize_app(width, height);

   printf("Canvas resized: WebGL RTT size: %dx%d, canvas CSS size: %02gx%02g\n", width, height, css_width, css_height);

   return 0;
}

void requestFullscreen(int scaleMode, int canvasResolutionScaleMode, int filteringMode)
{
   EmscriptenFullscreenStrategy s;
   memset(&s, 0, sizeof(s));
   s.scaleMode = scaleMode;
   s.canvasResolutionScaleMode = canvasResolutionScaleMode;
   s.filteringMode = filteringMode;
   s.canvasResizedCallback = on_canvassize_changed;
   EMSCRIPTEN_RESULT ret = emscripten_request_fullscreen_strategy(0, 1, &s);
   TEST_RESULT(requestFullscreen);
}

void enterSoftFullscreen(int scaleMode, int canvasResolutionScaleMode, int filteringMode)
{
   EmscriptenFullscreenStrategy s;
   memset(&s, 0, sizeof(s));
   s.scaleMode = scaleMode;
   s.canvasResolutionScaleMode = canvasResolutionScaleMode;
   s.filteringMode = filteringMode;
   s.canvasResizedCallback = on_canvassize_changed;
   EMSCRIPTEN_RESULT ret = emscripten_enter_soft_fullscreen(0, &s);
   TEST_RESULT(enterSoftFullscreen);
}

int on_button_click(int event_type, const EmscriptenMouseEvent *mouseEvent, void *user_data)
{
   switch ((int)user_data)
   {
   case 0: requestFullscreen(EMSCRIPTEN_FULLSCREEN_SCALE_DEFAULT, EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_NONE, EMSCRIPTEN_FULLSCREEN_FILTERING_DEFAULT); break;
   case 1: requestFullscreen(EMSCRIPTEN_FULLSCREEN_SCALE_STRETCH, EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_STDDEF, EMSCRIPTEN_FULLSCREEN_FILTERING_DEFAULT); break;
   case 2: requestFullscreen(EMSCRIPTEN_FULLSCREEN_SCALE_STRETCH, EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_HIDEF, EMSCRIPTEN_FULLSCREEN_FILTERING_DEFAULT); break;
   case 3: requestFullscreen(EMSCRIPTEN_FULLSCREEN_SCALE_ASPECT, EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_STDDEF, EMSCRIPTEN_FULLSCREEN_FILTERING_DEFAULT); break;
   case 4: requestFullscreen(EMSCRIPTEN_FULLSCREEN_SCALE_ASPECT, EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_HIDEF, EMSCRIPTEN_FULLSCREEN_FILTERING_DEFAULT); break;
   case 5: requestFullscreen(EMSCRIPTEN_FULLSCREEN_SCALE_STRETCH, EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_NONE, EMSCRIPTEN_FULLSCREEN_FILTERING_DEFAULT); break;
   case 6: requestFullscreen(EMSCRIPTEN_FULLSCREEN_SCALE_ASPECT, EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_NONE, EMSCRIPTEN_FULLSCREEN_FILTERING_DEFAULT); break;
   case 7: requestFullscreen(EMSCRIPTEN_FULLSCREEN_SCALE_CENTER, EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_NONE, EMSCRIPTEN_FULLSCREEN_FILTERING_DEFAULT); break;

   case 8: enterSoftFullscreen(EMSCRIPTEN_FULLSCREEN_SCALE_STRETCH, EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_STDDEF, EMSCRIPTEN_FULLSCREEN_FILTERING_DEFAULT); break;
   case 9: enterSoftFullscreen(EMSCRIPTEN_FULLSCREEN_SCALE_STRETCH, EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_HIDEF, EMSCRIPTEN_FULLSCREEN_FILTERING_DEFAULT); break;
   case 10: enterSoftFullscreen(EMSCRIPTEN_FULLSCREEN_SCALE_ASPECT, EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_STDDEF, EMSCRIPTEN_FULLSCREEN_FILTERING_DEFAULT); break;
   case 11: enterSoftFullscreen(EMSCRIPTEN_FULLSCREEN_SCALE_ASPECT, EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_HIDEF, EMSCRIPTEN_FULLSCREEN_FILTERING_DEFAULT); break;
   case 12: enterSoftFullscreen(EMSCRIPTEN_FULLSCREEN_SCALE_STRETCH, EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_NONE, EMSCRIPTEN_FULLSCREEN_FILTERING_DEFAULT); break;
   case 13: enterSoftFullscreen(EMSCRIPTEN_FULLSCREEN_SCALE_ASPECT, EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_NONE, EMSCRIPTEN_FULLSCREEN_FILTERING_DEFAULT); break;
   case 14: enterSoftFullscreen(EMSCRIPTEN_FULLSCREEN_SCALE_CENTER, EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_NONE, EMSCRIPTEN_FULLSCREEN_FILTERING_DEFAULT); break;

   case 15: requestFullscreen(EMSCRIPTEN_FULLSCREEN_SCALE_ASPECT, EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_NONE, EMSCRIPTEN_FULLSCREEN_FILTERING_NEAREST); break;
   case 16: enterSoftFullscreen(EMSCRIPTEN_FULLSCREEN_SCALE_ASPECT, EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_NONE, EMSCRIPTEN_FULLSCREEN_FILTERING_NEAREST); break;
   default: return 0;
   }
   return 1;
}

void run_step()
{
   emst_main::get_instance()->run();
}

int main()
{
   enterSoftFullscreen(EMSCRIPTEN_FULLSCREEN_SCALE_STRETCH, EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_HIDEF, EMSCRIPTEN_FULLSCREEN_FILTERING_DEFAULT);

   EmscriptenWebGLContextAttributes attr;

   emscripten_webgl_init_context_attributes(&attr);
   attr.alpha = attr.stencil = attr.preserveDrawingBuffer = attr.preferLowPowerToHighPerformance = attr.failIfMajorPerformanceCaveat = 0;
   attr.antialias = attr.depth = 1;
   attr.enableExtensionsByDefault = 1;
   attr.premultipliedAlpha = 0;
   //attr.alpha = 1;

#if OPENGL_ES_VERSION == OPENGL_ES_2_0

   attr.majorVersion = 1;
   attr.minorVersion = 0;

#elif OPENGL_ES_VERSION == OPENGL_ES_3_0

   attr.majorVersion = 2;
   attr.minorVersion = 0;

#endif // OPENGL_ES_VERSION == OPENGL_ES_2_0

   EMSCRIPTEN_WEBGL_CONTEXT_HANDLE ctx = emscripten_webgl_create_context(0, &attr);
   emscripten_webgl_make_context_current(ctx);

   EM_BOOL anisotropy_enabled = emscripten_webgl_enable_extension(ctx, "EXT_texture_filter_anisotropic");
   printf("anisotropy enabled: %d\n", anisotropy_enabled);

   EMSCRIPTEN_RESULT ret = emscripten_set_keypress_callback(0, 0, 1, key_callback);
   TEST_RESULT(emscripten_set_keypress_callback);

   ret = emscripten_set_fullscreenchange_callback(0, 0, 1, fullscreenchange_callback);
   TEST_RESULT(emscripten_set_fullscreenchange_callback);

   // For Internet Explorer, fullscreen and pointer lock requests cannot be run
   // from inside keyboard event handlers. Therefore we must register a callback to
   // mouse events (any other than mousedown) to activate deferred fullscreen/pointerlock
   // requests to occur for IE. The callback itself can be a no-op.
   ret = emscripten_set_click_callback(0, 0, 1, mouse_callback);
   TEST_RESULT(emscripten_set_click_callback);
   ret = emscripten_set_mousedown_callback(0, 0, 1, mouse_callback);
   TEST_RESULT(emscripten_set_mousedown_callback);
   ret = emscripten_set_mouseup_callback(0, 0, 1, mouse_callback);
   TEST_RESULT(emscripten_set_mouseup_callback);
   ret = emscripten_set_dblclick_callback(0, 0, 1, mouse_callback);
   TEST_RESULT(emscripten_set_dblclick_callback);

   //emscripten_set_click_callback("b0", (void*)0, 1, on_button_click);
   //emscripten_set_click_callback("b1", (void*)1, 1, on_button_click);
   //emscripten_set_click_callback("b2", (void*)2, 1, on_button_click);
   //emscripten_set_click_callback("b3", (void*)3, 1, on_button_click);
   //emscripten_set_click_callback("b4", (void*)4, 1, on_button_click);
   //emscripten_set_click_callback("b5", (void*)5, 1, on_button_click);
   //emscripten_set_click_callback("b6", (void*)6, 1, on_button_click);
   //emscripten_set_click_callback("b7", (void*)7, 1, on_button_click);
   //emscripten_set_click_callback("b8", (void*)8, 1, on_button_click);
   //emscripten_set_click_callback("b9", (void*)9, 1, on_button_click);
   //emscripten_set_click_callback("b10", (void*)10, 1, on_button_click);
   //emscripten_set_click_callback("b11", (void*)11, 1, on_button_click);
   //emscripten_set_click_callback("b12", (void*)12, 1, on_button_click);
   //emscripten_set_click_callback("b13", (void*)13, 1, on_button_click);
   //emscripten_set_click_callback("b14", (void*)14, 1, on_button_click);
   //emscripten_set_click_callback("b15", (void*)15, 1, on_button_click);
   //emscripten_set_click_callback("b16", (void*)16, 1, on_button_click);

   printf("To finish this test, press f to enter fullscreen mode, and then exit it.\n");
   printf("On IE, press a mouse key over the canvas after pressing f to activate the fullscreen request event.\n");

   //load_file("hello_world_file.txt");

   emst_main::get_instance()->init();
   emst_main::get_instance()->start();

   emscripten_set_main_loop(run_step, 0, 0);

   return 0;
}
