#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <string>
#include <ctime>
#include <algorithm>
#include <regex>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <limits.h>
#include <dirent.h>

using namespace std;

string data = "";
vector<string> listOfTeamData;
vector<string> listOfStartTable;
vector<string> listOfBenchTable;
vector<string> listOfFastBreakPoints;
vector<string> listOfPointsInThePaint;
vector<string> listOfTeamTurnoversPoints;
vector<string> listOfPointsOffTurnovers;

string stringOfHomeTeam = "";
string stringOfHomeScore = "";
string stringOfAwayTeam = "";
string stringOfAwayScore = "";
string stringOfGameTime = "";
string stringOfEventLocation = "";
string stringOfOfficials = "";
string stringOfAttendance = "";

string dataLine = "";
vector<string> dataLineList;

string teamNameScoreExpression = "espn.go.com/nba/team/.*?\">(.*?)</a> <span>(.*?)</span>";
string gameTimeExpression = "game-time-location\"><p>(.*?)<";
string eventLocationExpression = "game-time-location\"><p>.*?</p><p>(.*?)</p></div>";
string fastBreakExpression = "Fast break points:</strong>&nbsp;&nbsp;&nbsp;(.*?)<br />";
string pointsInThePaintExpression = "Points in the paint:</strong>&nbsp;&nbsp;&nbsp;(.*?)<br />";
string teamTurnoversExpression = "Total Team Turnovers \\(Points off turnovers\\):</strong>&nbsp;&nbsp;&nbsp;(.*?) \\(";
string pointsOffTurnoversExpression =  "Total Team Turnovers \\(Points off turnovers\\):</strong>&nbsp;&nbsp;&nbsp;.*? \\((.*?)\\)</div><div";
string officialsExpression = "Officials:</strong> (.*?)<br>";
string attendanceExpression = "Attendance:</strong> (.*?)<br";
string teamDataExpression = "STARTERS([\\s\\S]*?TOTALS)";

string startersExpression = "</thead><tbody><tr align=right valign=middle class=\"odd ([\\s\\S]*?)>BENCH</th>";
string benchExpression = "</thead><tbody><tr align=right valign=middle class=\"even ([\\s\\S]*?)>TOTALS";

string playerStatsExpression = "href=\".*?\">(.*?)</a>, (.*?)</td><td>(.*?)</td><td>(.*?)-(.*?)</td><td>(.*?)-(.*?)</td><td>(.*?)-(.*?)</td><td.*?>(.*?)</td><td.*?>(.*?)</td><td>(.*?)</td><td>(.*?)</td><td>(.*?)</td><td>(.*?)</td><td>(.*?)</td><td>(.*?)</td><td>(.*?)</td><td>(.*?)<";
string playerDNPexpression = "href=\".*?\">(.*?)</a>, (.*?)</td><td.*?DNP.*?</td></tr>";

string errorFileUniqueString = ".line-score-container";

string get_file_contents(const char *filename);
bool fileExists(string name);

void listDir(vector<string> &filesInDirectory);
void captureHomeAndAwayTeam();
void oneCapture(string &captureString, string &expression);
void captureNLength(vector<string> &capList, string &expression, string &searchString);
void getPlayerStats(string &expression, string searchString, bool starter, bool home);
void getPlayerStatsDNP(string &expression, string searchString, bool starter, bool home);
void JrSrFormat(string &dataLine);
void emptyAllVectors();

