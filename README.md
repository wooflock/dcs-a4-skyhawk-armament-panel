# dcs-a4-skyhawk-armament-panel
This is a start to create an armament panel for the A4 skyhawk in DCS.

Since i mostly fly in VR, i am not concerned about how things really look when it comes to
HOTAS and such. But i found that the armament panel in the A4 skyhawk was a little bit of a 
hassle with the hotas setup i had.
I started by adding the 5 pylon armament switches, since these where the ones i had some
issue finding a good way to manage with the buttons i already had on my stick.

![picture of the armament panel from the DCS community manual for A4 skyhawk.](https://github.com/wooflock/dcs-a4-skyhawk-armament-panel/armament.png)

After adding the 5 armament pylon switches i did find to my satisfaction that they really worked well
with DCS BIOS and an arduino. But as soon as i started playing multiplayer i found that as soon as i crashed
and the plane respawned, the state of the switches on my home made panel, and the virtual switches in the
VR cockpit where not in the same state.

This is due to the basic way DCS BIOS works when using the simple coding examples.
the arduino only send a pulse when you switch on, or off the switch. when you respawn in a new
plane, the arduino does not resend that switch.
When working with momentary switches, that more act as a push button, that does not matter much
but since i am using switches that interact with the switches in the vr cockpit i needed to 
keep these in sync.

So the soloution became a bit messier. Maybee there are much better ways to do this than the
way i ended up doing it. and maybee DCS BIOS already have a good arduino library that i am just
not aware of all the functions in, but this is how i did it.

I start with creating a state for the physical switches, and one for the state of the switches 
in DCS. if the switches ever end to not be in the same sate, i resend the state of the physical switch to DCS
wait 250 millis, and then check state again.
Much more code to get it running. But now DCS always follows the physical switch state.
If the switch is ON on my plaxiglass armamanetpanel, the switch will be turned on automatically if i crash
and respawn in a new plane.


