// MIT License

// Copyright (c) 2019 Jacob Dahl

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <PIDController.hpp>

namespace controllers
{

PIDController::PIDController(float P, float I, float D, float max_effort, float max_integrator)
	: _kP(P)
	, _kI(I)
	, _kD(D)
	, _max_effort(max_effort)
	, _max_integrator(max_integrator)
{
	// anything to do?
}

float PIDController::get_effort(float target, float current)
{
	float error = target - current;

	_error_integral += error;

	// integral clamp
	if (_error_integral > _max_integrator)
	{
		_error_integral = _max_integrator;
	}
	else if (_error_integral < -_max_integrator)
	{
		_error_integral = -_max_integrator;
	}

	float p_effort = error * _kP;
	float i_effort = _error_integral * _kI;
	float d_effort = (error - _last_error) * _kD;

	float effort = p_effort + i_effort + d_effort;

	if (effort > _max_effort)
	{
		effort = _max_effort;
	}
	else if (effort < -_max_effort)
	{
		effort = -_max_effort;
	}

	_last_error = error;

	return effort;
}

/////----- NON LINEAR PID -----/////
float NonlinearPIDController::get_effort(float target, float current)
{
	float error = target - current;

	_error_integral += error;

	// integral clamp
	if (_error_integral > _max_integrator)
	{
		_error_integral = _max_integrator;
	}
	else if (_error_integral < -_max_integrator)
	{
		_error_integral = -_max_integrator;
	}

	// Non-linear gains
	float p_effort = apply_nonlinear_controller(error, _max_effort, _kP);
	float i_effort = apply_nonlinear_controller(_error_integral, _max_effort, _kI);
	float d_effort = apply_nonlinear_controller(error - _last_error, _max_effort, _kD);

	float effort = p_effort + i_effort + d_effort;

	if (effort > _max_effort)
	{
		effort = _max_effort;
	}
	else if (effort < -_max_effort)
	{
		effort = -_max_effort;
	}

	_last_error = error;

	return effort;
}

float NonlinearPIDController::apply_nonlinear_controller(const float error, const float scale_factor, const float gain)
{
	float effort = scale_factor * std::erf(error * gain);

    if (std::isfinite(effort))
    {
        return effort;
    }
    else
    {
        return 0.;
    }
}

} // end namespace controllers
