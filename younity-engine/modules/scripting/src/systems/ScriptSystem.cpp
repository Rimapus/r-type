/*
** EPITECH PROJECT, 2026
** younity-engine/modules/scripting/src/systems/ScriptSystem
** File description:
** ScriptSystem source file
*/

#include "younity/scripting/systems/ScriptSystem.hpp"
#include "../AudioLibrary.hpp"
#include "../EcsLibrary.hpp"
#include "../GameLibrary.hpp"
#include "../InputLibrary.hpp"
#include "../LogLibrary.hpp"
#include "../ModuleLoader.hpp"
#include "../StorageLibrary.hpp"
#include "younity/ecs/Registry.hpp"
#include "younity/physics/CollisionEvent.hpp"
#include "younity/scripting/Log.hpp"
#include "younity/scripting/components/Script.hpp"

#include <algorithm>
#include <cstdint>
#include <expected>
#include <filesystem>
#include <memory>
#include <optional>
#include <sol/sol.hpp>
#include <string>
#include <system_error>
#include <unordered_map>
#include <utility>
#include <vector>

namespace younity::scripting {

namespace {

/**
 * @brief The state of the script of one entity.
 */
struct Instance
{
    // The number written in the Script component when it started
    std::uint64_t number = 0;
    std::string path;
    // When the file was last written as the script started: a newer file starts it again
    std::filesystem::file_time_type version;
    // Empty when the script has no update function
    std::optional<sol::protected_function> update;
    // Empty when the script has no onCollision function
    std::optional<sol::protected_function> onCollision;
    bool failed = false;
    // The last step the entity had its script, to free the state of the ones that lost it
    std::uint64_t seenAt = 0;
    // The modules the script required as it started: saving one of them starts the script again
    std::vector<ModuleLoader::Required> modules;
};

/**
 * @brief Get the message of a failed load or call.
 *
 * @param result The failed result
 *
 * @return The message given by Lua, with the file and the line.
 */
template <typename Result>
std::string errorOf(const Result &result)
{
    const sol::error error = result;

    return error.what();
}

/**
 * @brief Get a function of the table of a script.
 *
 * @param script The table returned by the script
 * @param name The name of the function
 *
 * @return The function, an empty optional if the script has none, or the error if it is not a function.
 */
std::expected<std::optional<sol::protected_function>, std::string> getFunction(const sol::table &script,
                                                                               const char *name)
{
    // Raw: a failing __index metamethod would raise a Lua error outside any protected call
    const sol::object value = script.raw_get<sol::object>(name);

    if (value.get_type() == sol::type::lua_nil) {
        return std::nullopt;
    }
    if (value.get_type() != sol::type::function) {
        return std::unexpected(std::string("'") + name + "' must be a function");
    }
    return value.as<sol::protected_function>();
}

/**
 * @brief The functions of a script called by the system after its start.
 */
struct Functions
{
    std::optional<sol::protected_function> update;
    std::optional<sol::protected_function> onCollision;
};

/**
 * @brief Run a script file for an entity and call its start function.
 *
 * @param lua The interpreter
 * @param file The script file
 * @param entity The entity the script belongs to
 *
 * @return The functions of the script, or the reason it cannot run.
 */
std::expected<Functions, std::string> startScript(sol::state &lua, const std::filesystem::path &file, ecs::Entity entity)
{
    const sol::load_result chunk = lua.load_file(file.string());

    if (!chunk.valid()) {
        return std::unexpected(errorOf(chunk));
    }
    const sol::protected_function_result returned = chunk.get<sol::protected_function>()();

    if (!returned.valid()) {
        return std::unexpected(errorOf(returned));
    }
    if (returned.get_type() != sol::type::table) {
        return std::unexpected(std::string("the script must return a table"));
    }
    const sol::table script = returned;
    const auto start = getFunction(script, "start");
    const auto update = getFunction(script, "update");
    const auto onCollision = getFunction(script, "onCollision");

    for (const auto *function : {&start, &update, &onCollision}) {
        if (!*function) {
            return std::unexpected(function->error());
        }
    }
    if (*start) {
        const sol::protected_function_result started = (**start)(entity.getId());

        if (!started.valid()) {
            return std::unexpected(errorOf(started));
        }
    }
    return Functions{*update, *onCollision};
}

/**
 * @brief Gives the registry of a step to the ecs table, and takes it back at the end of the step,
 *        even when the step throws.
 */
class StepRegistry
{
public:
    StepRegistry(EcsContext &context, ecs::Registry &registry)
        : _context(context)
    {
        this->_context.registry = &registry;
        this->_context.destroyed.clear();
    }

    ~StepRegistry(void)
    {
        this->_context.registry = nullptr;
        this->_context.destroyed.clear();
    }