int main()
{
	vector<string> filesInAsciiGameDirectory;
	
	int chdirRet;
	int amountFound;
	int mkdirRet;
	//int renameRet;
	
	//get cwd 
	char* cwd;
	char buffer[PATH_MAX];
	cwd = getcwd(buffer, PATH_MAX);
	string currentDirectory(cwd);
	
	//create directory strings
	string masterDirectory = currentDirectory + "/ESPN_HTML_Cache";
	string AsciiGameDirectory = masterDirectory + "/Ascii_Cut_Game_Files";
	string dataExtractedDirectory = masterDirectory + "/Data_Extracted_Files";
	
	//make directory strings into const char*
	const char* cMasterDirectory = masterDirectory.c_str();
	const char* cAsciiGameDirectory = AsciiGameDirectory.c_str();
	const char* cDataExtractedDirectory = dataExtractedDirectory.c_str();
	
	mkdirRet = mkdir(cDataExtractedDirectory, S_IRWXU | S_IRWXG | S_IRWXO);
	if (mkdirRet == -1)
		cout << "\"" << cDataExtractedDirectory << "\" already exists!" << endl;
	
	//change to master directory in order to see if file data file is empty
	chdirRet = chdir(cMasterDirectory);
	if (chdirRet == -1)
	{
		cout << "Could not change directory! Quitting." << endl;
		return 1;
	}
	
	//if file does not exist we need the header for the file
	bool doesDataFileExist = fileExists("completeDataFile.txt");
	if (!doesDataFileExist)
	{
		dataLineList.push_back("Player,Position,Start,Minutes,FGM,FGA,3PM,3PA,FTM,FTA,OREB,DREB,REB,AST,STL,BLK,TO,PF,Plus/Minus,PT,Home/Away,Team,Opponent,Win/Loss,FastBreakPoints,PointsInThePaint,TotalTeamTurnovers,PointsOffTurnovers,Time,Date,Year,Stadium,City,State/Province,Off1,Off2,Off3,Off4,Attendance");
	}
	
	//change the current directory to ascii cut strings directory
	chdirRet = chdir(cAsciiGameDirectory);
	if (chdirRet == -1)
	{
		cout << "Could not change directory! Quitting." << endl;
		return 1;
	}
	
	listDir(filesInAsciiGameDirectory);
	
	sort(filesInAsciiGameDirectory.begin(), filesInAsciiGameDirectory.end());
	
	for(size_t i = 2; i < filesInAsciiGameDirectory.size(); i++)
	{
		bool insufficientData = false;
		size_t found = 0;
		
		//clear data and get new contents
		data = "";
		data = get_file_contents(filesInAsciiGameDirectory[i].c_str());
		
		//cout << filesInAsciiGameDirectory[i] << endl;
		
		//commence regular expressions
		captureHomeAndAwayTeam();
		oneCapture(stringOfGameTime, gameTimeExpression);
		
		//fix format for date then append
		while(1)
		{
			found = stringOfGameTime.find(", ");
			if (found != std::string::npos)
			{
				stringOfGameTime.erase(found + 1, 1);
			}
			
			else
			{
				break;
			}
		}
		
		oneCapture(stringOfEventLocation, eventLocationExpression);
		
		//fix format for event location then append
		while(1)
		{
			found = stringOfEventLocation.find(", ");
			if (found != std::string::npos)
			{
				stringOfEventLocation.erase(found + 1, 1);
			}
			
			else
			{
				break;
			}
		}
		
		oneCapture(stringOfAttendance, attendanceExpression);
		//cout << stringOfAttendance << endl;
		
		if(stringOfAttendance == "N/A")
		{
			stringOfAttendance = "";
		}
		
		//get rid of commas in attendance
		while(1)
		{
			found = stringOfAttendance.find(",");
			if (found != std::string::npos)
			{
				stringOfAttendance.erase(found, 1);
			}
			
			else
			{
				break;
			}
		}
		
		oneCapture(stringOfOfficials, officialsExpression);
		//cout << stringOfOfficials << endl;
		
		//format Jr and Sr in officials
		JrSrFormat(stringOfOfficials);
		
		//make offcials list have no whitespace near commas
		while(1)
		{
			found = stringOfOfficials.find(", ");
			if (found != std::string::npos)
			{
				stringOfOfficials.erase(found + 1, 1);
			}
			
			else
			{
				break;
			}
		}
		
		//determine amount of commas in stringOfOfficials
		amountFound = count(stringOfOfficials.begin(), stringOfOfficials.end(), ',');
		
		//append commas to the end until there are 4 total commas
		char lastChar = stringOfOfficials.back();
		
		//there is a special case where there is trailing whitespace
		if(lastChar == ' ')
		{
			stringOfOfficials = stringOfOfficials.substr(0, stringOfOfficials.length() - 1);
		}
		
		//append commas until the correct amount of columns are made
		for(; amountFound < 4; amountFound++)
		{
			stringOfOfficials = stringOfOfficials + ",";
		}
		
		found = data.find("Top Performers");
		if (found == std::string::npos)
		{
			found = data.find("<!-- end line score -->");
			if(found != std::string::npos)
			{
				insufficientData = true;
			}
		}
		
		if (insufficientData == false)
		{
			captureNLength(listOfTeamData, teamDataExpression, data);
			captureNLength(listOfFastBreakPoints, fastBreakExpression, data);
			captureNLength(listOfPointsInThePaint, pointsInThePaintExpression, data);
			captureNLength(listOfTeamTurnoversPoints, teamTurnoversExpression, data);
			captureNLength(listOfPointsOffTurnovers, pointsOffTurnoversExpression, data);
		}
		
		//if teamData returns an empty string we have extra tests
		if(listOfTeamData.size() < 2)
		{
			//cout << "That file contains insufficient data! Skipping" << endl;
			insufficientData = true;
		}
		
		if(insufficientData == true)
		{
			//run second test to see if file is truly garbage
			found = data.find(errorFileUniqueString);
			if(found == string::npos)
			{
				//in this unique situation we need special treatment
				//captureNLength(listOfStartTable, typeZeroStartersExpression, data);
				//captureNLength(listOfBenchTable, typeZeroBenchExpression, data);
				cout << filesInAsciiGameDirectory[i] << endl;
				//getPlayerStatsTypeZero(typeOneTeams, listOfStartTable[0], true, false);
				//getPlayerStatsTypeZero(typeOneTeams, listOfBenchTable[0], false, false);
				//getPlayerStatsTypeZero(typeOneTeams, listOfStartTable[1], true, true);
				//getPlayerStatsTypeZero(typeOneTeams, listOfBenchTable[1], false, true);
			
			}
			//if data truly is not there we need to skip this file
		}
		
		//if sufficient data is found we run further tests
		else
		{
			captureNLength(listOfStartTable, startersExpression, data);
			captureNLength(listOfBenchTable, benchExpression, data);
					
			getPlayerStats(playerStatsExpression, listOfStartTable[0], true, false);
			getPlayerStats(playerStatsExpression, listOfBenchTable[0], false, false);
			getPlayerStatsDNP(playerDNPexpression, listOfBenchTable[0], false, false);
			
			getPlayerStats(playerStatsExpression, listOfStartTable[1], true, true);
			getPlayerStats(playerStatsExpression, listOfBenchTable[1], false, true);
			getPlayerStatsDNP(playerDNPexpression, listOfBenchTable[1], false, true);
		}
		
		//append found data from loop into data file
		fstream fileInput;
		fileInput.open (masterDirectory + "/" + "completeDataFile.txt", std::fstream::in | std::fstream::out | std::fstream::app);
		for(size_t k = 0; k < dataLineList.size(); k++)
		{
			fileInput << dataLineList[k] << endl;
		}
		
		fileInput.close();
		
		emptyAllVectors();
		
		//create strings for old and new file location
		//string oldFileName = AsciiGameDirectory + "/" + filesInAsciiGameDirectory[i];
		//string newFileName = dataExtractedDirectory + "/" + filesInAsciiGameDirectory[i];
		
		//const char* cOldFileName = oldFileName.c_str();
		//const char* cNewFileName = newFileName.c_str();
		
		//attempt to move file into another directory
		//renameRet = rename(cOldFileName, cNewFileName);
		//if (renameRet == -1)
		//{
		//	cout << filesInAsciiGameDirectory[i] << "could not be renamed!" << endl;
		//	cout << "Exiting Program" << endl;
		//	return 1;
		//}
	}
	
	return 0;
}

