# IOCTL-Flooder
IOCTL-Flooder is a verbose tool designed to help with Windows driver fuzzing by brute forcing IOCTLs on loaded drivers and uses GetLastError to determine if the IOCTL is valid. 
This program doesn't fuzz a driver, in the sense of trying random data types and addresses to find exploits, but just helps the analyst find valid IOCTLs. 
# Example Use
IOCTL-Flooder [Target Device]

Target Device: A valid device name (usually created through IoCreateSymbolicLink)
