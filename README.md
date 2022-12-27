# Pig
A giant pig hangs overhead, with tracks leading into it from multiple directions. 
Balls can be fed into the tracks through various blowers, cranks, et cetera. 
As the balls fill the pig, progress will be displayed.
When enough balls have entered, a sound will play and the balls will be dumped.

I found a conflict that would make the RFM69 module hang while transmitting
if a pin interrupt is attached to A2. I don't know if the pinout confirms this.

## To Do
- Separate the counts from the different tracks and send separate numbers.
- Solder up audio wing and test with it attached.
- Decide on display tech and incorporate into SW and HW.
- Better EMI shielding on HW, see if that is cause of count creep on emptyPig().
