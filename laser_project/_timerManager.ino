

class TimerManager
{
    typedef void (*Func)();

private:
    int delay = 0;
    unsigned long startTime;
    Func pFunc;

public:
    TimerManager(int delay, Func pFunc)
    {
        this->delay = delay;
        this->pFunc = pFunc;
        this->startTime = millis();
    }

    void setDelay(int delay)
    {
        this->delay = delay;
    }

    void loop()
    {
        if (millis() - this->startTime > this->delay)
        {
            this->startTime = millis();
            this->pFunc();
        }
    }
};