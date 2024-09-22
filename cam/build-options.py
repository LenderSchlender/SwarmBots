# This is a SCons scripting file
Import("env")

import os
try:
    from dotenv import load_dotenv
except ImportError:
    env.Execute("$PYTHONEXE -m pip install python-dotenv")
    from dotenv import load_dotenv # attempt two

load_dotenv()


# Do not run a script when external applications, such as IDEs,
# dump integration data. Otherwise, input() will block the process
# waiting for the user input
if env.IsIntegrationDump():
    # stop the current script execution
    Return()

env.Append(CPPDEFINES=[
        ("WIFI_SSID", env.StringifyMacro(os.getenv("WIFI_SSID"))),
        ("WIFI_PASS", env.StringifyMacro(os.getenv("WIFI_PASS")))])

# Ask for camera id
print("Enter camera ID:")
cam_id = input()
# Only define it if provided
if cam_id and cam_id.strip():
    env.Append(CPPDEFINES=[
        ("CAM_ID", env.StringifyMacro(cam_id))])