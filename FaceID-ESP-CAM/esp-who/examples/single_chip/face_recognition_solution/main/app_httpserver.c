// /* ESPRESSIF MIT License
//  * 
//  * Copyright (c) 2018 <ESPRESSIF SYSTEMS (SHANGHAI) PTE LTD>
//  * 
//  * Permission is hereby granted for use on all ESPRESSIF SYSTEMS products, in which case,
//  * it is free of charge, to any person obtaining a copy of this software and associated
//  * documentation files (the "Software"), to deal in the Software without restriction, including
//  * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
//  * and/or sell copies of the Software, and to permit persons to whom the Software is furnished
//  * to do so, subject to the following conditions:
//  * 
//  * The above copyright notice and this permission notice shall be included in all copies or
//  * substantial portions of the Software.
//  * 
//  * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
//  * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
//  * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
//  * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
//  * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//  */

// #include "app_httpserver.h"
// #include "esp_log.h"
// #include "image_util.h"
// #include "fb_gfx.h"
// #include "app_main.h"

// static const char *TAG = "app_httpserver";

// #define FACE_COLOR_WHITE  0x00FFFFFF
// #define FACE_COLOR_BLACK  0x00000000
// #define FACE_COLOR_RED    0x000000FF
// #define FACE_COLOR_GREEN  0x0000FF00
// #define FACE_COLOR_BLUE   0x00FF0000
// #define FACE_COLOR_YELLOW (FACE_COLOR_RED | FACE_COLOR_GREEN)
// #define FACE_COLOR_CYAN   (FACE_COLOR_BLUE | FACE_COLOR_GREEN)
// #define FACE_COLOR_PURPLE (FACE_COLOR_BLUE | FACE_COLOR_RED)

// #define ENROLL_CONFIRM_TIMES    1
// #define FACE_ID_SAVE_NUMBER     10

// #define PART_BOUNDARY "123456789000000000000987654321"
// static const char* _STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
// static const char* _STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
// static const char* _STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

// face_id_list st_face_list = {0};

// dl_matrix3du_t *aligned_face = NULL;

// //static void oneshot_timer_callback(void* arg);

// const char *number_suffix(int32_t number)
// {
//     uint8_t n = number % 10;

//     if (n == 0)
//         return "zero";
//     else if (n == 1)
//         return "st";
//     else if (n == 2)
//         return "nd";
//     else if (n == 3)
//         return "rd";
//     else
//         return "th";
// }

// static void rgb_print(dl_matrix3du_t *image_matrix, uint32_t color, const char * str){
//     fb_data_t fb;
//     fb.width = image_matrix->w;
//     fb.height = image_matrix->h;
//     fb.data = image_matrix->item;
//     fb.bytes_per_pixel = 3;
//     fb.format = FB_BGR888;
//     fb_gfx_print(&fb, (fb.width - (strlen(str) * 14)) / 2, 10, color, str);
// }

// static int rgb_printf(dl_matrix3du_t *image_matrix, uint32_t color, const char *format, ...)
// {
//     char loc_buf[64];
//     char * temp = loc_buf;
//     int len;
//     va_list arg;
//     va_list copy;
//     va_start(arg, format);
//     va_copy(copy, arg);
//     len = vsnprintf(loc_buf, sizeof(loc_buf), format, arg);
//     va_end(copy);
//     if(len >= sizeof(loc_buf)){
//         temp = (char*)malloc(len+1);
//         if(temp == NULL) {
//             return 0;
//         }
//     }
//     vsnprintf(temp, len+1, format, arg);
//     va_end(arg);
//     rgb_print(image_matrix, color, temp);
//     if(len > 64){
//         free(temp);
//     }
//     return len;
// }

// static void draw_face_boxes(dl_matrix3du_t *image_matrix, box_array_t *boxes){
//     int x, y, w, h, i;
//     uint32_t color = FACE_COLOR_YELLOW;
//     fb_data_t fb;
//     fb.width = image_matrix->w;
//     fb.height = image_matrix->h;
//     fb.data = image_matrix->item;
//     fb.bytes_per_pixel = 3;
//     fb.format = FB_BGR888;
//     for (i = 0; i < boxes->len; i++){
//         // rectangle box
//         x = (int)boxes->box[i].box_p[0];
//         y = (int)boxes->box[i].box_p[1];
//         w = (int)boxes->box[i].box_p[2] - x + 1;
//         h = (int)boxes->box[i].box_p[3] - y + 1;
//         fb_gfx_drawFastHLine(&fb, x, y, w, color);
//         fb_gfx_drawFastHLine(&fb, x, y+h-1, w, color);
//         fb_gfx_drawFastVLine(&fb, x, y, h, color);
//         fb_gfx_drawFastVLine(&fb, x+w-1, y, h, color);
// #if 0
//         // landmark
//         int x0, y0, j;
//         for (j = 0; j < 10; j+=2) {
//             x0 = (int)boxes->landmark[i].landmark_p[j];
//             y0 = (int)boxes->landmark[i].landmark_p[j+1];
//             fb_gfx_fillRect(&fb, x0, y0, 3, 3, color);
//         }
// #endif
//     }
// }