string get_file_contents(const char *filename)
{
	ifstream in(filename, ios::in);
	if (in)
	{
		string contents;
		in.seekg(0, ios::end);
		contents.resize(in.tellg());
		in.seekg(0, ios::beg);
		in.read(&contents[0], contents.size());
		in.close();
		return contents;
	}
	throw(errno);
}

bool fileExists(string name)
{
	struct stat buffer;   
	return (stat (name.c_str(), &buffer) == 0); 
}

void listDir(vector<string> &filesInDirectory)
{
	filesInDirectory.clear();
	
	DIR		*d;
	struct dirent *dir;
	d = opendir(".");
	if (d)
	{
		while ((dir = readdir(d)) != NULL)
		{
			filesInDirectory.push_back(dir->d_name);
		}
		
		closedir(d);
	}
}

void captureHomeAndAwayTeam()
{
	//init regular expression
	const regex teamsAndScore(teamNameScoreExpression);
	smatch matches;
	int pos = 0;
	int awayOrHome = 0;
	string tempDataString = data;
	
	try
	{
		while(1)
		{
			tempDataString = data.substr(pos);
			
			//start regular expression iterators
			if(regex_search(tempDataString, matches, teamsAndScore))
			{
				if(awayOrHome % 2 == 0)
				{
					stringOfAwayTeam = matches[1];
					stringOfAwayScore = matches[2];
				}
				
				else
				{
					stringOfHomeTeam = matches[1];
					stringOfHomeScore = matches[2];
				}
				
				pos = pos + matches.position(2) + matches.length(2);
				awayOrHome += 1;
			}
			
			else
			{
				break;
			}
		}
	}
	
	//if an error occurs exit program
	catch(regex_error& e)
	{
		cout << "A regex error has occured" << endl;
		exit (1);
	}
}