    StepRegistry(const StepRegistry &) = delete;
    StepRegistry &operator=(const StepRegistry &) = delete;
    StepRegistry(StepRegistry &&) = delete;
    StepRegistry &operator=(StepRegistry &&) = delete;

private:
    EcsContext &_context;
};

/**
 * @brief A collision received from the event bus, with the number of the script each entity had then.
 */
struct Received
{
    physics::CollisionEvent collision;
    std::uint64_t firstScript = 0;
    std::uint64_t secondScript = 0;
};

/**
 * @brief What the event bus writes to, shared so that a bus outliving the system does not write to it once destroyed.
 */
struct Inbox
{
    std::vector<Received> collisions;
    // The number of the script of each entity having one: a collision goes to the script that was running
    std::unordered_map<ecs::Entity::Id, std::uint64_t> scripts;
};

/**
 * @brief A collision of an entity, as given to its script.
 */
struct Contact
{
    ecs::Entity other;
    // The number of the script of the entity when the collision was published
    std::uint64_t script = 0;
};

} // namespace

struct ScriptSystem::Impl
{
    // Declared before the interpreter, so destroyed after it: the ecs table works on it
    EcsContext context;
    // Declared before the instances, so destroyed after them: the functions kept in the instances live inside it
    sol::state lua;
    // Declared after the interpreter, so destroyed before it: what the modules returned lives inside it
    ModuleLoader modules;
    std::filesystem::path root;
    std::unordered_map<ecs::Entity::Id, Instance> instances;
    std::shared_ptr<Inbox> inbox = std::make_shared<Inbox>();
    // The collisions of each entity given to the scripts during the current step
    std::unordered_map<ecs::Entity::Id, std::vector<Contact>> contacts;
    // When each file used in the current step was last written, read once per step
    std::unordered_map<std::string, std::filesystem::file_time_type> versions;
    std::uint64_t started = 0;
    std::uint64_t steps = 0;

    /**
     * @brief Get when the file of a script was last written, read once per step.
     *
     * @return The time, the minimum when the file cannot be read, e.g. while it does not exist.
     */
    std::filesystem::file_time_type versionOf(const std::string &path)
    {
        const auto found = this->versions.find(path);

        if (found != this->versions.end()) {
            return found->second;
        }
        std::error_code error;
        std::filesystem::file_time_type version = std::filesystem::last_write_time(this->root / path, error);

        if (error) {
            version = std::filesystem::file_time_type::min();
        }
        return this->versions.emplace(path, version).first->second;
    }

    /**
     * @brief Tell if a file was saved since a version of it.
     *        A missing file is not a change: an editor may delete it for a moment while it saves it.
     */
    bool wasSaved(const std::string &path, std::filesystem::file_time_type version)
    {
        const std::filesystem::file_time_type now = this->versionOf(path);

        return now != std::filesystem::file_time_type::min() && now != version;
    }

    /**
     * @brief Tell if the file of a script, or of a module it required as it started, was saved since it started.
     */
    bool wasSaved(const Instance &instance)
    {
        return this->wasSaved(instance.path, instance.version) ||
               std::ranges::any_of(instance.modules, [this](const ModuleLoader::Required &module) {
                   return this->wasSaved(module.path, module.version);
               });
    }

    /**
     * @brief Start the script of an entity, or log why it cannot run.
     */
    Instance &start(ecs::Entity entity, Script &script)
    {
        Instance &instance = this->instances[entity.getId()];

        instance = Instance{
            ++this->started,
            script.path,
            this->versionOf(script.path),
            std::nullopt,
            std::nullopt,
            false,
            this->steps,
            {},
        };
        script.instance = instance.number;
        this->inbox->scripts[entity.getId()] = instance.number;

        this->modules.record(&instance.modules);
        auto functions = startScript(this->lua, this->root / script.path, entity);

        this->modules.record(nullptr);

        if (functions) {
            instance.update = std::move(functions->update);
            instance.onCollision = std::move(functions->onCollision);
        } else {
            this->fail(instance, entity, functions.error());
        }
        return instance;
    }

    /**
     * @brief Sort the collisions received since the last step by entity, for collide().
     */
    void receiveCollisions(void)
    {
        this->contacts.clear();
        for (const Received &received : std::exchange(this->inbox->collisions, {})) {
            const physics::CollisionEvent &collision = received.collision;

            this->contacts[collision.first.getId()].push_back(Contact{collision.second, received.firstScript});
            this->contacts[collision.second.getId()].push_back(Contact{collision.first, received.secondScript});
        }
    }

    /**
     * @brief Tell if the script of an entity stops during this step: its entity was destroyed by a script,
     *        or lost its Script.
     */
    [[nodiscard]] bool isStopping(const ecs::Registry &registry, ecs::Entity entity) const
    {
        return this->context.destroyed.contains(entity.getId()) || !registry.hasComponent<Script>(entity);
    }

