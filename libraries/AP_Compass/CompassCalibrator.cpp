#include "CompassCalibrator.h"
#include <AP_HAL.h>

extern const AP_HAL::HAL& hal;

////////////////////////////////////////////////////////////
///////////////////// PUBLIC INTERFACE /////////////////////
////////////////////////////////////////////////////////////

CompassCalibrator::CompassCalibrator():
_tolerance(COMPASS_CAL_DEFAULT_TOLERANCE)
{
    clear();
}

void CompassCalibrator::clear() {
    set_status(COMPASS_CAL_NOT_STARTED);
}

void CompassCalibrator::start(bool retry, bool autosave, float delay) {
    if(running()) {
        return;
    }
    _autosave = autosave;
    _attempt = 1;
    _retry = retry;
    _delay_start_sec = delay;
    _start_time_ms = hal.scheduler->millis();
    set_status(COMPASS_CAL_WAITING_TO_START);
}

void CompassCalibrator::get_calibration(Vector3f &offsets, Vector3f &diagonals, Vector3f &offdiagonals) {
    if (_status != COMPASS_CAL_SUCCESS) {
        return;
    }

    offsets = _params.offset;
    diagonals = _params.diag;
    offdiagonals = _params.offdiag;
}

float CompassCalibrator::get_completion_percent() const {
    // first sampling step is 1/3rd of the progress bar
    // never return more than 99% unless _status is COMPASS_CAL_SUCCESS
    switch(_status) {
        case COMPASS_CAL_NOT_STARTED:
        case COMPASS_CAL_WAITING_TO_START:
            return 0.0f;
        case COMPASS_CAL_RUNNING_STEP_ONE:
            return 33.3f * _samples_collected/COMPASS_CAL_NUM_SAMPLES;
        case COMPASS_CAL_RUNNING_STEP_TWO:
            return 33.3f + 65.7f*((float)(_samples_collected-_samples_thinned)/(COMPASS_CAL_NUM_SAMPLES-_samples_thinned));
        case COMPASS_CAL_SUCCESS:
            return 100.0f;
        case COMPASS_CAL_FAILED:
        default:
            return 0.0f;
    };
}

bool CompassCalibrator::check_for_timeout() {
    uint32_t tnow = hal.scheduler->millis();
    if(running() && tnow - _last_sample_ms > 1000) {
        _retry = false;
        set_status(COMPASS_CAL_FAILED);
        return true;
    }
    return false;
}

void CompassCalibrator::new_sample(const Vector3f& sample) {
    _last_sample_ms = hal.scheduler->millis();

    if(_status == COMPASS_CAL_WAITING_TO_START) {
        set_status(COMPASS_CAL_RUNNING_STEP_ONE);
    }

    if(running() && _samples_collected < COMPASS_CAL_NUM_SAMPLES && accept_sample(sample)) {
        _sample_buffer[_samples_collected].set(sample);
        _samples_collected++;
    }
}

void CompassCalibrator::update(bool &failure) {
    failure = false;

    if(!fitting()) {
        return;
    }

    if(_status == COMPASS_CAL_RUNNING_STEP_ONE) {
        if (_fit_step >= 10) {
            if(_fitness == _initial_fitness || isnan(_fitness)) {           //if true, means that fitness is diverging instead of converging
                set_status(COMPASS_CAL_FAILED);
                failure = true;
            }
            set_status(COMPASS_CAL_RUNNING_STEP_TWO);
        } else {
            run_sphere_fit();
            _fit_step++;
        }
    } else if(_status == COMPASS_CAL_RUNNING_STEP_TWO) {
        if (_fit_step >= 35) {
            if(fit_acceptable()) {
                set_status(COMPASS_CAL_SUCCESS);
            } else {
                set_status(COMPASS_CAL_FAILED);
                failure = true;
            }
        } else if (_fit_step < 15) {
            run_sphere_fit();
            _fit_step++;
        } else {
            run_ellipsoid_fit();
            _fit_step++;
        }
    }
}