void oneCapture(string &captureString, string &expression)
{
	bool found = false;
	smatch matches;
	
	try
	{
		//init regular expression
		const regex regularExp(expression);
		
		//find if regular expression exists
		found = regex_search(data, matches, regularExp);
		
		//if we find a match push back data
		if(found)
		{
			captureString = matches[1];
		}
		
		//if no match push back empty string
		else
		{
			captureString = "";
		}
	}
	
	//if an error occurs exit program
	catch(regex_error& e)
	{
		cout << "A regex error has occured" << endl;
		exit (1);
	}
}

void captureNLength(vector<string> &capList, string &expression, string &searchString)
{
	const regex NCapRegex(expression);
	smatch matches;
	int pos = 0;
	string tempDataString = searchString;
	
	try
	{
		while(1)
		{
			tempDataString = searchString.substr(pos);
			
			//start regular expression iterators
			if(regex_search(tempDataString, matches, NCapRegex))
			{
				capList.push_back(matches[1]);
				pos = pos + matches.position(1);
			}
			
			else
			{
				break;
			}
		}
	}
	
	//if an error occurs exit program
	catch(regex_error& e)
	{
		cout << "A regex error has occured" << endl;
		exit (1);
	}
}

void getPlayerStats(string &expression, string searchString, bool starter, bool home)
{
	const regex NCapRegex(expression);
	smatch matches;
	int pos = 0;
	
	try
	{
		while(1)
		{
			dataLine = "";
			searchString = searchString.substr(pos);
			
			//start regular expression iterators
			if(regex_search(searchString, matches, NCapRegex))
			{
				dataLine = dataLine + matches[1].str() + ",";
				
				//make position NA into a blank string
				if(matches[2] == "NA")
				{
					dataLine = dataLine + ",";
				}
				
				else
				{
					dataLine = dataLine + matches[2].str() + ",";
				}
				
				//determine whether starter or not
				if(starter)
				{
					dataLine = dataLine + "Starter,";
				}
				
				else
				{
					dataLine = dataLine + "Bench,";
				}
				
				//append data in formated string
				for(int i = 3; i < 20; i++)
				{
					dataLine = dataLine + matches[i].str() + ",";
				}
				
				//determine whether home or not
				if(home)
				{
					//assign stats found if Home
					dataLine = dataLine + "Home,";
					dataLine = dataLine + stringOfHomeTeam + ",";
					dataLine = dataLine + stringOfAwayTeam + ",";
					
					//determine winner and append data
					if (stoi(stringOfHomeScore) > stoi(stringOfAwayScore))
					{
						dataLine = dataLine + "Won,";
					}
					
					else
					{
						dataLine = dataLine + "Loss,";
					}
					
					dataLine = dataLine + listOfFastBreakPoints[1] + ",";
					dataLine = dataLine + listOfPointsInThePaint[1] + ",";
					dataLine = dataLine + listOfTeamTurnoversPoints[1] + ",";
					dataLine = dataLine + listOfPointsOffTurnovers[1] + ",";
				}
				
				else
				{
					dataLine = dataLine + "Away,";
					dataLine = dataLine + stringOfAwayTeam + ",";
					dataLine = dataLine + stringOfHomeTeam + ",";
					
					if (stoi(stringOfAwayScore) > stoi(stringOfHomeScore))
					{
						dataLine = dataLine + "Won,";
					}
					
					else
					{
						dataLine = dataLine + "Loss,";
					}
					
					dataLine = dataLine + listOfFastBreakPoints[0] + ",";
					dataLine = dataLine + listOfPointsInThePaint[0] + ",";
					dataLine = dataLine + listOfTeamTurnoversPoints[0] + ",";
					dataLine = dataLine + listOfPointsOffTurnovers[0] + ",";
				}
				
				dataLine = dataLine + stringOfGameTime + ",";
				
				dataLine = dataLine + stringOfEventLocation + ",";
				
				dataLine = dataLine + stringOfOfficials;
				
				dataLine = dataLine + stringOfAttendance;
				
				pos = matches.position(18);
				
				//fix the format of Jr and Sr
				JrSrFormat(dataLine);
				
				dataLineList.push_back(dataLine);
			}
			
			else
			{
				break;
			}
		}
	}
	
	//if an error occurs exit program
	catch(regex_error& e)
	{
		cout << "A regex error has occured" << endl;
		exit (1);
	}
}

