#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <ctime>
#include <algorithm>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <curl/curl.h>
#include <dirent.h>
#include <regex>

using namespace std;

//global for file contents
string data;

//constants
const vector<int> thirtyDays {4,6,9,11};
const vector<int> thirtyOneDays {1,3,5,7,8,10,12};
const string zero = "0";

//function prototypes
size_t writeCallback(char* buf, size_t size, size_t nmemb, void* up);
string iterateDate(int &currentDay, int &currentMonth, int &currentYear, string &masterLink);
int urlOpen(string URL);
void listDir(vector<string> &filesInDirectory);
string get_file_contents(const char *filename);
bool findIfDateIsValid(string gameDay, string todayDate);

int main()
{
	stringstream dateBuffer;
	string todayDate;
	string masterLink = "http://espn.go.com/nba/schedule?date=";
	string currentDayStr;
	string currentMonthStr;
	string currentYearStr;
	const string zero = "0";
	stringstream getDate;
	string getDateStr;
	
	vector<string> filesInDayDirectory;
	vector<string> filesInGameDirectory;
	vector<string> filesInCompletedGameDirectory;
	vector<string> filesInExtractedDirectory;
	vector<string> listOfURLs;
	vector<string> listOfGameIDs;
	
	bool skipFile = false;
	bool invalidDate = false;
	
	int currentDay;
	int currentMonth;
	int currentYear;
	
	int mkdirRet;
	int chdirRet;
	int urlOpenRet;
	
	//get date in correct format
	time_t t = time(0);
	struct tm * now = localtime( & t );
	dateBuffer << now->tm_mday << ' ' 
		<< (now->tm_mon + 1) << ' '
		<< (now->tm_year + 1900);
	
	todayDate = dateBuffer.str();
	//assign a temporary date until automatic algorithm works during game season
	
	//initialize conditionals for while loop
	vector<int> thirtyDays {4,6,9,11};
	vector<int> thirtyOneDays {1,3,5,7,8,10,12};
	string currentDateVariable = "01 09 2012";
	string urlLoopLink = masterLink + "20120901";
	string gameDay = "";
	
	//convert date to stringstream
	getDate.str(currentDateVariable);
	
	getDateStr = getDate.str();
	
	//take parts of stringstream and put them into respective strings
	int getDateIterator = 0;
	while (getDateStr != "")
	{
		if (getDateIterator == 0)
			getDate >> currentDayStr;
			
		if (getDateIterator == 1)
			getDate >> currentMonthStr;
			
		if (getDateIterator == 2)
			getDate >> currentYearStr;
			
		if (getDateIterator == 3)
			break;
			
		getDateIterator++;
	}
	
	currentDay = stoi(currentDayStr);
	currentMonth = stoi(currentMonthStr);
	currentYear = stoi(currentYearStr);
	
	//get cwd 
	char* cwd;
	char buffer[PATH_MAX];
	cwd = getcwd(buffer, PATH_MAX);
	string currentDirectory(cwd);
	
	//create directory strings
	string masterDirectory = currentDirectory + "/ESPN_HTML_Cache";
	string dayDirectory = masterDirectory + "/Day_Files";
	string gameDirectory = masterDirectory + "/Game_Files";
	string completedGameDirectory = masterDirectory + "/Completed_Game_Files";
	string extractedDirectory = masterDirectory + "/Data_Extracted_Files";
	
	//convert directory strings to const char for use in mkdir
	const char* cMasterDirectory = masterDirectory.c_str();
	const char* cDayDirectory = dayDirectory.c_str();
	const char* cGameDirectory = gameDirectory.c_str();
	const char* cCompletedGameDirectory = completedGameDirectory.c_str();
	const char* cExtractedDirectory = extractedDirectory.c_str();
	
	//Create HTML cache directories
	mkdirRet = mkdir(cMasterDirectory, S_IRWXU | S_IRWXG | S_IRWXO);
	if (mkdirRet == -1)
		cout << "\"" << cMasterDirectory << "\" already exists!" << endl;
	
	mkdirRet = mkdir(cDayDirectory, S_IRWXU | S_IRWXG | S_IRWXO);
	if (mkdirRet == -1)
		cout << "\"" << cDayDirectory << "\" already exists!" << endl;
		
	mkdirRet = mkdir(cGameDirectory, S_IRWXU | S_IRWXG | S_IRWXO);
	if (mkdirRet == -1)
		cout << "\"" << cGameDirectory << "\" already exists!" << endl;
		
	mkdirRet = mkdir(cCompletedGameDirectory, S_IRWXU | S_IRWXG | S_IRWXO);
	if (mkdirRet == -1)
		cout << "\"" << cCompletedGameDirectory << "\" already exists!" << endl;
		
	mkdirRet = mkdir(cExtractedDirectory, S_IRWXU | S_IRWXG | S_IRWXO);
	if (mkdirRet == -1)
		cout << "\"" << cExtractedDirectory << "\" already exists!" << endl;
	
	//change to day directory
	chdirRet = chdir(cDayDirectory);
	if (chdirRet == -1)
	{
		cout << "Could not change directory! Quitting." << endl;
		return 1;
	}
	
	//create a list of files currently in directory
	listDir(filesInDayDirectory);
	
	//loop through all the days
	while(currentDateVariable != todayDate)
	{
		//see if file is already obtained
		string fileCheck = currentDateVariable + ".HTML";
		if(find(filesInDayDirectory.begin(), filesInDayDirectory.end(), fileCheck) != filesInDayDirectory.end())
		{
			skipFile = true;
		}
		
		//if file has not been downloaded yet download
		if (skipFile == false)
		{
			//try to open the link 5 times
			for (int i = 0; i < 5; i++)
			{
				data = "";
				urlOpenRet = urlOpen(urlLoopLink);
				
				//if successful break out of the loop
				if (urlOpenRet == 0)
				{
					break;
				}
				sleep(15);
			}
			
			//if after 5 tries we still fail quit the program
			if (urlOpenRet == -1)
			{
				return 1;
			}
			
			//save file
			cout << "Saving " << currentDateVariable << endl;
			ofstream out(currentDateVariable + ".HTML");
			out << data;
			out.close();
		}
		
		//if file has been downloaded skip it
		else
		{
			cout << "Skipped " << currentDateVariable << endl;
		}
		
		//function to increase the day by 1
		urlLoopLink = iterateDate(currentDay, currentMonth, currentYear, masterLink);
		
		//update date variable
		currentDayStr = to_string(currentDay);
		currentMonthStr = to_string(currentMonth);
		currentYearStr = to_string(currentYear);
		currentDateVariable = currentDayStr + " " + currentMonthStr + " " + currentYearStr;
		
		skipFile = false;
	}
	
	//function to get a list of files in the directory
	listDir(filesInDayDirectory);
	
	//first two elements are '.' and '..' so we skip those
	for (size_t i = 2; i < filesInDayDirectory.size(); i++)
	{
		//get contents of file
		string fileName = filesInDayDirectory[i];
		const char* cFileName = fileName.c_str(); 
		data = get_file_contents(cFileName);
		
		try
		{
			//init regular expression
			const regex boxScoreR("gameId=(\\d+)\">");
			
			//start regular expression iterators
			sregex_token_iterator next(data.begin(), data.end(), boxScoreR, 1);
			sregex_token_iterator End;
			
			//push_back matched IDs
			for(; next != End; ++next)
			{
				listOfGameIDs.push_back(*next);
			}
		}
		
		//if an error occurs exit program
		catch(regex_error& e)
		{
			cout << "A regex error has occured" << endl;
			return 1;
		}
		
		//for (int o = 0; o < listOfGameIDs.size(); o++)
		//{
			//cout << listOfGameIDs[o] << endl;
		//}
		
	}
	
	//get rid of duplicates in the game ID vector
	sort(listOfGameIDs.begin(), listOfGameIDs.end() );
	listOfGameIDs.erase(unique(listOfGameIDs.begin(), listOfGameIDs.end() ), listOfGameIDs.end());
	
	//generate game URLs from game IDs
	for (size_t i = 0; i < listOfGameIDs.size(); i++)
	{
        listOfURLs.push_back("espn.go.com/nba/boxscore?gameId=" + listOfGameIDs[i]);
	}
	
	//gather files from game directory, completed game directory and extracted directory
	chdirRet = chdir(cExtractedDirectory);
	if (chdirRet == -1)
	{
		cout << "Could not change directory! Quitting." << endl;
		return 1;
	}
	listDir(filesInExtractedDirectory);
	
	chdirRet = chdir(cCompletedGameDirectory);
	if (chdirRet == -1)
	{
		cout << "Could not change directory! Quitting." << endl;
		return 1;
	}
	listDir(filesInCompletedGameDirectory);
	
	chdirRet = chdir(cGameDirectory);
	if (chdirRet == -1)
	{
		cout << "Could not change directory! Quitting." << endl;
		return 1;
	}
	listDir(filesInGameDirectory);
	
	skipFile = false;
	for (size_t i = 0; i < listOfGameIDs.size(); i++)
	{
		invalidDate = false;
		string tempFileName = listOfGameIDs[i] + ".HTML";
		
		if(find(filesInExtractedDirectory.begin(), filesInExtractedDirectory.end(), tempFileName) != filesInExtractedDirectory.end())
		{
			skipFile = true;
		}
		
		if(find(filesInCompletedGameDirectory.begin(), filesInCompletedGameDirectory.end(), tempFileName) != filesInCompletedGameDirectory.end())
		{
			skipFile = true;
		}
		
		if(find(filesInGameDirectory.begin(), filesInGameDirectory.end(), tempFileName) != filesInGameDirectory.end())
		{
			skipFile = true;
		}
		
		if (skipFile == false)
		{
			//try to open the link 5 times
			for (int k = 0; k < 5; k++)
			{
				data = "";
				urlOpenRet = urlOpen(listOfURLs[i]);
				
				//if successful break out of the loop
				if (urlOpenRet == 0)
				{
					break;
				}
				sleep(15);
			}
			
			//if after 5 tries we still fail quit the program
			if (urlOpenRet == -1)
			{
				return 1;
			}
			
			try
			{
				smatch matches;
				const regex NOTFOUND("NOT FOUND:");
				
				if(regex_search(data, matches, NOTFOUND))
				{
					invalidDate = true;
				}
				
				else
				{
					//init regular expression
					const regex findDayR("- Box Score - (.*?) - ESPN");
				
					//start regular expression iterators
					sregex_token_iterator next(data.begin(), data.end(), findDayR, 1);
					sregex_token_iterator End;
				
					gameDay = *next;
				}
			}
			
			catch(regex_error& e)
			{
				cout << "A regex error has occured" << endl;
				return 1;
			}
			
			if(invalidDate == false)
			{
				invalidDate = findIfDateIsValid(gameDay, todayDate);
			}
			
			if(invalidDate == false)
			{
				cout << "Saving " << listOfGameIDs[i] << endl;
				ofstream out(listOfGameIDs[i] + ".HTML");
				out << data;
				out.close();
			}
		}
		
		skipFile = false;
	}
	
	return 0;
}