/////////////////////////////////////////////////////////////
////////////////////// PRIVATE METHODS //////////////////////
/////////////////////////////////////////////////////////////
bool CompassCalibrator::running() const {
    return _status == COMPASS_CAL_RUNNING_STEP_ONE || _status == COMPASS_CAL_RUNNING_STEP_TWO;
}

bool CompassCalibrator::fitting() const {
    return running() && _samples_collected == COMPASS_CAL_NUM_SAMPLES;
}

void CompassCalibrator::initialize_fit() {
    //initialize _fitness before starting a fit
    if (_samples_collected != 0) {
        _fitness = calc_mean_squared_residuals(_params);
    } else {
        _fitness = 1.0e30f;
    }
    _ellipsoid_lambda = 1.0f;
    _sphere_lambda = 1.0f;
    _initial_fitness = _fitness;
    _fit_step = 0;
}

void CompassCalibrator::reset_state() {
    _samples_collected = 0;
    _samples_thinned = 0;
    _params.radius = 200;
    _params.offset.zero();
    _params.diag = Vector3f(1.0f,1.0f,1.0f);
    _params.offdiag.zero();

    initialize_fit();
}

bool CompassCalibrator::set_status(compass_cal_status_t status) {
    if (status != COMPASS_CAL_NOT_STARTED && _status == status) {
        return true;
    }

    switch(status) {
        case COMPASS_CAL_NOT_STARTED:
            reset_state();
            _status = COMPASS_CAL_NOT_STARTED;

            if(_sample_buffer != NULL) {
                free(_sample_buffer);
                _sample_buffer = NULL;
            }
            return true;

        case COMPASS_CAL_WAITING_TO_START:
            reset_state();
            _status = COMPASS_CAL_WAITING_TO_START;

            set_status(COMPASS_CAL_RUNNING_STEP_ONE);
            return true;

        case COMPASS_CAL_RUNNING_STEP_ONE:
            if(_status != COMPASS_CAL_WAITING_TO_START) {
                return false;
            }

            if(_attempt == 1 && (hal.scheduler->millis()-_start_time_ms)*1.0e-3f < _delay_start_sec) {
                return false;
            }

            if(_sample_buffer != NULL) {
                initialize_fit();
                _status = COMPASS_CAL_RUNNING_STEP_ONE;
                return true;
            }

            _sample_buffer = (CompassSample*)malloc(sizeof(CompassSample)*COMPASS_CAL_NUM_SAMPLES);

            if(_sample_buffer != NULL) {
                initialize_fit();
                _status = COMPASS_CAL_RUNNING_STEP_ONE;
                return true;
            }

            return false;

        case COMPASS_CAL_RUNNING_STEP_TWO:
            if(_status != COMPASS_CAL_RUNNING_STEP_ONE) {
                return false;
            }
            thin_samples();
            initialize_fit();
            _status = COMPASS_CAL_RUNNING_STEP_TWO;
            return true;

        case COMPASS_CAL_SUCCESS:
            if(_status != COMPASS_CAL_RUNNING_STEP_TWO) {
                return false;
            }

            if(_sample_buffer != NULL) {
                free(_sample_buffer);
                _sample_buffer = NULL;
            }

            _status = COMPASS_CAL_SUCCESS;
            return true;

        case COMPASS_CAL_FAILED:
            if(_status == COMPASS_CAL_NOT_STARTED) {
                return false;
            }

            if(_retry && set_status(COMPASS_CAL_WAITING_TO_START)) {
                _attempt++;
                return true;
            }

            if(_sample_buffer != NULL) {
                free(_sample_buffer);
                _sample_buffer = NULL;
            }

            _status = COMPASS_CAL_FAILED;
            return true;

        default:
            return false;
    };
}

