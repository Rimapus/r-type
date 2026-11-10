/*
** EPITECH PROJECT, 2026
** younity-engine/tests/core/Application
** File description:
** Application unit tests
*/

#include <criterion/criterion.h>

#include <atomic>
#include <chrono>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include "younity/core/Application.hpp"

namespace {

// Clock moved by hand by the tests, so the loop never sleeps and the number of steps is exact
struct FakeSource
{
    using duration = std::chrono::nanoseconds;
    using rep = duration::rep;
    using period = duration::period;
    using time_point = std::chrono::time_point<FakeSource>;

    // Named by the std::chrono clock requirements
    static constexpr bool is_steady = true; // NOLINT(readability-identifier-naming)

    // Criterion runs each test in its own process: this time is never shared between tests
    static inline time_point current{};

    static time_point now(void) noexcept
    {
        return current;
    }

    static void advance(std::chrono::nanoseconds elapsed) noexcept
    {
        current += elapsed;
    }
};

using FakeApplication = younity::BasicApplication<FakeSource>;

// Records the hooks it receives, each frame lasting frameTime, and stops after a number of frames
class RecordingApplication : public FakeApplication
{
public:
    RecordingApplication(float step, std::chrono::nanoseconds frameTime, std::size_t frames)
        : FakeApplication(step), _frameTime(frameTime), _frames(frames)
    {
    }

    std::vector<std::string> calls;
    std::vector<float> steps;
    std::vector<float> alphas;

protected:
    void onStart(void) override
    {
        this->calls.emplace_back("start");
    }

    void onInput(void) override
    {
        this->calls.emplace_back("input");
        if (this->_frames == 0) {
            this->stop();
            return;
        }
        --this->_frames;
        FakeSource::advance(this->_frameTime);
    }

    void onFixedUpdate(float step) override
    {
        this->calls.emplace_back("update");
        this->steps.push_back(step);
    }

    void onRender(float alpha) override
    {
        this->calls.emplace_back("render");
        this->alphas.push_back(alpha);
    }

    void onStop(void) override
    {
        this->calls.emplace_back("stop");
    }

private:
    std::chrono::nanoseconds _frameTime;
    std::size_t _frames;
};

// Throws from its first step
class FailingApplication : public FakeApplication
{
public:
    FailingApplication(void)
        : FakeApplication(0.1F)
    {
    }

    bool stopped = false;

protected:
    void onInput(void) override
    {
        FakeSource::advance(std::chrono::milliseconds(100));
    }

    void onFixedUpdate(float /* step */) override
    {
        throw std::runtime_error("broken system");
    }

    void onStop(void) override
    {
        this->stopped = true;
    }
};

// Throws from onStart(), before any frame
class FailingStartApplication : public FakeApplication
{
public:
    FailingStartApplication(void)
        : FakeApplication(0.1F)
    {
    }

    bool stopped = false;

protected:
    void onStart(void) override
    {
        throw std::runtime_error("missing asset");
    }

    void onFixedUpdate(float /* step */) override
    {
    }

    void onStop(void) override
    {
        this->stopped = true;
    }
};

// Calls run() from inside its own loop
class ReentrantApplication : public FakeApplication
{
public:
    ReentrantApplication(void)
        : FakeApplication(0.1F)
    {
    }

    bool refused = false;

protected:
    void onStart(void) override
    {
        try {
            this->run();
        } catch (const std::logic_error &) {
            this->refused = true;
        }
        this->stop();
    }

    void onFixedUpdate(float /* step */) override
    {
    }
};

// Stops from a step, and leaves onStart, onInput and onStop to their default, empty, body
class StoppingApplication : public FakeApplication
{
public:
    StoppingApplication(void)
        : FakeApplication(0.1F)
    {
    }

    std::size_t updates = 0;
    std::size_t renders = 0;

protected:
    void onFixedUpdate(float /* step */) override
    {
        ++this->updates;
        if (this->updates == 3) {
            this->stop();
        }
    }

    void onRender(float /* alpha */) override
    {
        ++this->renders;
        FakeSource::advance(std::chrono::milliseconds(100));
    }
};

} // namespace

Test(core_application, is_not_running_before_run, .timeout = 5)
{
    const RecordingApplication app(0.1F, std::chrono::milliseconds(100), 0);

    cr_assert_not(app.isRunning());
}

Test(core_application, keeps_its_step, .timeout = 5)
{
    const RecordingApplication app(0.02F, std::chrono::milliseconds(20), 0);

    cr_assert_float_eq(app.getStep(), 0.02F, 0.00001F);
}

Test(core_application, invalid_step_throws, .timeout = 5)
{
    cr_assert_throw(RecordingApplication app(0.0F, std::chrono::milliseconds(0), 0), std::invalid_argument);
}

Test(core_application, hooks_run_in_the_order_of_a_frame, .timeout = 5)
{
    RecordingApplication app(0.1F, std::chrono::milliseconds(100), 2);

    app.run();
    const std::vector<std::string> expected{"start", "input", "update", "render", "input", "update", "render",
                                            "input", "stop"};
    cr_assert(app.calls == expected);
}