size_t writeCallback(char* buf, size_t size, size_t nmemb, void* up)
{	//callback must have this declaration
	//buf is a pointer to the data that curl has for us
	//size*nmemb is the size of the buffer
	
	for (size_t c = 0; c < size*nmemb; c++)
	{
		data.push_back(buf[c]);
	}
	return size*nmemb; //tell curl how many bytes we handled
}


int urlOpen(string URL)
{
	CURL *curl;
	CURLcode res;
	string userAgent = "Mozilla/5.0 (Windows NT 6.1; WOW64; rv:40.0) Gecko/20100101 Firefox/40.1";
	
	curl_global_init(CURL_GLOBAL_ALL);
	curl = curl_easy_init();
	if(curl) 
	{
		curl_easy_setopt(curl, CURLOPT_URL, URL.c_str());
		curl_easy_setopt(curl, CURLOPT_USERAGENT, userAgent.c_str());
		// example.com is redirected, so we tell libcurl to follow redirection
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &writeCallback);
		//curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

		//curl_easy_setopt(curl, CURLOPT_POST, 1);
		
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
		
		//Perform the request, res will get the return code 
		res = curl_easy_perform(curl);
		//Check for errors  
		if(res != CURLE_OK)
		{
			fprintf(stderr, "curl_easy_perform() failed: %s\n",
				curl_easy_strerror(res));
			
			return -1;
		}
		
		curl_easy_cleanup(curl);
		curl_global_cleanup();
	}
	
	return 0;
}