// esp_err_t facenet_stream_handler(httpd_req_t *req)
// {
//     esp_err_t res = ESP_OK;
//     if (g_state != WAIT_FOR_CONNECT)
//     {
//         res = httpd_resp_send_404(req);
//         return res;
//     }
//     g_state = START_DETECT;

//     camera_fb_t * fb = NULL;
//     size_t _jpg_buf_len = 0;
//     uint8_t * _jpg_buf = NULL;
//     char * part_buf[64];
//     dl_matrix3du_t *image_matrix = NULL;

//     int face_id = -1;

//     int64_t fr_start = 0;
//     int64_t fr_ready = 0;
//     int64_t fr_face = 0;
//     int64_t fr_recognize = 0;
//     int64_t fr_encode = 0;

//     mtmn_config_t mtmn_config = mtmn_init_config();

//     static int64_t last_frame = 0;
//     if(!last_frame) {
//         last_frame = esp_timer_get_time();
//     }

//     res = httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);
//     if(res != ESP_OK){
//         return res;
//     }

//     ESP_LOGI(TAG, "Get count %d\n", st_face_list.count);

//     while(true)
//     {
//         // update fsm state
//         if (g_is_enrolling)
//         {
//             g_state = START_ENROLL;
//         }
//         else if (g_is_deleting)
//         {
//             g_is_deleting = 0;
//             g_state = START_DELETE;
//         }
//         else if (g_state != START_ENROLL)
//         {
//             if (st_face_list.count == 0)
//                 g_state = START_DETECT;
//             else
//                 g_state = START_RECOGNITION;
//         }
        
//         ESP_LOGD(TAG, "State: %d, head:%d, tail:%d, count:%d", g_state, st_face_list.head, st_face_list.tail, st_face_list.count);
//         // exec event
//         if (g_state == START_DELETE)
//         {
//             int8_t left = delete_face_id_in_flash(&st_face_list);
//             if (left >= 0)
//                 ESP_LOGW(TAG, "%d ID Left", left);
//             g_state = START_DETECT;
//             continue;
//         }

//         // Detection Start
//         fb = esp_camera_fb_get();
//         if (!fb)
//         {
//             ESP_LOGE(TAG, "Camera capture failed");
//             res = ESP_FAIL;
//             break;
//         }

//         fr_start = esp_timer_get_time();
//         fr_ready = fr_start;
//         fr_face = fr_start;
//         fr_encode = fr_start;
//         fr_recognize = fr_start;
//         image_matrix = dl_matrix3du_alloc(1, fb->width, fb->height, 3);
//         if (!image_matrix)
//         {
//             ESP_LOGE(TAG, "dl_matrix3du_alloc failed");
//             res = ESP_FAIL;
//             break;
//         }

//         if(!fmt2rgb888(fb->buf, fb->len, fb->format, image_matrix->item))
//         {
//             ESP_LOGW(TAG, "fmt2rgb888 failed");
//             _jpg_buf = fb->buf;
//             _jpg_buf_len = fb->len;
//             //res = ESP_FAIL;
//             //dl_matrix3du_free(image_matrix);
//             //break;
//         }
//         else
//         {

//             fr_ready = esp_timer_get_time();
//             box_array_t *net_boxes = face_detect(image_matrix, &mtmn_config);
//             fr_face = esp_timer_get_time();
//             // Detection End

//             fr_recognize = fr_face;
//             if (net_boxes)
//             {
//                 if ((g_state == START_ENROLL || g_state == START_RECOGNITION)
//                         && (align_face(net_boxes, image_matrix, aligned_face) == ESP_OK))
//                 {
//                     if (g_state == START_ENROLL)
//                     {
//                         rgb_print(image_matrix, FACE_COLOR_YELLOW, "START ENROLLING");
//                         ESP_LOGD(TAG, "START ENROLLING");

