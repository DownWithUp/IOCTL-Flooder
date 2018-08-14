# IOCTL-Flooder
IOCTL-Flooder is a verbose tool designed to help with Windows driver fuzzing by brute forcing IOCTLs on loaded drivers and uses GetLastError to determine if the IOCTL is valid. 
This program doesn't fuzz a driver, in the sense of trying random data types and addresses to find exploits, but just helps the analyst find valid IOCTLs. The code's comments may provide more information.
# Example Use
IOCTL-Flooder [Target Device]

Target Device: A valid device name (usually created through IoCreateSymbolicLink)
# Notes
Please feel free to modify this to fit your needs (Open Source for a reason!)<br/>
Also remember that not all drivers use IOCTLs

