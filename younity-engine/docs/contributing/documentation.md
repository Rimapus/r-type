# Writing the documentation

This website is built with [MkDocs](https://www.mkdocs.org) and the [Material](https://squidfunk.github.io/mkdocs-material/) theme, from the Markdown files of the `docs/` folder. The API reference is generated from the Doxygen comments of the code.

## See the website on your machine

Install [Doxygen](https://www.doxygen.nl), then the Python packages:

```sh
python3 -m venv .venv
source .venv/bin/activate
pip install -r docs/requirements.txt
```

Start the website:

```sh
mkdocs serve
```

The command prints the local address of the website, `http://127.0.0.1:8000/chrystoval_group/younity-engine/`, and the pages reload each time one is saved.

## Add a page

1. Create a Markdown file in the folder of its section, for example `docs/guides/my-guide.md`.
2. Add it to the `nav` list of `mkdocs.yml`.
3. Check the whole website the way the pipeline does:

```sh
mkdocs build --strict
```

This command stops on a broken link or on a page left out of the navigation.

## The sections

| Folder                  | Content                                                        |
|-------------------------|----------------------------------------------------------------|
| `docs/getting-started/` | Build the engine, run the tests, use it in a game              |
| `docs/architecture/`    | The modules and the main systems                               |
| `docs/guides/`          | Tutorials and how-to                                           |
| `docs/study/`           | The technical and comparative study                            |
| `docs/contributing/`    | The workflow, the code style and this page                     |

The API reference is produced from the headers of `modules/` at each build, so `docs/` holds the written pages only.

## Writing rules

- The documentation is in English.
- A page starts with what it is about, and stays short.
- A diagram is written in [Mermaid](https://mermaid.js.org), followed by a sentence that describes it.
- An image has an alternative text.
- A command shown in a page has been run.
- The pages explain the engine at the level of its modules and systems. The detail of a class is the job of its Doxygen comment.
- The version of the engine is written as `younity_version` between double braces: the hook `docs/hooks/version.py` replaces it with the `VERSION` of `CMakeLists.txt` when the website is built, so a release changes no page.
