# Contributing

This page describes how the team worked on the engine during the project, and what the original GitLab pipeline checked. The GitLab configuration is not part of this repository.

## The workflow

Every change follows the same path, from an issue to `main`.

1. **Pick an issue.** Each task has its issue, with the module it belongs to, its type and its priority.
2. **Create a branch from `main`**, named after the issue: `feat/ecs-add-scene-management`.
3. **Commit** with a message of the form `type(module): what the commit does`.
4. **Open a merge request** whose description closes the issue: `Closes #19`. It stays a draft while the work is in progress.
5. **Get a review** from a teammate, then merge. The branch is deleted with the merge.

## Naming

| What            | Form                              | Example                                 |
|-----------------|-----------------------------------|-----------------------------------------|
| Issue           | `[MODULE] What it does`           | `[ECS] Add scene management`            |
| Branch          | `type/module-what-it-does`        | `feat/ecs-add-scene-management`         |
| Commit          | `type(module): what it does`      | `feat(ecs): add scene management`       |
| Merge request   | same as the commit                | `feat(ecs): add scene management`       |

The types are `feat` for a feature, `fix` for a bug, `docs` for the documentation, `test` for tests, `refactor` for a change that keeps the behavior, and `chore` for the tooling.

## What the original pipeline checked

The original GitLab pipeline ran these jobs on each merge request.

| Stage   | Job             | What it does                                         |
|---------|-----------------|------------------------------------------------------|
| check   | `format`        | The code follows `.clang-format`                     |
| check   | `tidy`          | The code passes the checks of `.clang-tidy`          |
| check   | `docs`          | This website builds, with every link valid           |
| build   | `build-linux`   | The engine builds on Linux                           |
| build   | `build-windows` | The engine builds on Windows                         |
| test    | `tests-linux`   | The unit tests pass, and the coverage is measured    |
| deploy  | `pages`         | This website was published, once the change was on `main` |

Run the same checks on your machine before pushing: see the [code style](code-style.md) and the [tests](../getting-started/tests.md).

## A change is ready when

- it builds and the tests pass;
- the new code has its unit tests in `tests/<module>/`;
- every public class and function has its Doxygen comment;
- the documentation is updated when the change adds or changes a feature.
