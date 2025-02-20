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

#include <timers/Time.hpp>

volatile uint32_t _freertos_stats_base_ticks = 0;

namespace time {

// Global static pointer used to ensure a single instance of the class.
HighPrecisionTimer* HighPrecisionTimer::_instance = nullptr;

void HighPrecisionTimer::Instantiate(void)
{
	if (_instance == nullptr)
	{
		_instance = new HighPrecisionTimer();
	}
}

HighPrecisionTimer* HighPrecisionTimer::Instance()
{
	return _instance;
}

HighPrecisionTimer::~HighPrecisionTimer()
{
	delete _instance;
}

abs_time_t HighPrecisionTimer::get_absolute_time_us(void)
{
	abs_time_t current_time;
	uint16_t tick_val;

	// Not reentrant
	taskENTER_CRITICAL();

	tick_val = FTM0_CNT;

	current_time = FTM0_PICOS_PER_TICK * (_base_ticks + tick_val);

	current_time /= PICOS_PER_MICRO;

	taskEXIT_CRITICAL();

	return current_time;
}

abs_time_t HighPrecisionTimer::get_absolute_time_us_from_isr(void)
{
	abs_time_t current_time;
	uint16_t tick_val;

	tick_val = FTM0_CNT;

	current_time = FTM0_PICOS_PER_TICK * (_base_ticks + tick_val);

	current_time /= PICOS_PER_MICRO;

	return current_time;
}

void HighPrecisionTimer::handle_timer_overflow(void)
{
	_base_ticks += FTM0_MAX_TICKS;
	_freertos_stats_base_ticks = _base_ticks;

	if (_callback_registered && _callback_enabled)
	{
		_callback();
	}
}

} // end namespace time

// Define ISR for FlexTimer Module 0
extern "C" void ftm0_isr(void)
{
	auto saved_state = taskENTER_CRITICAL_FROM_ISR();

	SEGGER_SYSVIEW_RecordEnterISR();

	if (time::HighPrecisionTimer::Instance() != nullptr)
	{
		time::HighPrecisionTimer::Instance()->handle_timer_overflow();
	}

	// Clear overflow flag
	if ((FTM0_SC & FTM_SC_TOF) != 0)
	{
		FTM0_SC &= ~FTM_SC_TOF;
	}

	SEGGER_SYSVIEW_RecordExitISR();

	taskEXIT_CRITICAL_FROM_ISR(saved_state);
}
