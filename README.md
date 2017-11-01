# PSX-MultiMode3-PIC12F683

Port of the open-source MM3 code to XC8 and MPLAB X for the PIC 12F683.

Currently includes everything except the BIOS patches for the PSOne consoles.

Uses the same diagrams as MM3, as this is pin-compatible. Only connect Pins 1,4,5,6,7,8. Pins 2,3 are unused.

                      ________  ________
                      |       \/       |
           VDD (5V) --+ 1 >>      >> 8 +-- VSS (Ground)
                      |                |
           /        --+ 2         << 7 +-- signal from door (GPIO0)
                      |                |
           /        --+ 3         >> 6 +-- data stream (GPIO1)
                      |                |
         MCLR Reset --+ 4 >>      >> 5 +-- gate output (GPIO2)
                      |                |
                      +----------------+


For an Arduino (Atmel) version, refer to PsNee or PsNee V6 (OneChip)
PsNee:    https://pastebin.com/82h52q37
PsNee V6: https://pastebin.com/cWCsYugc

Note, the version of the Flash library linked in those is outdated and will not compile in recent Arduino builds.
For both, you will need this library: https://github.com/schinken/Flash/releases/tag/v1.0.1

For MM3 diagrams, refer to: http://www.eurasia.nu/modules.php?name=Downloads&d_op=viewdownloaddetails&lid=1644&ttitle=MM3%20diagrams