void getPlayerStatsDNP(string &expression, string searchString, bool starter, bool home)
{
	const regex NCapRegex(expression);
	smatch matches;
	int pos = 0;
	
	try
	{
		while(1)
		{
			dataLine = "";
			searchString = searchString.substr(pos);
			
			//start regular expression iterators
			if(regex_search(searchString, matches, NCapRegex))
			{
				//append stats to data string
				dataLine = dataLine + matches[1].str() + ",";
				
				//make position NA into a blank string
				if(matches[2] == "NA")
				{
					dataLine = dataLine + ",";
				}
				
				else
				{
					dataLine = dataLine + matches[2].str() + ",";
				}
				
				//determine whether starter or not
				if(starter)
				{
					dataLine = dataLine + "Starter,";
				}
				
				else
				{
					dataLine = dataLine + "Bench,";
				}
				
				//append data in formated string
				for(int i = 0; i < 17; i++)
				{
					dataLine = dataLine + "0,";
				}
				//determine whether home or not
				if(home)
				{
					//assign stats found if Home
					dataLine = dataLine + "Home,";
					dataLine = dataLine + stringOfHomeTeam + ",";
					dataLine = dataLine + stringOfAwayTeam + ",";
					
					//determine winner and append data
					if (stoi(stringOfHomeScore) > stoi(stringOfAwayScore))
					{
						dataLine = dataLine + "Won,";
					}
					
					else
					{
						dataLine = dataLine + "Loss,";
					}
					
					dataLine = dataLine + listOfFastBreakPoints[1] + ",";
					dataLine = dataLine + listOfPointsInThePaint[1] + ",";
					dataLine = dataLine + listOfTeamTurnoversPoints[1] + ",";
					dataLine = dataLine + listOfPointsOffTurnovers[1] + ",";
				}
				
				else
				{
					dataLine = dataLine + "Away,";
					dataLine = dataLine + stringOfAwayTeam + ",";
					dataLine = dataLine + stringOfHomeTeam + ",";
					
					if (stoi(stringOfAwayScore) > stoi(stringOfHomeScore))
					{
						dataLine = dataLine + "Won,";
					}
					
					else
					{
						dataLine = dataLine + "Loss,";
					}
					
					dataLine = dataLine + listOfFastBreakPoints[0] + ",";
					dataLine = dataLine + listOfPointsInThePaint[0] + ",";
					dataLine = dataLine + listOfTeamTurnoversPoints[0] + ",";
					dataLine = dataLine + listOfPointsOffTurnovers[0] + ",";
				}
				
				dataLine = dataLine + stringOfGameTime + ",";
				
				dataLine = dataLine + stringOfEventLocation + ",";
				
				dataLine = dataLine + stringOfOfficials;
				
				dataLine = dataLine + stringOfAttendance;
				
				pos = matches.position(2);
				
				//fix the format of Jr and Sr
				JrSrFormat(dataLine);
				
				dataLineList.push_back(dataLine);
			}
			
			else
			{
				break;
			}
		}
	}
	
	//if an error occurs exit program
	catch(regex_error& e)
	{
		cout << "A regex error has occured" << endl;
		exit (1);
	}
}

