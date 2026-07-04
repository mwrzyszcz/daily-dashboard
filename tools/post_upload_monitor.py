Import("env")

import subprocess
import sys


def start_monitor(*_args, **_kwargs):
    project_dir = env.subst("$PROJECT_DIR")
    pioenv = env.subst("$PIOENV")
    command = [
        sys.executable,
        "-m",
        "platformio",
        "device",
        "monitor",
        "--project-dir",
        project_dir,
        "--environment",
        pioenv,
    ]

    subprocess.Popen(command, cwd=project_dir)


env.AddPostAction("upload", start_monitor)