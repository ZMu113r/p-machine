- have made changes to print Assembly and virtual machine
	- virtual mchine no longer reads input from file, it directly reads from code array
	- printAssembly prints the code array
- condition has global boolean 
- saved_index is used to save the instructionCount at jpc 
- inside the endsym recognition I have if condition then find offset using the current instructionCount
- we can find the jpc and overwrite the M value yay!

:)))))))))))))))))