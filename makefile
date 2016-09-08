all:
	g++ GrabData.cpp -g -O3 -std=c++11 -Wall -o GrabData -lcurl
	g++ TrimData.cpp -g -O3 -std=c++11 -Wall -o TrimData
	g++ TextPurifier.cpp -g -O3 -Wall -o TextPurifier
	g++ MakeDataFile.cpp -g -O3 -std=c++11 -Wall -o MakeDataFile

clean:
	rm GrabData
	rm TrimData
	rm TextPurifier
	rm MakeDataFile