//                         int left_sample_face = enroll_face_id_to_flash(&st_face_list, aligned_face);
//                         ESP_LOGD(TAG, "Face ID %d Enrollment: Taken the %d%s sample",
//                                 st_face_list.tail,
//                                 ENROLL_CONFIRM_TIMES - left_sample_face,
//                                 number_suffix(ENROLL_CONFIRM_TIMES - left_sample_face));
//                         gpio_set_level(GPIO_LED_RED, 0);
//                         rgb_printf(image_matrix, FACE_COLOR_CYAN, "\nThe %u%s sample",
//                                 ENROLL_CONFIRM_TIMES - left_sample_face,
//                                 number_suffix(ENROLL_CONFIRM_TIMES - left_sample_face));

//                         if (left_sample_face == 0)
//                         {
//                             ESP_LOGI(TAG, "Enrolled Face ID: %d", st_face_list.tail ? st_face_list.tail - 1 : FACE_ID_SAVE_NUMBER - 1);
//                             rgb_printf(image_matrix, FACE_COLOR_CYAN, "\n\nEnrolled Face ID: %d", st_face_list.tail - 1);
//                             g_is_enrolling = 0;
//                             g_state = START_RECOGNITION;
//                         }
//                     }
//                     else
//                     {
//                         face_id = recognize_face(&st_face_list, aligned_face);

//                         if (face_id >= 0)
//                         {
//                             gpio_set_level(GPIO_LED_RED, 1);
//                             rgb_printf(image_matrix, FACE_COLOR_GREEN, "Hello ID %u", face_id);
//                         }
//                         else
//                         {
//                             rgb_print(image_matrix, FACE_COLOR_RED, "\nWHO?");
//                         }
//                     }
//                 }

//                 draw_face_boxes(image_matrix, net_boxes);
//                 dl_lib_free(net_boxes->score);
//                 dl_lib_free(net_boxes->box);
//                 dl_lib_free(net_boxes->landmark);
//                 dl_lib_free(net_boxes);

//                 fr_recognize = esp_timer_get_time();
//                 if(!fmt2jpg(image_matrix->item, fb->width*fb->height*3, fb->width, fb->height, PIXFORMAT_RGB888, 90, &_jpg_buf, &_jpg_buf_len))
//                 {
//                     ESP_LOGE(TAG, "fmt2jpg failed");
//                 }
//                 esp_camera_fb_return(fb);
//                 fb = NULL;
//             }
//             else
//             {
//                 _jpg_buf = fb->buf;
//                 _jpg_buf_len = fb->len;
//             }
//         }
//         dl_matrix3du_free(image_matrix);
//         fr_encode = esp_timer_get_time();


//         if(res == ESP_OK){
//             size_t hlen = snprintf((char *)part_buf, 64, _STREAM_PART, _jpg_buf_len);
//             res = httpd_resp_send_chunk(req, (const char *)part_buf, hlen);
//         }
//         if(res == ESP_OK){
//             res = httpd_resp_send_chunk(req, (const char *)_jpg_buf, _jpg_buf_len);
//         }
//         if(res == ESP_OK){
//             res = httpd_resp_send_chunk(req, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));
//         }
//         if(fb){
//             esp_camera_fb_return(fb);
//             fb = NULL;
//             _jpg_buf = NULL;
//         } else if(_jpg_buf){
//             free(_jpg_buf);
//             _jpg_buf = NULL;
//         }
//         if(res != ESP_OK){
//             break;
//         }
//         int64_t fr_end = esp_timer_get_time();

//         int64_t ready_time = (fr_ready - fr_start)/1000;
//         int64_t face_time = (fr_face - fr_ready)/1000;
//         int64_t recognize_time = (fr_recognize - fr_face)/1000;
//         int64_t encode_time = (fr_encode - fr_recognize)/1000;
//         int64_t process_time = (fr_encode - fr_start)/1000;

//         int64_t frame_time = fr_end - last_frame;
//         last_frame = fr_end;
//         frame_time /= 1000;
//         ESP_LOGD(TAG, "MJPG: %uKB %ums (%.1ffps), %u+%u+%u+%u=%u",
//                 (uint32_t)(_jpg_buf_len/1024),
//                 (uint32_t)frame_time, 1000.0 / (uint32_t)frame_time,
//                 (uint32_t)ready_time, (uint32_t)face_time, (uint32_t)recognize_time, (uint32_t)encode_time, (uint32_t)process_time);
//     }

//     last_frame = 0;
//     g_state = WAIT_FOR_WAKEUP;
//     return ESP_OK;
// }

// httpd_uri_t _face_stream_handler = {
//     .uri       = "/face_stream",
//     .method    = HTTP_GET,
//     .handler   = facenet_stream_handler,
//     .user_ctx  = NULL
// };

// httpd_handle_t camera_httpd = NULL;