    /**
     * @brief Call the onCollision function of a script once for each collision its entity had while it ran.
     */
    void collide(const ecs::Registry &registry, ecs::Entity entity, Instance &instance) const
    {
        const auto found = this->contacts.find(entity.getId());

        if (found == this->contacts.end()) {
            return;
        }
        for (const Contact &contact : found->second) {
            if (!instance.onCollision || this->isStopping(registry, entity)) {
                return;
            }
            // Published before this script started: for another entity with the same id, or a previous script
            if (contact.script != instance.number) {
                continue;
            }
            const sol::protected_function_result result = (*instance.onCollision)(entity.getId(), contact.other.getId());

            if (!result.valid()) {
                fail(instance, entity, errorOf(result));
            }
        }
    }

    /**
     * @brief Stop the script of an entity and log the reason, once.
     */
    static void fail(Instance &instance, ecs::Entity entity, const std::string &reason)
    {
        instance.failed = true;
        instance.update.reset();
        instance.onCollision.reset();
        logger().error("script '{}' of entity {} stopped: {}", instance.path, entity.getId(), reason);
    }
};

ScriptSystem::ScriptSystem(const ecs::ComponentRegistry &components, EventBus &bus, const Actions &actions,
                           std::filesystem::path root)
    : ScriptSystem(components, bus, Host{actions}, std::move(root))
{
}

ScriptSystem::ScriptSystem(const ecs::ComponentRegistry &components, EventBus &bus, const Host &host,
                           std::filesystem::path root)
    : _impl(std::make_unique<Impl>(EcsContext{components, nullptr, {}}))
{
    this->_impl->root = std::move(root);
    this->_impl->lua.open_libraries(sol::lib::base, sol::lib::coroutine, sol::lib::math, sol::lib::string,
                                    sol::lib::table, sol::lib::utf8);
    // The base library reads files too: a script only runs its own code, and the modules of require
    this->_impl->lua["dofile"] = sol::lua_nil;
    this->_impl->lua["loadfile"] = sol::lua_nil;
    openEcsLibrary(this->_impl->lua, this->_impl->context);
    openInputLibrary(this->_impl->lua, host.actions);
    openAudioLibrary(this->_impl->lua, host.audio);
    openGameLibrary(this->_impl->lua, host.game);
    openStorageLibrary(this->_impl->lua, host.saves);
    openLogLibrary(this->_impl->lua);
    this->_impl->modules.open(this->_impl->lua, this->_impl->root);
    bus.subscribe<physics::CollisionEvent>([weakInbox = std::weak_ptr(this->_impl->inbox)](const physics::CollisionEvent &collision) {
        const std::shared_ptr<Inbox> inbox = weakInbox.lock();

        if (!inbox) {
            return;
        }
        const auto scriptOf = [&inbox](ecs::Entity entity) {
            const auto found = inbox->scripts.find(entity.getId());

            return found != inbox->scripts.end() ? found->second : 0;
        };

        inbox->collisions.push_back(Received{collision, scriptOf(collision.first), scriptOf(collision.second)});
    });
}

ScriptSystem::~ScriptSystem(void) = default;

void ScriptSystem::update(ecs::Registry &registry, const std::vector<ecs::Entity> &entities, float dt)
{
    Impl &impl = *this->_impl;

    const StepRegistry stepRegistry(impl.context, registry);

    ++impl.steps;
    // The collisions published since the last step: the ones published during this step go to the next one
    impl.receiveCollisions();
    impl.versions.clear();
    for (const ecs::Entity entity : entities) {
        // A script earlier in the step may have removed this Script
        if (!registry.hasComponent<Script>(entity)) {
            continue;
        }
        Script &script = registry.getComponent<Script>(entity);
        const auto found = impl.instances.find(entity.getId());
        // A different number is a new entity with the id of a destroyed one, a newer file was saved since the start
        const bool isNew = found == impl.instances.end() || found->second.number != script.instance ||
                           found->second.path != script.path || impl.wasSaved(found->second);
        Instance &instance = isNew ? impl.start(entity, script) : found->second;

        instance.seenAt = impl.steps;
        impl.collide(registry, entity, instance);
        if (!instance.update || impl.isStopping(registry, entity)) {
            continue;
        }
        const sol::protected_function_result result = (*instance.update)(entity.getId(), dt);

        if (!result.valid()) {
            Impl::fail(instance, entity, errorOf(result));
        }
    }
    std::erase_if(impl.instances, [&impl](const auto &entry) {
        if (entry.second.seenAt == impl.steps) {
            return false;
        }
        impl.inbox->scripts.erase(entry.first);
        return true;
    });
}

bool ScriptSystem::isRunning(ecs::Entity entity) const
{
    const auto found = this->_impl->instances.find(entity.getId());

    return found != this->_impl->instances.end() && !found->second.failed;
}

} // namespace younity::scripting