bool CompassCalibrator::fit_acceptable() {
    if( !isnan(_fitness) &&
        _params.radius > 150 && _params.radius < 950 && //Earth's magnetic field strength range: 250-850mG
        fabsf(_params.offset.x) < 1000 &&
        fabsf(_params.offset.y) < 1000 &&
        fabsf(_params.offset.z) < 1000 &&
        _params.diag.x > 0.2f && _params.diag.x < 5.0f &&
        _params.diag.y > 0.2f && _params.diag.y < 5.0f &&
        _params.diag.z > 0.2f && _params.diag.z < 5.0f &&
        fabsf(_params.offdiag.x) <  1.0f &&      //absolute of sine/cosine output cannot be greater than 1
        fabsf(_params.offdiag.y) <  1.0f &&
        fabsf(_params.offdiag.z) <  1.0f ){

            return _fitness <= sq(_tolerance);
        }
    return false;
}

void CompassCalibrator::thin_samples() {
    if(_sample_buffer == NULL) {
        return;
    }

    _samples_thinned = 0;
    // shuffle the samples http://en.wikipedia.org/wiki/Fisher%E2%80%93Yates_shuffle
    // this is so that adjacent samples don't get sequentially eliminated
    for(uint16_t i=_samples_collected-1; i>=1; i--) {
        uint16_t j = get_random() % (i+1);
        CompassSample temp = _sample_buffer[i];
        _sample_buffer[i] = _sample_buffer[j];
        _sample_buffer[j] = temp;
    }

    for(uint16_t i=0; i < _samples_collected; i++) {
        if(!accept_sample(_sample_buffer[i])) {
            _sample_buffer[i] = _sample_buffer[_samples_collected-1];
            _samples_collected --;
            _samples_thinned ++;
        }
    }
}

bool CompassCalibrator::accept_sample(const Vector3f& sample)
{
    if(_sample_buffer == NULL) {
        return false;
    }

    float min_distance = fabsf(5.38709f * _params.radius / sqrtf((float)COMPASS_CAL_NUM_SAMPLES)) / 3.0f;

    for (uint16_t i = 0; i<_samples_collected; i++){
        float distance = (sample - _sample_buffer[i].get()).length();
        if(distance < min_distance) {
            return false;
        }
    }
    return true;
}

bool CompassCalibrator::accept_sample(const CompassSample& sample) {
    return accept_sample(sample.get());
}

float CompassCalibrator::calc_residual(const Vector3f& sample, const param_t& params) const {
    Matrix3f softiron(
        params.diag.x    , params.offdiag.x , params.offdiag.y,
        params.offdiag.x , params.diag.y    , params.offdiag.z,
        params.offdiag.y , params.offdiag.z , params.diag.z
    );
    return params.radius - (softiron*(sample+params.offset)).length();
}

float CompassCalibrator::calc_mean_squared_residuals() const
{
    return calc_mean_squared_residuals(_params);
}

float CompassCalibrator::calc_mean_squared_residuals(const param_t& params) const
{
    if(_sample_buffer == NULL || _samples_collected == 0) {
        return 1.0e30f;
    }
    float sum = 0.0f;
    for(uint16_t i=0; i < _samples_collected; i++){
        Vector3f sample = _sample_buffer[i].get();
        float resid = calc_residual(sample, params);
        sum += sq(resid);
    }
    sum /= _samples_collected;
    return sum;
}