void JrSrFormat(string &dataLine)
{
	size_t foundJrTypeZero;
	size_t foundJrTypeOne;
	size_t foundJrTypeTwo;
	size_t foundSrTypeZero;
	size_t foundSrTypeOne;
	size_t foundSrTypeTwo;
	
	string JrTypeZero = "Jr.";
	string JrTypeOne = ",Jr";
	string JrTypeTwo = ", Jr";
	string SrTypeZero = "Sr.";
	string SrTypeOne = ",Sr";
	string SrTypeTwo = ", Sr";
	
	while(1)
	{
		foundJrTypeZero = dataLine.find(JrTypeZero);
		if (foundJrTypeZero != std::string::npos)
		{
			dataLine.erase(foundJrTypeZero + 2, 1);
		}
		
		else
		{
			break;
		}
	}
	
	while(1)
	{
		foundJrTypeOne = dataLine.find(JrTypeOne);
		if (foundJrTypeOne != std::string::npos)
		{
			dataLine.replace(foundJrTypeOne, 1, " ");
		}
		
		else
		{
			break;
		}
	}
	
	while(1)
	{
		foundJrTypeTwo = dataLine.find(JrTypeTwo);
		if (foundJrTypeTwo != std::string::npos)
		{
			dataLine.erase(foundJrTypeTwo, 1);
		}
		
		else
		{
			break;
		}
	}
	
	while(1)
	{
		foundSrTypeZero = dataLine.find(SrTypeZero);
		if (foundSrTypeZero != std::string::npos)
		{
			dataLine.erase(foundSrTypeZero + 2, 1);
		}
		
		else
		{
			break;
		}
	}
	
	while(1)
	{
		foundSrTypeOne = dataLine.find(SrTypeOne);
		if (foundSrTypeOne != std::string::npos)
		{
			dataLine.replace(foundSrTypeOne, 1, " ");
		}
		
		else
		{
			break;
		}
	}
	
	while(1)
	{
		foundSrTypeTwo = dataLine.find(SrTypeTwo);
		if (foundSrTypeTwo != std::string::npos)
		{
			dataLine.erase(foundSrTypeTwo, 1);
		}
		
		else
		{
			break;
		}
	}
}

void emptyAllVectors()
{
	//clear out all vectors to keep memory low
	listOfTeamData.clear();
	listOfStartTable.clear();
	listOfBenchTable.clear();
	listOfFastBreakPoints.clear();
	listOfPointsInThePaint.clear();
	listOfTeamTurnoversPoints.clear();
	listOfPointsOffTurnovers.clear();
	dataLineList.clear();
}
//listDir algorithm "http://stackoverflow.com/questions/4204666/how-to-list-files-in-a-directory-in-a-c-program"
//get_file_contents algorithm "http://insanecoding.blogspot.com/2011/11/how-to-read-in-file-in-c.html"
//fast "file exists" algorithm http://stackoverflow.com/questions/12774207/fastest-way-to-check-if-a-file-exist-using-standard-c-c11-c
