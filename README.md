# periodic_signal

## usage
simply create a period signal with the desired frequency and embed it into another while loop, but there are some considerations
- when the process function is called it check to see if the time since the last on signal was greater or equal to the period time, if the time since the last one is strictly greater than the period time, then that means that the signal will be running slow, the greater the rate of the outer while loop the more accuracy it will have.
- if the outer while loop runs a frequency which is slower than this signals period, then based on the previous bullet point it will always return true, so just don't do that
- read more details [here](https://toolbox.cuppajoeman.com/programming/looping_in_time.html)