void CompassCalibrator::calc_sphere_jacob(const Vector3f& sample, const param_t& params, float* ret) const{
    const Vector3f &offset = params.offset;
    const Vector3f &diag = params.diag;
    const Vector3f &offdiag = params.offdiag;
    Matrix3f softiron(
        diag.x    , offdiag.x , offdiag.y,
        offdiag.x , diag.y    , offdiag.z,
        offdiag.y , offdiag.z , diag.z
    );

    float A =  (diag.x    * (sample.x + offset.x)) + (offdiag.x * (sample.y + offset.y)) + (offdiag.y * (sample.z + offset.z));
    float B =  (offdiag.x * (sample.x + offset.x)) + (diag.y    * (sample.y + offset.y)) + (offdiag.z * (sample.z + offset.z));
    float C =  (offdiag.y * (sample.x + offset.x)) + (offdiag.z * (sample.y + offset.y)) + (diag.z    * (sample.z + offset.z));
    float length = (softiron*(sample+offset)).length();

    // 0: radius
    ret[0] = 1.0f;
    // 1-3: offsets
    ret[1] = -1.0f * (((diag.x    * A) + (offdiag.x * B) + (offdiag.y * C))/length);
    ret[2] = -1.0f * (((offdiag.x * A) + (diag.y    * B) + (offdiag.z * C))/length);
    ret[3] = -1.0f * (((offdiag.y * A) + (offdiag.z * B) + (diag.z    * C))/length);
}

void CompassCalibrator::run_sphere_fit()
{
    if(_sample_buffer == NULL) {
        return;
    }

    const float lma_damping = 10.0f;

    float fitness = _fitness;
    float fit1, fit2;
    param_t fit1_params, fit2_params;
    fit1_params = fit2_params = _params;

    float JTJ[COMPASS_CAL_NUM_SPHERE_PARAMS*COMPASS_CAL_NUM_SPHERE_PARAMS];
    float JTJ2[COMPASS_CAL_NUM_SPHERE_PARAMS*COMPASS_CAL_NUM_SPHERE_PARAMS];
    float JTFI[COMPASS_CAL_NUM_SPHERE_PARAMS];

    memset(&JTJ,0,sizeof(JTJ));
    memset(&JTJ2,0,sizeof(JTJ2));
    memset(&JTFI,0,sizeof(JTFI));

    for(uint16_t k = 0; k<_samples_collected; k++) {
        Vector3f sample = _sample_buffer[k].get();

        float sphere_jacob[COMPASS_CAL_NUM_SPHERE_PARAMS];

        calc_sphere_jacob(sample, fit1_params, sphere_jacob);

        for(uint8_t i = 0;i < COMPASS_CAL_NUM_SPHERE_PARAMS; i++) {
            // compute JTJ
            for(uint8_t j = 0; j < COMPASS_CAL_NUM_SPHERE_PARAMS; j++) {
                JTJ[i*COMPASS_CAL_NUM_SPHERE_PARAMS+j] += sphere_jacob[i] * sphere_jacob[j];
                JTJ2[i*COMPASS_CAL_NUM_SPHERE_PARAMS+j] += sphere_jacob[i] * sphere_jacob[j];   //a backup JTJ for LM
            }
            // compute JTFI
            JTFI[i] += sphere_jacob[i] * calc_residual(sample, fit1_params);
        }
    }


    //------------------------Levenberg-part-starts-here---------------------------------//
    for(uint8_t i = 0; i < COMPASS_CAL_NUM_SPHERE_PARAMS; i++) {
        JTJ[i*COMPASS_CAL_NUM_SPHERE_PARAMS+i] += _sphere_lambda;
        JTJ2[i*COMPASS_CAL_NUM_SPHERE_PARAMS+i] += _sphere_lambda/lma_damping;
    }

    if(!inverse(JTJ, JTJ, 4)) {
        return;
    }

    if(!inverse(JTJ2, JTJ2, 4)) {
        return;
    }

    for(uint8_t row=0; row < COMPASS_CAL_NUM_SPHERE_PARAMS; row++) {
        for(uint8_t col=0; col < COMPASS_CAL_NUM_SPHERE_PARAMS; col++) {
            fit1_params.get_sphere_params()[row] -= JTFI[col] * JTJ[row*COMPASS_CAL_NUM_SPHERE_PARAMS+col];
            fit2_params.get_sphere_params()[row] -= JTFI[col] * JTJ2[row*COMPASS_CAL_NUM_SPHERE_PARAMS+col];
        }
    }

    fit1 = calc_mean_squared_residuals(fit1_params);
    fit2 = calc_mean_squared_residuals(fit2_params);

    if(fit1 > _fitness && fit2 > _fitness){
        _sphere_lambda *= lma_damping;
    } else if(fit2 < _fitness && fit2 < fit1) {
        _sphere_lambda /= lma_damping;
        fit1_params = fit2_params;
        fitness = fit2;
    } else if(fit1 < _fitness){
        fitness = fit1;
    }
    //--------------------Levenberg-part-ends-here--------------------------------//

    if(!isnan(fitness) && fitness < _fitness) {
        _fitness = fitness;
        _params = fit1_params;
    }
}



