"""MkDocs hook: writes the version of the engine in place of {{ younity_version }} in every page.

The version is read from the project() of CMakeLists.txt, the one the engine is built with,
so a release changes it in one place and the website follows.
"""

import re
from pathlib import Path

PLACEHOLDER = "{{ younity_version }}"


def on_config(config):
    """Read the version of the engine once, before the pages are built."""
    cmake = Path(config["config_file_path"]).parent / "CMakeLists.txt"
    match = re.search(r"project\(\s*Younity\s+VERSION\s+([0-9.]+)", cmake.read_text(encoding="utf-8"))

    if match is None:
        raise ValueError(f"{cmake}: the project() of the engine has no VERSION")
    config["extra"]["younity_version"] = match.group(1)
    return config


def on_page_markdown(markdown, page, config, files):
    """Write the version in place of the placeholder, code blocks included."""
    return markdown.replace(PLACEHOLDER, config["extra"]["younity_version"])