Test(core_application, is_not_running_after_run, .timeout = 5)
{
    RecordingApplication app(0.1F, std::chrono::milliseconds(100), 1);

    app.run();
    cr_assert_not(app.isRunning());
}

Test(core_application, each_step_receives_the_fixed_step, .timeout = 5)
{
    RecordingApplication app(0.1F, std::chrono::milliseconds(250), 1);

    app.run();
    cr_assert_eq(app.steps.size(), 2U);
    cr_assert_float_eq(app.steps[0], 0.1F, 0.00001F);
    cr_assert_float_eq(app.steps[1], 0.1F, 0.00001F);
}

Test(core_application, fast_frames_render_without_stepping, .timeout = 5)
{
    RecordingApplication app(0.1F, std::chrono::milliseconds(60), 2);

    app.run();
    // 60 ms then 120 ms in total: no step on the first frame, one on the second
    cr_assert_eq(app.steps.size(), 1U);
    cr_assert_eq(app.alphas.size(), 2U);
}

Test(core_application, render_receives_the_alpha, .timeout = 5)
{
    RecordingApplication app(0.1F, std::chrono::milliseconds(125), 1);

    app.run();
    cr_assert_eq(app.alphas.size(), 1U);
    cr_assert_float_eq(app.alphas[0], 0.25F, 0.001F);
}

Test(core_application, time_before_run_is_not_simulated, .timeout = 5)
{
    RecordingApplication app(0.1F, std::chrono::milliseconds(0), 1);

    // A long loading between the creation and run() must not run steps on the first frame
    FakeSource::advance(std::chrono::milliseconds(200));
    app.run();
    cr_assert(app.steps.empty());
}

Test(core_application, stop_during_input_skips_the_rest_of_the_frame, .timeout = 5)
{
    RecordingApplication app(0.1F, std::chrono::milliseconds(100), 0);

    app.run();
    const std::vector<std::string> expected{"start", "input", "stop"};
    cr_assert(app.calls == expected);
}

Test(core_application, stop_during_a_step_ends_the_loop_after_the_frame, .timeout = 5)
{
    StoppingApplication app;

    app.run();
    // The frame that stops is drawn, and no other one starts
    cr_assert_eq(app.updates, 3U);
    cr_assert_eq(app.renders, 4U);
    cr_assert_not(app.isRunning());
}

Test(core_application, can_run_again_after_stopping, .timeout = 5)
{
    RecordingApplication app(0.1F, std::chrono::milliseconds(100), 1);

    app.run();
    app.calls.clear();
    app.run();
    // The frame count was used up by the first run: the second one stops at its first input
    const std::vector<std::string> expected{"start", "input", "stop"};
    cr_assert(app.calls == expected);
}

Test(core_application, exception_still_calls_on_stop, .timeout = 5)
{
    FailingApplication app;

    cr_assert_throw(app.run(), std::runtime_error);
    cr_assert(app.stopped);
    cr_assert_not(app.isRunning());
}

Test(core_application, exception_in_on_start_still_calls_on_stop, .timeout = 5)
{
    FailingStartApplication app;

    cr_assert_throw(app.run(), std::runtime_error);
    cr_assert(app.stopped);
    cr_assert_not(app.isRunning());
}

Test(core_application, stop_before_run_ends_it_before_the_first_frame, .timeout = 5)
{
    RecordingApplication app(0.1F, std::chrono::milliseconds(100), 5);

    // e.g. Ctrl+C on a server while it starts: the request must not be lost
    app.stop();
    app.run();
    const std::vector<std::string> expected{"start", "stop"};
    cr_assert(app.calls == expected);
}

Test(core_application, stop_request_is_cleared_after_a_run, .timeout = 5)
{
    RecordingApplication app(0.1F, std::chrono::milliseconds(100), 1);

    app.stop();
    app.run();
    app.calls.clear();
    app.run();
    const std::vector<std::string> expected{"start", "input", "update", "render", "input", "stop"};
    cr_assert(app.calls == expected);
}

Test(core_application, stop_from_another_thread_ends_the_loop, .timeout = 5)
{
    class SpinningApplication : public younity::Application
    {
    public:
        SpinningApplication(void)
            : younity::Application(0.01F)
        {
        }

        std::atomic<bool> started{false};

    protected:
        void onStart(void) override
        {
            this->started = true;
        }

        void onFixedUpdate(float /* step */) override
        {
        }
    };

    SpinningApplication app;
    std::thread stopper([&app] {
        while (!app.started) {
            std::this_thread::yield();
        }
        app.stop();
    });

    app.run();
    stopper.join();
    cr_assert_not(app.isRunning());
}

Test(core_application, run_inside_run_throws, .timeout = 5)
{
    ReentrantApplication app;

    app.run();
    cr_assert(app.refused);
}

Test(core_application, real_clock_application_runs, .timeout = 5)
{
    // Stops at its first step: checks the steady clock version builds and loops
    class RealApplication : public younity::Application
    {
    public:
        RealApplication(void)
            : younity::Application(0.001F)
        {
        }

    protected:
        void onFixedUpdate(float /* step */) override
        {
            this->stop();
        }
    };

    RealApplication app;

    app.run();
    cr_assert_not(app.isRunning());
}