// //const esp_timer_create_args_t oneshot_timer_args = {
// //            .callback = &oneshot_timer_callback,
// //            /* argument specified here will be passed to timer callback function */
// //            .name = "one-shot"
// //};
// //esp_timer_handle_t oneshot_timer;
// //
// //static void oneshot_timer_callback(void* arg)
// //{
// //    //if(g_state != START_ENROLL)
// //    //    g_is_enrolling = 1;
// //}

// // ------------------

// // Đoạn này thêm vào sau
// esp_err_t root_get_handler(httpd_req_t *req)
// {
//     const char* resp_str = (const char*)
//         "<!DOCTYPE html><html>"
//         "<head><meta charset='utf-8'><title>ESP-EYE</title></head>"
//         "<body>"
//         "<h2>ESP-EYE Face Recognition</h2>"
//         "<p>Camera streaming: <a href=\"/stream\">Click here</a></p>"
//         "<p>Capture image: <a href=\"/capture\">Click here</a></p>"
//         "</body></html>";

//     httpd_resp_send(req, resp_str, HTTPD_RESP_USE_STRLEN);
//     return ESP_OK;
// }

// httpd_uri_t root_uri = {
//     .uri      = "/",
//     .method   = HTTP_GET,
//     .handler  = root_get_handler,
//     .user_ctx = NULL
// };

// // ------------------


// static void IRAM_ATTR gpio_isr_handler_enroll(void* arg)
// {
//     if(g_state != START_ENROLL)
//         g_is_enrolling = 1;
//     //esp_timer_start_once(oneshot_timer, 500000);
//     //if(err == ESP_ERR_INVALID_STATE)
//     //{
//     //    ESP_ERROR_CHECK(esp_timer_stop(oneshot_timer));
//     //    g_is_enrolling = 0;
//     //    g_is_deleting = 1;
//     //    gpio_set_level(GPIO_LED_WHITE, 0);
//     //}
// }

// static void IRAM_ATTR gpio_isr_handler_delete(void* arg)
// {
//     g_is_enrolling = 0;
//     g_is_deleting = 1;
    
// }

// void app_httpserver_init ()
// {
//     httpd_config_t config = HTTPD_DEFAULT_CONFIG();
//     config.stack_size = 4096 * 2;

//     //ESP_ERROR_CHECK(esp_timer_create(&oneshot_timer_args, &oneshot_timer));

//     gpio_config_t io_conf = {0};
//     io_conf.mode = GPIO_MODE_INPUT;
//     io_conf.intr_type = GPIO_PIN_INTR_POSEDGE;
//     io_conf.pin_bit_mask = 1LL << GPIO_BUTTON | 1LL << GPIO_BOOT;
//     io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
//     gpio_config(&io_conf);
//     gpio_isr_handler_add(GPIO_BUTTON, gpio_isr_handler_enroll, NULL);
//     gpio_isr_handler_add(GPIO_BOOT, gpio_isr_handler_delete, NULL);

//     face_id_init(&st_face_list, FACE_ID_SAVE_NUMBER, ENROLL_CONFIRM_TIMES);
//     aligned_face = dl_matrix3du_alloc(1, FACE_WIDTH, FACE_HEIGHT, 3);
//     read_face_id_from_flash(&st_face_list);

//     if (httpd_start(&camera_httpd, &config) == ESP_OK)
//     {
//         httpd_register_uri_handler(camera_httpd, &_face_stream_handler);

//         httpd_register_uri_handler(camera_httpd, &root_uri);   // <--- thêm dòng này
//     }

//     // ---------------------------- added
//     // handler cho /stream
//     httpd_uri_t stream_uri = {
//         .uri      = "/stream",
//         .method   = HTTP_GET,
//         .handler  = facenet_stream_handler,   // dùng lại handler facenet
//         .user_ctx = NULL
//     };
//     httpd_register_uri_handler(camera_httpd, &stream_uri);

//     // handler cho /capture
//     esp_err_t capture_handler(httpd_req_t *req)
//     {
//         camera_fb_t *fb = esp_camera_fb_get();
//         if (!fb) {
//             httpd_resp_send_500(req);
//             return ESP_FAIL;
//         }
//         httpd_resp_set_type(req, "image/jpeg");
//         httpd_resp_send(req, (const char *)fb->buf, fb->len);
//         esp_camera_fb_return(fb);
//         return ESP_OK;
//     }

//     httpd_uri_t capture_uri = {
//         .uri      = "/capture",
//         .method   = HTTP_GET,
//         .handler  = capture_handler,
//         .user_ctx = NULL
//     };
//     httpd_register_uri_handler(camera_httpd, &capture_uri);
//     // ----------------------------
// }

