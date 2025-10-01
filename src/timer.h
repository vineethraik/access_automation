#include <vector>
// #include <functional>
// typedef function<bool(void)> timer_function_t;
using timer_function_t = bool (*)();
using timer_loop_function_t = void (*)();

struct TIMER_UNIT
{
    int id = 0;
    bool repeate = false;
    bool first_run = true;
    bool run = false;
    unsigned long reference_time = 0;
    unsigned long start_delay = 0;
    unsigned long repeate_delay = 0;
    timer_function_t timer_function = nullptr;
};

class Timer
{
private:
    vector<TIMER_UNIT> events;
    timer_loop_function_t loop_function = nullptr;

public:
    void run();
    void set_loop_function(timer_loop_function_t function) { loop_function = function; }
    void delay(unsigned long);
    int set_new_event(TIMER_UNIT);
    int set_new_event(unsigned long, timer_function_t);
    int set_new_event(unsigned long, unsigned long, timer_function_t);
    void end_event(int);

    int get_new_id();
    TIMER_UNIT get_event(int);
} timer;

void Timer::run()
{
    for (int i = 0; i < events.size(); i++)
    {
        if (events[i].run)
        {
            if (events[i].first_run)
            {
                if ((millis() - events[i].reference_time) < 0)
                    events[i].reference_time = 0;
                if ((millis() - events[i].reference_time) > events[i].start_delay)
                {
                    events[i].reference_time = millis();
                    events[i].first_run = false;
                    events[i].run = events[i].timer_function();
                }
            }
            else
            {
                if (events[i].repeate)
                {
                    if ((millis() - events[i].reference_time) > events[i].repeate_delay)
                    {
                        events[i].reference_time = millis();
                        events[i].run = events[i].timer_function();
                    }
                }
                else
                {
                    events[i].run = false;
                }
            }
        }
        else
        {
            events.erase(events.begin() + i);
        }
    }
}

int Timer::set_new_event(TIMER_UNIT unit)
{
    events.push_back(unit);
    return unit.id;
}

int Timer::set_new_event(unsigned long s_delay, timer_function_t function)
{
    TIMER_UNIT tm;
    tm.id = get_new_id();
    tm.run = true;
    tm.reference_time = millis();
    tm.start_delay = s_delay;
    tm.repeate = false;
    tm.repeate_delay = 0;
    tm.timer_function = function;

    set_new_event(tm);
    return tm.id;
}

int Timer::set_new_event(unsigned long s_delay, unsigned long r_delay, timer_function_t function)
{
    TIMER_UNIT tm;
    tm.id = get_new_id();
    tm.run = true;
    tm.reference_time = millis();
    tm.start_delay = s_delay;
    tm.repeate = true;
    tm.repeate_delay = r_delay;
    tm.timer_function = function;

    set_new_event(tm);
    return tm.id;
}

void Timer::end_event(int _id)
{
    for (int i = 0; i < events.size(); i++)
    {
        if (events[i].id == _id)
        {
            events.erase(events.begin() + i);
        }
    }
}

int Timer::get_new_id()
{
    int x = random(1, 60000);
    for (int i = 0; i < events.size(); i++)
    {
        if (events[i].id == x)
        {
            return get_new_id();
        }
    }
    return x;
}

TIMER_UNIT Timer::get_event(int id)
{
    for (int i = 0; i < events.size(); i++)
    {
    }
    TIMER_UNIT tm;
    return tm;
}

void Timer::delay(unsigned long _delay)
{
    unsigned long current = millis();
    unsigned long next = millis();

    while (next-current < _delay)
    {
        next = millis();
        yield();
        // delay(1);
        if (loop_function != nullptr)
        {
            loop_function();
        }
        
        timer.run();
    }
}