void CompassCalibrator::calc_ellipsoid_jacob(const Vector3f& sample, const param_t& params, float* ret) const{
    const Vector3f &offset = params.offset;
    const Vector3f &diag = params.diag;
    const Vector3f &offdiag = params.offdiag;
    Matrix3f softiron(
        diag.x    , offdiag.x , offdiag.y,
        offdiag.x , diag.y    , offdiag.z,
        offdiag.y , offdiag.z , diag.z
    );

    float A =  (diag.x    * (sample.x + offset.x)) + (offdiag.x * (sample.y + offset.y)) + (offdiag.y * (sample.z + offset.z));
    float B =  (offdiag.x * (sample.x + offset.x)) + (diag.y    * (sample.y + offset.y)) + (offdiag.z * (sample.z + offset.z));
    float C =  (offdiag.y * (sample.x + offset.x)) + (offdiag.z * (sample.y + offset.y)) + (diag.z    * (sample.z + offset.z));
    float length = (softiron*(sample+offset)).length();

    // 0-2: offsets
    ret[0] = -1.0f * (((diag.x    * A) + (offdiag.x * B) + (offdiag.y * C))/length);
    ret[1] = -1.0f * (((offdiag.x * A) + (diag.y    * B) + (offdiag.z * C))/length);
    ret[2] = -1.0f * (((offdiag.y * A) + (offdiag.z * B) + (diag.z    * C))/length);
    // 3-5: diagonals
    ret[3] = -1.0f * ((sample.x + offset.x) * A)/length;
    ret[4] = -1.0f * ((sample.y + offset.y) * B)/length;
    ret[5] = -1.0f * ((sample.z + offset.z) * C)/length;
    // 6-8: off-diagonals
    ret[6] = -1.0f * (((sample.y + offset.y) * A) + ((sample.x + offset.x) * B))/length;
    ret[7] = -1.0f * (((sample.z + offset.z) * A) + ((sample.x + offset.x) * C))/length;
    ret[8] = -1.0f * (((sample.z + offset.z) * B) + ((sample.y + offset.y) * C))/length;
}