// #include "app_httpserver.h"
// #include "esp_log.h"
// #include "esp_camera.h"
// #include "esp_http_server.h"

// static const char *TAG = "app_httpserver";

// httpd_handle_t camera_httpd = NULL;

// // ---------------- Root page ----------------
// esp_err_t root_get_handler(httpd_req_t *req)
// {
//     const char* resp_str = (const char*)
//         "<!DOCTYPE html><html>"
//         "<head><meta charset='utf-8'><title>ESP32-CAM</title></head>"
//         "<body>"
//         "<h2>ESP32-CAM Face Recognition Demo</h2>"
//         "<p>Camera streaming: <a href=\"/stream\">Click here</a></p>"
//         "<p>Capture image: <a href=\"/capture\">Click here</a></p>"
//         "</body></html>";

//     httpd_resp_send(req, resp_str, HTTPD_RESP_USE_STRLEN);
//     return ESP_OK;
// }

// httpd_uri_t root_uri = {
//     .uri      = "/",
//     .method   = HTTP_GET,
//     .handler  = root_get_handler,
//     .user_ctx = NULL
// };

// // ---------------- Capture ----------------
// esp_err_t capture_handler(httpd_req_t *req)
// {
//     camera_fb_t * fb = esp_camera_fb_get();
//     if (!fb) {
//         ESP_LOGE(TAG, "Camera capture failed");
//         httpd_resp_send_500(req);
//         return ESP_FAIL;
//     }

//     httpd_resp_set_type(req, "image/jpeg");
//     httpd_resp_send(req, (const char *)fb->buf, fb->len);
//     esp_camera_fb_return(fb);
//     return ESP_OK;
// }

// httpd_uri_t capture_uri = {
//     .uri      = "/capture",
//     .method   = HTTP_GET,
//     .handler  = capture_handler,
//     .user_ctx = NULL
// };

// // ---------------- Stream ----------------
// #define PART_BOUNDARY "123456789000000000000987654321"
// static const char* _STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
// static const char* _STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
// static const char* _STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

// esp_err_t stream_handler(httpd_req_t *req)
// {
//     esp_err_t res = httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);
//     if(res != ESP_OK) return res;

//     char part_buf[64];
//     while(true) {
//         camera_fb_t * fb = esp_camera_fb_get();
//         if (!fb) {
//             ESP_LOGE(TAG, "Camera capture failed");
//             return ESP_FAIL;
//         }
//         size_t hlen = snprintf(part_buf, 64, _STREAM_PART, fb->len);
//         res = httpd_resp_send_chunk(req, part_buf, hlen);
//         if(res == ESP_OK) res = httpd_resp_send_chunk(req, (const char *)fb->buf, fb->len);
//         if(res == ESP_OK) res = httpd_resp_send_chunk(req, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));
//         esp_camera_fb_return(fb);
//         if(res != ESP_OK) break;
//     }
//     return res;
// }

// httpd_uri_t stream_uri = {
//     .uri      = "/stream",
//     .method   = HTTP_GET,
//     .handler  = stream_handler,
//     .user_ctx = NULL
// };

// // ---------------- Init ----------------
// void app_httpserver_init()
// {
//     httpd_config_t config = HTTPD_DEFAULT_CONFIG();
//     config.stack_size = 4096 * 2;

//     if (httpd_start(&camera_httpd, &config) == ESP_OK)
//     {
//         httpd_register_uri_handler(camera_httpd, &root_uri);
//         httpd_register_uri_handler(camera_httpd, &capture_uri);
//         httpd_register_uri_handler(camera_httpd, &stream_uri);
//     }
// }

// --------------------------------------------------------------------


/* ESPRESSIF MIT License
 *
 * This file merges: root page + capture + stream (with face detect/recognize)
 * and adds /enroll and /delete endpoints for ESP-WHO face_id workflow.
 */
#include <string.h>
#include "esp_log.h"
#include "esp_http_server.h"
#include "esp_camera.h"

#include "image_util.h"
#include "fb_gfx.h"
#include "app_main.h"        // chứa g_state, g_is_enrolling, g_is_deleting, GPIO_* ...
                             // và các API face_id_* / align_face / face_detect

static const char *TAG = "app_httpserver";

