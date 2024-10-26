# This SCons scripting file adds environment variables for the WiFi connection as CPPDEFINES
Import("env")

import os
try:
    from dotenv import load_dotenv
except ImportError:
    env.Execute("$PYTHONEXE -m pip install python-dotenv")
    from dotenv import load_dotenv # attempt two

load_dotenv(".env")

env.Append(CPPDEFINES=[
        ("WIFI_SSID", env.StringifyMacro(os.getenv("WIFI_SSID"))),
        ("WIFI_PASS", env.StringifyMacro(os.getenv("WIFI_PASS")))])