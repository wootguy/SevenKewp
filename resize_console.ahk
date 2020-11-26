#SingleInstance Force

; Exit after a minute of inactivity, in case a debugging session wasn't actually started
Sleep, 60*1000
ExitApp

; resize the console window on the first invocation
$`::
Send ``
Sleep, 10
MouseClickDrag, Left, 92, 95, 2000, 95, 0
MouseClickDrag, Left, 1960, 474, 1346, 1400, 0
ExitApp 