/* ====================== Configs & Globals ====================== */
#define FACE_COLOR_WHITE  0x00FFFFFF
#define FACE_COLOR_BLACK  0x00000000
#define FACE_COLOR_RED    0x000000FF
#define FACE_COLOR_GREEN  0x0000FF00
#define FACE_COLOR_BLUE   0x00FF0000
#define FACE_COLOR_YELLOW (FACE_COLOR_RED | FACE_COLOR_GREEN)
#define FACE_COLOR_CYAN   (FACE_COLOR_BLUE | FACE_COLOR_GREEN)
#define FACE_COLOR_PURPLE (FACE_COLOR_BLUE | FACE_COLOR_RED)

#define ENROLL_CONFIRM_TIMES    1
#define FACE_ID_SAVE_NUMBER     10

#define PART_BOUNDARY "123456789000000000000987654321"
static const char* _STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char* _STREAM_BOUNDARY     = "\r\n--" PART_BOUNDARY "\r\n";
static const char* _STREAM_PART         = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

httpd_handle_t camera_httpd = NULL;

face_id_list st_face_list = {0};
dl_matrix3du_t *aligned_face = NULL;

/* ====================== Small helpers ====================== */
static const char *number_suffix(int32_t number)
{
    uint8_t n = number % 10;
    if (n == 1) return "st";
    if (n == 2) return "nd";
    if (n == 3) return "rd";
    return "th";
}

static void rgb_print(dl_matrix3du_t *im, uint32_t color, const char *str)
{
    fb_data_t fb = {
        .width = im->w, .height = im->h, .data = im->item,
        .bytes_per_pixel = 3, .format = FB_BGR888
    };
    fb_gfx_print(&fb, (fb.width - (strlen(str) * 14)) / 2, 10, color, str);
}

static void draw_face_boxes(dl_matrix3du_t *im, box_array_t *boxes)
{
    fb_data_t fb = {
        .width = im->w, .height = im->h, .data = im->item,
        .bytes_per_pixel = 3, .format = FB_BGR888
    };
    for (int i = 0; i < boxes->len; i++){
        int x = (int)boxes->box[i].box_p[0];
        int y = (int)boxes->box[i].box_p[1];
        int w = (int)boxes->box[i].box_p[2] - x + 1;
        int h = (int)boxes->box[i].box_p[3] - y + 1;
        fb_gfx_drawFastHLine(&fb, x,       y,       w, FACE_COLOR_YELLOW);
        fb_gfx_drawFastHLine(&fb, x,       y+h-1,   w, FACE_COLOR_YELLOW);
        fb_gfx_drawFastVLine(&fb, x,       y,       h, FACE_COLOR_YELLOW);
        fb_gfx_drawFastVLine(&fb, x+w-1,   y,       h, FACE_COLOR_YELLOW);
    }
}

