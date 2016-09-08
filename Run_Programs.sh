#!/bin/bash

echo "Starting NCAA Basketball Script."

while true; do
	#GrabData
	./GrabData

	if [ $? -eq 0 ]
	
	then :

	else
		#Call program to email us something went wrong
		#python PythonErrorEmail.py
		#Halt Shell Script
		exit

	fi
	
	#TrimData
	./TrimData

	if [ $? -eq 0 ]

	then :

	else
		#Call program to email us something went wrong
		#python PythonErrorEmail.py
		#Halt Shell Script
		exit

	fi
		
	#TextPurifier
	./TextPurifier

	if [ $? -eq 0 ]
	
	then :

	else
		#Call program to email us something went wrong
		#python PythonErrorEmail.py
		#Halt Shell Script
		exit

	fi
		
	#MakeDataFile
	./MakeDataFile

	if [ $? -eq 0 ]

	then :

	else
		#Call program to email us something went wrong
		#python PythonErrorEmail.py
		#Halt Shell Script
		exit

	fi
		
	echo "Success! Now we wait."

	sleep 86400 #Defaults to seconds, can use 5m for 5 minutes, 5h for hours, 5d for days

done
	
	