void CompassCalibrator::run_ellipsoid_fit()
{
    if(_sample_buffer == NULL) {
        return;
    }

    const float lma_damping = 10.0f;


    float fitness = _fitness;
    float fit1, fit2;
    param_t fit1_params, fit2_params;
    fit1_params = fit2_params = _params;


    float JTJ[COMPASS_CAL_NUM_ELLIPSOID_PARAMS*COMPASS_CAL_NUM_ELLIPSOID_PARAMS];
    float JTJ2[COMPASS_CAL_NUM_ELLIPSOID_PARAMS*COMPASS_CAL_NUM_ELLIPSOID_PARAMS];
    float JTFI[COMPASS_CAL_NUM_ELLIPSOID_PARAMS];

    memset(&JTJ,0,sizeof(JTJ));
    memset(&JTJ2,0,sizeof(JTJ2));
    memset(&JTFI,0,sizeof(JTFI));

    for(uint16_t k = 0; k<_samples_collected; k++) {
        Vector3f sample = _sample_buffer[k].get();

        float ellipsoid_jacob[COMPASS_CAL_NUM_ELLIPSOID_PARAMS];

        calc_ellipsoid_jacob(sample, fit1_params, ellipsoid_jacob);

        for(uint8_t i = 0;i < COMPASS_CAL_NUM_ELLIPSOID_PARAMS; i++) {
            // compute JTJ
            for(uint8_t j = 0; j < COMPASS_CAL_NUM_ELLIPSOID_PARAMS; j++) {
                JTJ [i*COMPASS_CAL_NUM_ELLIPSOID_PARAMS+j] += ellipsoid_jacob[i] * ellipsoid_jacob[j];
                JTJ2[i*COMPASS_CAL_NUM_ELLIPSOID_PARAMS+j] += ellipsoid_jacob[i] * ellipsoid_jacob[j];
            }
            // compute JTFI
            JTFI[i] += ellipsoid_jacob[i] * calc_residual(sample, fit1_params);
        }
    }



    //------------------------Levenberg-part-starts-here---------------------------------//
    for(uint8_t i = 0; i < COMPASS_CAL_NUM_ELLIPSOID_PARAMS; i++) {
        JTJ[i*COMPASS_CAL_NUM_ELLIPSOID_PARAMS+i] += _ellipsoid_lambda;
        JTJ2[i*COMPASS_CAL_NUM_ELLIPSOID_PARAMS+i] += _ellipsoid_lambda/lma_damping;
    }

    if(!inverse(JTJ, JTJ, 9)) {
        return;
    }

    if(!inverse(JTJ2, JTJ2, 9)) {
        return;
    }

    for(uint8_t row=0; row < COMPASS_CAL_NUM_ELLIPSOID_PARAMS; row++) {
        for(uint8_t col=0; col < COMPASS_CAL_NUM_ELLIPSOID_PARAMS; col++) {
            fit1_params.get_ellipsoid_params()[row] -= JTFI[col] * JTJ[row*COMPASS_CAL_NUM_ELLIPSOID_PARAMS+col];
            fit2_params.get_ellipsoid_params()[row] -= JTFI[col] * JTJ2[row*COMPASS_CAL_NUM_ELLIPSOID_PARAMS+col];
        }
    }

    fit1 = calc_mean_squared_residuals(fit1_params);
    fit2 = calc_mean_squared_residuals(fit2_params);

    if(fit1 > _fitness && fit2 > _fitness){
        _ellipsoid_lambda *= lma_damping;
    } else if(fit2 < _fitness && fit2 < fit1) {
        _ellipsoid_lambda /= lma_damping;
        fit1_params = fit2_params;
        fitness = fit2;
    } else if(fit1 < _fitness){
        fitness = fit1;
    }
    //--------------------Levenberg-part-ends-here--------------------------------//

    if(fitness < _fitness) {
        _fitness = fitness;
        _params = fit1_params;
    }
}


uint16_t CompassCalibrator::get_random(void)
{
    static uint32_t m_z = 1234;
    static uint32_t m_w = 76542;
    m_z = 36969 * (m_z & 65535) + (m_z >> 16);
    m_w = 18000 * (m_w & 65535) + (m_w >> 16);
    return ((m_z << 16) + m_w) & 0xFFFF;
}

//////////////////////////////////////////////////////////
//////////// CompassSample public interface //////////////
//////////////////////////////////////////////////////////

Vector3f CompassCalibrator::CompassSample::get() const {
    Vector3f out;
    out.x = (float)x*2048.0f/32700.0f;
    out.y = (float)y*2048.0f/32700.0f;
    out.z = (float)z*2048.0f/32700.0f;
    return out;
}

void CompassCalibrator::CompassSample::set(const Vector3f &in) {
    x = (int16_t)(in.x*32700.0f/2048.0f + 0.5f);
    y = (int16_t)(in.y*32700.0f/2048.0f + 0.5f);
    z = (int16_t)(in.z*32700.0f/2048.0f + 0.5f);
}