/* ====================== Stream (face detect/recognize) ====================== */
static esp_err_t facenet_stream_handler(httpd_req_t *req)
{
    if (g_state != WAIT_FOR_CONNECT) {
        return httpd_resp_send_404(req);
    }
    g_state = START_DETECT;

    esp_err_t res = httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);
    if (res != ESP_OK) return res;

    ESP_LOGI(TAG, "Face DB count: %d", st_face_list.count);

    mtmn_config_t mtmn_config = mtmn_init_config();
    static int64_t last_frame = 0;
    if(!last_frame) last_frame = esp_timer_get_time();

    while (true) {
        /* --- FSM cập nhật theo API /enroll /delete --- */
        if (g_is_enrolling) {
            g_state = START_ENROLL;
        } else if (g_is_deleting) {
            g_is_deleting = 0;
            g_state = START_DELETE;
        } else if (g_state != START_ENROLL) {
            g_state = (st_face_list.count == 0) ? START_DETECT : START_RECOGNITION;
        }

        if (g_state == START_DELETE) {
            int8_t left = delete_face_id_in_flash(&st_face_list);
            if (left >= 0) ESP_LOGW(TAG, "%d ID Left", left);
            g_state = START_DETECT;
            continue;
        }

        camera_fb_t *fb = esp_camera_fb_get();
        if (!fb) {
            ESP_LOGE(TAG, "Camera capture failed");
            return ESP_FAIL;
        }

        dl_matrix3du_t *rgb = dl_matrix3du_alloc(1, fb->width, fb->height, 3);
        if (!rgb) {
            esp_camera_fb_return(fb);
            ESP_LOGE(TAG, "dl_matrix3du_alloc failed");
            return ESP_FAIL;
        }

        uint8_t *jpeg_buf = NULL;
        size_t   jpeg_len = 0;

        if (!fmt2rgb888(fb->buf, fb->len, fb->format, rgb->item)) {
            /* Không convert được: trả về frame JPEG thô */
            jpeg_buf = fb->buf;
            jpeg_len = fb->len;
        } else {
            /* Detect & recognize */
            box_array_t *boxes = face_detect(rgb, &mtmn_config);
            if (boxes) {
                if ((g_state == START_ENROLL || g_state == START_RECOGNITION) &&
                    (align_face(boxes, rgb, aligned_face) == ESP_OK)) {

                    if (g_state == START_ENROLL) {
                        rgb_print(rgb, FACE_COLOR_YELLOW, "START ENROLLING");
                        int left = enroll_face_id_to_flash(&st_face_list, aligned_face);
                        if (left == 0) {
                            ESP_LOGI(TAG, "Enrolled Face ID: %d",
                                     st_face_list.tail ? st_face_list.tail - 1 : FACE_ID_SAVE_NUMBER - 1);
                            g_is_enrolling = 0;
                            g_state = START_RECOGNITION;
                        }
                    } else {
                        int id = recognize_face(&st_face_list, aligned_face);
                        if (id >= 0) {
                            gpio_set_level(GPIO_LED_RED, 1);
                            char buf[32]; snprintf(buf, sizeof(buf), "Hello ID %d", id);
                            rgb_print(rgb, FACE_COLOR_GREEN, buf);
                        } else {
                            rgb_print(rgb, FACE_COLOR_RED, "WHO?");
                        }
                    }
                }
                draw_face_boxes(rgb, boxes);
                dl_lib_free(boxes->score);
                dl_lib_free(boxes->box);
                dl_lib_free(boxes->landmark);
                dl_lib_free(boxes);
            }
            if(!fmt2jpg(rgb->item, fb->width*fb->height*3, fb->width, fb->height,
                        PIXFORMAT_RGB888, 90, &jpeg_buf, &jpeg_len)) {
                ESP_LOGE(TAG, "fmt2jpg failed");
            }
        }

        /* stream MJPEG chunk */
        char header[64];
        size_t hlen = snprintf(header, sizeof(header), _STREAM_PART, (unsigned)jpeg_len);
        res = httpd_resp_send_chunk(req, header, hlen);
        if (res == ESP_OK) res = httpd_resp_send_chunk(req, (const char *)jpeg_buf, jpeg_len);
        if (res == ESP_OK) res = httpd_resp_send_chunk(req, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));

        /* free buffers */
        if (fb) esp_camera_fb_return(fb);
        if (jpeg_buf && jpeg_buf != fb->buf) free(jpeg_buf);
        dl_matrix3du_free(rgb);
        if (res != ESP_OK) break;

        last_frame = esp_timer_get_time();
    }

    g_state = WAIT_FOR_WAKEUP;
    return res;
}

