/*
** EPITECH PROJECT, 2026
** younity-engine/modules/core/include/younity/core/Application
** File description:
** Application header file
*/

#pragma once

#include <atomic>
#include <chrono>
#include <cstddef>
#include <stdexcept>

#include "younity/core/Clock.hpp"
#include "younity/core/FixedTimestep.hpp"
#include "younity/core/Log.hpp"

namespace younity {

/**
 * @brief The main loop of a game: the game derives from it, fills the hooks and calls run().
 *
 *        class MyGame : public younity::Application
 *        {
 *        public:
 *            MyGame(void) : younity::Application(1.0F / 60.0F) {}
 *
 *        protected:
 *            void onInput(void) override;               // window events, inputs, network
 *            void onFixedUpdate(float step) override;   // the scenes, at a fixed rate
 *            void onRender(float alpha) override;       // drawing
 *        };
 *
 * Each frame calls onInput(), then onFixedUpdate() once per fixed step that fits in the time
 * of the frame (possibly 0), then onRender(). The loop ends once stop() is called.
 * It runs as fast as it can: on a client, the vertical sync of the window paces it.
 *
 * @tparam Source The source of time, a fake one can be given to the tests
 */
template <TimeSource Source>
class BasicApplication
{
public:
    /**
     * @brief Create an application that is not running yet.
     *
     * @param step The duration of one step of the game logic, in seconds, e.g. 1.0F / 60.0F
     * @param maxFrameTime The longest frame time simulated, in seconds, see FixedTimestep
     *
     * @throw std::invalid_argument if the step is not a positive number, or if maxFrameTime is shorter than it.
     */
    explicit BasicApplication(float step, float maxFrameTime = FixedTimestep::DEFAULT_MAX_FRAME_TIME)
        : _timestep(step, maxFrameTime)
    {
    }

    virtual ~BasicApplication(void) = default;

    BasicApplication(const BasicApplication &) = delete;
    BasicApplication &operator=(const BasicApplication &) = delete;
    BasicApplication(BasicApplication &&) = delete;
    BasicApplication &operator=(BasicApplication &&) = delete;

    /**
     * @brief Run the main loop until stop() is called: onStart(), the frames, then onStop().
     *        onStop() is called even if a hook throws, onStart() included, then the exception goes on
     *        to the caller. Once it returns, run() can be called again.
     *        A stop() asked before run() is kept: this run ends before its first frame.
     *
     * @throw std::logic_error if the application is already running, e.g. run() called from a hook.
     */
    void run(void)
    {
        if (this->_running.exchange(true)) {
            throw std::logic_error("younity::Application: run() called while the application is already running");
        }
        core::logger().info("Application started, one step every {}s", this->_timestep.getStep());
        try {
            this->onStart();
            this->loop();
        } catch (...) {
            this->finish();
            throw;
        }
        this->finish();
    }

    /**
     * @brief Ask the loop to end. The current frame ends first, unless it is called from onInput(),
     *        in which case the steps and the drawing of this frame are skipped.
     *        Can be called from another thread, e.g. to stop a server.
     */
    void stop(void) noexcept
    {
        this->_stopRequested = true;
    }

    /**
     * @brief Check if the main loop is running.
     *
     * @return true from the beginning of run() until onStop() is called.
     */
    [[nodiscard]] bool isRunning(void) const noexcept
    {
        return this->_running;
    }

    /**
     * @brief Get the duration of one step, the delta time given to onFixedUpdate().
     *
     * @return The step given at the creation, in seconds.
     */
    [[nodiscard]] float getStep(void) const noexcept
    {
        return this->_timestep.getStep();
    }

protected:
    /**
     * @brief Called once at the beginning of run(), before the first frame, e.g. to open the first scene.
     */
    virtual void onStart(void)
    {
    }

    /**
     * @brief Called first in each frame, to read what happened since the last one:
     *        window events, inputs, received packets. Calling stop() here ends the loop at once.
     */
    virtual void onInput(void)
    {
    }

    /**
     * @brief Called once per fixed step, to move the game forward, e.g. scenes.update(step).
     *
     * @param step The duration of the step, in seconds, the same at every call
     */
    virtual void onFixedUpdate(float step) = 0;

    /**
     * @brief Called last in each frame, to draw it.
     *
     * @param alpha How far the frame is between the last step and the next one, between 0 and 1,
     *              to draw the entities between their last two states
     */
    virtual void onRender(float /* alpha */)
    {
    }

    /**
     * @brief Called once at the end of run(), after the last frame or after a hook threw.
     */
    virtual void onStop(void)
    {
    }

private:
    /**
     * @brief Run the frames until stop() is called.
     */
    void loop(void)
    {
        // The time spent before run(), e.g. loading in onStart(), is not simulated
        this->_timestep.reset();
        (void)this->_clock.restart();
        while (!this->_stopRequested) {
            this->onInput();
            if (this->_stopRequested) {
                break;
            }
            for (std::size_t steps = this->_timestep.advance(this->_clock.restart()); steps > 0; --steps) {
                this->onFixedUpdate(this->_timestep.getStep());
            }
            this->onRender(this->_timestep.getAlpha());
        }
    }

    /**
     * @brief End a run, whether it stopped or a hook threw: the application is not running anymore
     *        when onStop() is called, so a throwing onStop() still leaves it ready for the next run.
     *        The stop request is cleared here and not at the start of run(): a stop() sent by another
     *        thread while run() starts, e.g. on Ctrl+C, is never lost.
     */
    void finish(void)
    {
        this->_stopRequested = false;
        this->_running = false;
        core::logger().info("Application stopped");
        this->onStop();
    }

    BasicClock<Source> _clock;
    FixedTimestep _timestep;
    // Atomic: stop() and isRunning() may be called from another thread
    std::atomic<bool> _running{false};
    std::atomic<bool> _stopRequested{false};
};

/**
 * @brief The main loop of the engine, timed by the monotonic clock of the system.
 */
using Application = BasicApplication<std::chrono::steady_clock>;

} // namespace younity