string iterateDate(int &currentDay, int &currentMonth, int &currentYear, string &masterLink)
{
	//special cases for february during leap years
	string dayString;
	string monthString;
	string yearString;
	string urlLoopLink;
	
	if (currentDay == 28)
	{
		if (currentMonth == 2 && (currentYear % 4 != 0))
		{
			currentMonth += 1;
			currentDay = 0;
		}
	}
	
	else if (currentDay == 29)
	{
		if (currentYear % 4 == 0 && currentMonth == 2)
		{
			currentMonth += 1;
			currentDay = 0;
		}
	}
	
	else if (currentDay == 30)
	{
		//check if current month is in thirtyDay vector
		if(find(thirtyDays.begin(), thirtyDays.end(), currentMonth) != thirtyDays.end())
		{
			currentMonth += 1;
			currentDay = 0;
		}
	}
	
	else if (currentDay == 31)
	{
		//check if current month is in thirtyOneDay vector
		if(find(thirtyOneDays.begin(), thirtyOneDays.end(), currentMonth) != thirtyOneDays.end())
		{
			currentMonth += 1;
			currentDay = 0;
		}
	}
	
	//skip from april to oct, no basketball during that time
	//if (currentDay > 15 && currentMonth == 4)
	//{
	//	currentMonth = 10;
	//	currentDay = 25;
	//}
	
	if (currentMonth == 13)
	{
		currentMonth = 1;
		currentYear += 1;
	}
	
	currentDay += 1;
	
	dayString = to_string(currentDay);
	monthString = to_string(currentMonth);
	yearString = to_string(currentYear);
	if (currentMonth < 10)
	{
		if (currentDay < 10)
		{
			urlLoopLink = masterLink + yearString + zero + monthString + zero + dayString;
		}
		
		else
		{
			urlLoopLink = masterLink + yearString + zero + monthString + dayString;
		}
	}
	
	else
	{
		if (currentDay < 10)
		{
			urlLoopLink = masterLink + yearString + monthString + zero + dayString;
		}
		
		else
		{
			urlLoopLink = masterLink + yearString + monthString + dayString;
		}
	}
	
	return urlLoopLink;
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

bool findIfDateIsValid(string gameDay, string todayDate)
{
	size_t found;
	
	while(1)
	{
		found = gameDay.find(",");
		if (found != std::string::npos)
		{
			gameDay.erase(found, 1);
		}
			
		else
		{
			break;
		}
	}
	
	vector<string> listOfMonths = {"January","February","March","April","May","June","July","August","September","October","November","December"};
	
	int gameDayDay, gameDayMonth, gameDayYear;
	string gameDayDayStr, gameDayMonthStr, gameDayYearStr;
	
	int todayDateDay, todayDateMonth, todayDateYear;
	string todayDateDayStr, todayDateMonthStr, todayDateYearStr;
	
	stringstream gameDaySS;
	stringstream todayDateSS;
	
	gameDaySS.str(gameDay);
	
	//take parts of stringstream and put them into respective strings
	int gameDayIterator = 0;
	while (1)
	{
		if (gameDayIterator == 0)
			gameDaySS >> gameDayMonthStr;
			
		if (gameDayIterator == 1)
			gameDaySS >> gameDayDayStr;
			
		if (gameDayIterator == 2)
			gameDaySS >> gameDayYearStr;
			
		if (gameDayIterator == 3)
			break;
			
		gameDayIterator++;
	}
	
	for(size_t i = 0; i < listOfMonths.size(); i++)
	{
		if(gameDayMonthStr == listOfMonths[i])
		{
			gameDayMonth = i+1;
		}
	}
	
	gameDayDay = stoi(gameDayDayStr);
	gameDayYear = stoi(gameDayYearStr);
	
	
	todayDateSS.str(todayDate);
	
	//take parts of stringstream and put them into respective strings
	int todayDateIterator = 0;
	while (1)
	{
		if (todayDateIterator == 0)
			todayDateSS >> todayDateDayStr;
			
		if (todayDateIterator == 1)
			todayDateSS >> todayDateMonthStr;
			
		if (todayDateIterator == 2)
			todayDateSS >> todayDateYearStr;
			
		if (todayDateIterator == 3)
			break;
			
		todayDateIterator++;
	}
	
	todayDateDay = stoi(todayDateDayStr);
	todayDateMonth = stoi(todayDateMonthStr);
	todayDateYear = stoi(todayDateYearStr);
	
	if(gameDayYear < todayDateYear)
	{
		return false;
	}
	
	else if(gameDayMonth < todayDateMonth)
	{
		return false;
	}
	
	else if(gameDayDay >= todayDateDay)
	{
		return true;
	}
	
	else
	{
		return false;
	}
}

//download algorithm "http://www.cplusplus.com/forum/unices/45878/"
//listDir algorithm "http://stackoverflow.com/questions/4204666/how-to-list-files-in-a-directory-in-a-c-program"
//get_file_contents algorithm "http://insanecoding.blogspot.com/2011/11/how-to-read-in-file-in-c.html"