/* ====================== Root & simple capture ====================== */
static esp_err_t root_get_handler(httpd_req_t *req)
{
    const char* resp =
        "<!DOCTYPE html><html><head><meta charset='utf-8'><title>ESP32-CAM</title></head><body>"
        "<h2>ESP32-CAM Face Recognition</h2>"
        "<p>Camera streaming: <a href=\"/stream\">Click here</a></p>"
        "<p>Capture image: <a href=\"/capture\">Click here</a></p>"
        "<hr>"
        "<p><b>Enroll new face:</b> <a href=\"/enroll\">/enroll</a></p>"
        "<p><b>Delete last face:</b> <a href=\"/delete\">/delete</a></p>"
        "</body></html>";
    httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

// static esp_err_t capture_handler(httpd_req_t *req)
// {
//     camera_fb_t *fb = esp_camera_fb_get();
//     if (!fb) { httpd_resp_send_500(req); return ESP_FAIL; }
//     httpd_resp_set_type(req, "image/jpeg");
//     httpd_resp_send(req, (const char *)fb->buf, fb->len);
//     esp_camera_fb_return(fb);
//     return ESP_OK;
// }

// ---------------- Capture ----------------
esp_err_t capture_handler(httpd_req_t *req)
{
    camera_fb_t * fb = esp_camera_fb_get();
    if (!fb) {
        ESP_LOGE(TAG, "Camera capture failed");
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    // 👉 chỗ này nè
    httpd_resp_set_type(req, "image/jpeg");
    httpd_resp_send(req, (const char *)fb->buf, fb->len);

    esp_camera_fb_return(fb);
    return ESP_OK;
}

/* ====================== Enroll/Delete APIs ====================== */
static esp_err_t enroll_handler(httpd_req_t *req)
{
    g_is_enrolling = 1;
    g_is_deleting  = 0;
    g_state        = START_ENROLL;
    httpd_resp_sendstr(req, "Enroll started. Look at the camera until message turns green.");
    return ESP_OK;
}

static esp_err_t delete_handler(httpd_req_t *req)
{
    g_is_enrolling = 0;
    g_is_deleting  = 1;  // xử lý trong vòng lặp stream
    g_state        = START_DELETE;
    httpd_resp_sendstr(req, "Deleting last face ID (done in stream loop).");
    return ESP_OK;
}

/* ====================== URI table ====================== */
static httpd_uri_t uri_root = {
    .uri = "/", .method = HTTP_GET, .handler = root_get_handler
};
static httpd_uri_t uri_capture = {
    .uri = "/capture", .method = HTTP_GET, .handler = capture_handler
};
static httpd_uri_t uri_stream = {
    .uri = "/stream", .method = HTTP_GET, .handler = facenet_stream_handler
};
/* giữ tương thích nếu mã khác gọi /face_stream */
static httpd_uri_t uri_face_stream = {
    .uri = "/face_stream", .method = HTTP_GET, .handler = facenet_stream_handler
};
static httpd_uri_t uri_enroll = {
    .uri = "/enroll", .method = HTTP_GET, .handler = enroll_handler
};
static httpd_uri_t uri_delete = {
    .uri = "/delete", .method = HTTP_GET, .handler = delete_handler
};

/* ====================== Init ====================== */
void app_httpserver_init(void)
{
    // httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    // config.stack_size = 4096 * 2;

    // /* Nút cứng ENROLL/DELETE (nếu dùng) – đảm bảo có ISR service */
    // gpio_install_isr_service(0);
    // gpio_config_t io_conf = {0};
    // io_conf.mode = GPIO_MODE_INPUT;
    // io_conf.intr_type = GPIO_PIN_INTR_POSEDGE;
    // io_conf.pin_bit_mask = (1ULL << GPIO_BUTTON) | (1ULL << GPIO_BOOT);
    // io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    // gpio_config(&io_conf);
    // gpio_isr_handler_add(GPIO_BUTTON, gpio_isr_handler_enroll, NULL);
    // gpio_isr_handler_add(GPIO_BOOT,   gpio_isr_handler_delete, NULL);

    // /* Face DB */
    // face_id_init(&st_face_list, FACE_ID_SAVE_NUMBER, ENROLL_CONFIRM_TIMES);
    // aligned_face = dl_matrix3du_alloc(1, FACE_WIDTH, FACE_HEIGHT, 3);
    // read_face_id_from_flash(&st_face_list);

    // if (httpd_start(&camera_httpd, &config) == ESP_OK) {
    //     httpd_register_uri_handler(camera_httpd, &uri_root);
    //     httpd_register_uri_handler(camera_httpd, &uri_capture);
    //     httpd_register_uri_handler(camera_httpd, &uri_stream);
    //     httpd_register_uri_handler(camera_httpd, &uri_face_stream);
    //     httpd_register_uri_handler(camera_httpd, &uri_enroll);
    //     httpd_register_uri_handler(camera_httpd, &uri_delete);
    //     ESP_LOGI(TAG, "HTTP server started");
    // }

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.stack_size = 4096 * 2;

    /* Không cần cài ISR nữa nếu chỉ dùng web để enroll/delete */
    // gpio_install_isr_service(0);
    // gpio_config_t io_conf = {0};
    // io_conf.mode = GPIO_MODE_INPUT;
    // io_conf.intr_type = GPIO_PIN_INTR_POSEDGE;
    // io_conf.pin_bit_mask = (1ULL << GPIO_BUTTON) | (1ULL << GPIO_BOOT);
    // io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    // gpio_config(&io_conf);
    // gpio_isr_handler_add(GPIO_BUTTON, gpio_isr_handler_enroll, NULL);
    // gpio_isr_handler_add(GPIO_BOOT,   gpio_isr_handler_delete, NULL);

    /* Face DB */
    face_id_init(&st_face_list, FACE_ID_SAVE_NUMBER, ENROLL_CONFIRM_TIMES);
    aligned_face = dl_matrix3du_alloc(1, FACE_WIDTH, FACE_HEIGHT, 3);
    read_face_id_from_flash(&st_face_list);

    if (httpd_start(&camera_httpd, &config) == ESP_OK) {
        httpd_register_uri_handler(camera_httpd, &uri_root);
        httpd_register_uri_handler(camera_httpd, &uri_capture);
        httpd_register_uri_handler(camera_httpd, &uri_stream);
        httpd_register_uri_handler(camera_httpd, &uri_face_stream);
        httpd_register_uri_handler(camera_httpd, &uri_enroll);
        httpd_register_uri_handler(camera_httpd, &uri_delete);
        ESP_LOGI(TAG, "HTTP server started");
    }
}



