/*
 * This file is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This file is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Code by Michael Oborne and Siddharth Bharat Purohit, Cubepilot Pty.
 */
#pragma once

#include <AP_HAL/AP_HAL.h>
#include <AP_ONVIF/onvifDeviceBindingProxy.h>
#include <AP_ONVIF/onvifMediaBindingProxy.h>
#include <AP_ONVIF/onvifPTZBindingProxy.h>
#include <plugin/wsseapi-lite.h>
#include <AP_Math/AP_Math.h>

class AP_ONVIF {
public:
    bool init();
    void set_credentials();
    bool set_absolutemove(float pan, float tilt, float zoom);
    void set_pan_norm(float pan) { pan_norm = pan; }
    void set_tilt_norm(float tilt) { tilt_norm = tilt; }

    // get singleton instance
    // static AP_ONVIF *get_singleton() { return _singleton; }

private:
    void report_error();
    bool probe_onvif_server();
    void rand_nonce(char *nonce, size_t noncelen);
    void update();

    Vector2f pan_tilt_limit_min;
    Vector2f pan_tilt_limit_max;
    float pan_norm, tilt_norm;
    float last_pan_cmd, last_tilt_cmd;

    float zoom_min, zoom_max;
    std::string profile_token;
    struct soap *soap;
    DeviceBindingProxy *proxy_device;
    MediaBindingProxy  *proxy_media;
    PTZBindingProxy    *proxy_ptz;
    static AP_ONVIF *_singleton;
    char* media_endpoint;
};


// namespace AP {
//     AP_ONVIF *onvif();
// };
