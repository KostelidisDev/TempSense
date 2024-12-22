import os.path
import shutil

# noinspection PyUnresolvedReferences
Import("env")

# noinspection PyUnresolvedReferences
if env.IsIntegrationDump():
    # noinspection PyUnresolvedReferences
    Return()

if not os.path.isfile('.env'):
    shutil.copy('.env.example', '.env')

with open('.env', mode='r') as env_file:
    for line in env_file:
        line = line.strip()
        line_segments = line.split('=')
        key = line_segments[0]
        value = line_segments[1]
        if f"{key}" in ["MONITOR_BAUD", "HTTP_PORT"]:
            # noinspection PyUnresolvedReferences
            env.Append(CPPDEFINES=[(
                f"{key}", f"{value}"
            )])
            continue
        # noinspection PyUnresolvedReferences
        env.Append(CPPDEFINES=[(
            f"{key}", f"\"{env.StringifyMacro(value)}\""
        )])
