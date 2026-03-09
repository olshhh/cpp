#include <cassert>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <iomanip>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

template <typename D = std::chrono::duration<double>>
class Timer
{
public:
    explicit Timer(std::string const& scope)
        : m_scope(scope), m_begin(), m_running(false), m_measurements()
    {
    }

    ~Timer()
    {
        double value = 0.0;

        if (!m_measurements.empty())
        {
            value = average();
        }

        std::cout << m_scope
                  << " : average = "
                  << std::fixed
                  << std::setprecision(6)
                  << value
                  << " s, runs = "
                  << m_measurements.size()
                  << '\n';
    }

    void start()
    {
        assert(!m_running);
        m_begin = clock_t::now();
        m_running = true;
    }

    void stop()
    {
        assert(m_running);

        duration_t interval = duration_t(clock_t::now() - m_begin);

        m_measurements.push_back(interval);
        m_running = false;
    }

    double average() const
    {
        duration_t sum = duration_t::zero();
        std::size_t i = 0U;

        assert(!m_running);
        assert(!m_measurements.empty());

        for (i = 0U; i < m_measurements.size(); ++i)
        {
            sum += m_measurements[i];
        }

        return (sum / static_cast<double>(m_measurements.size())).count();
    }

    std::size_t count() const
    {
        return m_measurements.size();
    }

private:
    using clock_t = std::chrono::steady_clock;
    using duration_t = std::chrono::duration<double>;

    std::string m_scope;
    clock_t::time_point m_begin;
    bool m_running;
    std::vector<duration_t> m_measurements;
};

double calculate(std::size_t size)
{
    double x = 0.0;
    std::size_t i = 0U;

    for (i = 0U; i < size; ++i)
    {
        double const value = static_cast<double>(i);
        x += std::pow(std::sin(value), 2.0) + std::pow(std::cos(value), 2.0);
    }

    return x;
}

bool equal(double x, double y, double epsilon = 1e-6)
{
    return std::abs(x - y) < epsilon;
}

int main()
{
    std::size_t const work_size = 1000000U;
    std::chrono::milliseconds const sleep_time(10);
    double result = 0.0;
    double avg = 0.0;

    Timer<> timer("main : timer");

    timer.start();
    result = calculate(work_size);
    timer.stop();

    timer.start();
    result += calculate(work_size);
    timer.stop();

    assert(equal(result, 2.0 * static_cast<double>(work_size)));
    assert(timer.count() == 2U);
    assert(timer.average() > 0.0);

    {
        Timer<> sleep_timer("demo : sleep timer");

        sleep_timer.start();
        std::this_thread::sleep_for(sleep_time);
        sleep_timer.stop();

        sleep_timer.start();
        std::this_thread::sleep_for(sleep_time);
        sleep_timer.stop();

        avg = sleep_timer.average();

        assert(sleep_timer.count() == 2U);
        assert(avg > 0.0);
    }

    return 0;
}