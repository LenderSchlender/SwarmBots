Code for the ESP-CAM modules used for remotely monitoring the robots

Before building, copy the `example.env` file to `.env` and fill in the variables. \
During the build process you will be asked for a camera id. You can either provide one or leave it blank and just press Enter, in which case the last 3 octets of the device's MAC address will be used as the id. \
All devices running this code will have a hostname of `sbcam-{id}`, for example `sbcam-0`. \
**This code can only handle one connection at once!**