/// -*- tab-width: 4; Mode: C++; c-basic-offset: 4; indent-tabs-mode: nil -*-
/*
 * quaternion.cpp
 * Copyright (C) Andrew Tridgell 2012
 *
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
 */

#include "AP_Math.h"

// return the rotation matrix equivalent for this quaternion
void Quaternion::rotation_matrix(Matrix3f &m) const
{
    float q3q3 = q3 * q3;
    float q3q4 = q3 * q4;
    float q2q2 = q2 * q2;
    float q2q3 = q2 * q3;
    float q2q4 = q2 * q4;
    float q1q2 = q1 * q2;
    float q1q3 = q1 * q3;
    float q1q4 = q1 * q4;
    float q4q4 = q4 * q4;

    m.a.x = 1-2*(q3q3 + q4q4);
    m.a.y =   2*(q2q3 - q1q4);
    m.a.z =   2*(q2q4 + q1q3);
    m.b.x =   2*(q2q3 + q1q4);
    m.b.y = 1-2*(q2q2 + q4q4);
    m.b.z =   2*(q3q4 - q1q2);
    m.c.x =   2*(q2q4 - q1q3);
    m.c.y =   2*(q3q4 + q1q2);
    m.c.z = 1-2*(q2q2 + q3q3);
}

// return the rotation matrix equivalent for this quaternion
// Thanks to Martin John Baker
// http://www.euclideanspace.com/maths/geometry/rotations/conversions/matrixToQuaternion/index.htm
void Quaternion::from_rotation_matrix(const Matrix3f &m)
{
    const float &m00 = m.a.x;
    const float &m11 = m.b.y;
    const float &m22 = m.c.z;
    const float &m10 = m.b.x;
    const float &m01 = m.a.y;
    const float &m20 = m.c.x;
    const float &m02 = m.a.z;
    const float &m21 = m.c.y;
    const float &m12 = m.b.z;
    float &qw = q1;
    float &qx = q2;
    float &qy = q3;
    float &qz = q4;

    float tr = m00 + m11 + m22;

    if (tr > 0) {
        float S = sqrtf(tr+1) * 2;
        qw = 0.25f * S;
        qx = (m21 - m12) / S;
        qy = (m02 - m20) / S; 
        qz = (m10 - m01) / S; 
    } else if ((m00 > m11) && (m00 > m22)) { 
        float S = sqrtf(1.0 + m00 - m11 - m22) * 2;
        qw = (m21 - m12) / S;
        qx = 0.25f * S;
        qy = (m01 + m10) / S; 
        qz = (m02 + m20) / S; 
    } else if (m11 > m22) { 
        float S = sqrtf(1.0 + m11 - m00 - m22) * 2;
        qw = (m02 - m20) / S;
        qx = (m01 + m10) / S; 
        qy = 0.25f * S;
        qz = (m12 + m21) / S; 
    } else { 
        float S = sqrtf(1.0 + m22 - m00 - m11) * 2;
        qw = (m10 - m01) / S;
        qx = (m02 + m20) / S;
        qy = (m12 + m21) / S;
        qz = 0.25f * S;
    }
}

// convert a vector from earth to body frame
void Quaternion::earth_to_body(Vector3f &v) const
{
    Matrix3f m;
    // we reverse z before and afterwards because of the differing
    // quaternion conventions from APM conventions.
    v.z = -v.z;
    rotation_matrix(m);
    v = m * v;
    v.z = -v.z;
}

// create a quaternion from Euler angles
void Quaternion::from_euler(float roll, float pitch, float yaw)
{
    float cr2 = cosf(roll*0.5f);
    float cp2 = cosf(pitch*0.5f);
    float cy2 = cosf(yaw*0.5f);
    float sr2 = sinf(roll*0.5f);
    float sp2 = sinf(pitch*0.5f);
    float sy2 = sinf(yaw*0.5f);

    q1 = cr2*cp2*cy2 + sr2*sp2*sy2;
    q2 = sr2*cp2*cy2 - cr2*sp2*sy2;
    q3 = cr2*sp2*cy2 + sr2*cp2*sy2;
    q4 = cr2*cp2*sy2 - sr2*sp2*cy2;
}

// create eulers from a quaternion
void Quaternion::to_euler(float *roll, float *pitch, float *yaw) const
{
    if (roll) {
        *roll = (atan2f(2.0f*(q1*q2 + q3*q4),
                       1 - 2.0f*(q2*q2 + q3*q3)));
    }
    if (pitch) {
        // we let safe_asin() handle the singularities near 90/-90 in pitch
        *pitch = safe_asin(2.0f*(q1*q3 - q4*q2));
    }
    if (yaw) {
        *yaw = atan2f(2.0f*(q1*q4 + q2*q3),
                      1 - 2.0f*(q3*q3 + q4*q4));
    }
}

float Quaternion::length(void) const
{
    return sqrtf(sq(q1) + sq(q2) + sq(q3) + sq(q4));
}

void Quaternion::normalize(void)
{
    float quatMag = length();
    if (quatMag > 1e-16f)
    {
        float quatMagInv = 1.0f/quatMag;
        q1 *= quatMagInv;
        q2 *= quatMagInv;
        q3 *= quatMagInv;
        q4 *= quatMagInv;
    }    
}
