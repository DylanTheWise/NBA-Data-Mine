#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>

using namespace std;

void listDir(vector<string> &filesInDirectory);
string get_file_contents(const char *filename);
bool invalidChar (char c);
void stripUnicode(string &str);

string data = "";

int main()
{
	vector<string> filesInCutGameDirectory;
	vector<string> filesInAsciiDirectory;
	vector<string> filesInExtractedGameDirectory;
	
	//make directory return values
	int mkdirRet;
	int chdirRet;
	
	//get cwd 
	char* cwd;
	char buffer[PATH_MAX];
	cwd = getcwd(buffer, PATH_MAX);
	string currentDirectory(cwd);
	
	//create directory strings
	string masterDirectory = currentDirectory + "/ESPN_HTML_Cache";
	string cutDownGameDirectory = masterDirectory + "/Cut_Down_Game_Files";
	string asciiDirectory = masterDirectory + "/Ascii_Cut_Game_Files";
	string extractedGameDirectory = masterDirectory + "/Data_Extracted_Files";
	
	//convert directory strings to const char for use in mkdir
	const char* cMasterDirectory = masterDirectory.c_str();
	const char* cCutDownGameDirectory = cutDownGameDirectory.c_str();
	const char* cAsciiDirectory = asciiDirectory.c_str();
	const char* cExtractedGameDirectory = extractedGameDirectory.c_str();
	
	//create directoies incase they dont exist.
	mkdirRet = mkdir(cMasterDirectory, S_IRWXU | S_IRWXG | S_IRWXO);
	if (mkdirRet == -1)
		cout << "\"" << cMasterDirectory << "\" already exists!" << endl;
		
	mkdirRet = mkdir(cCutDownGameDirectory, S_IRWXU | S_IRWXG | S_IRWXO);
	if (mkdirRet == -1)
		cout << "\"" << cCutDownGameDirectory << "\" already exists!" << endl;
	
	mkdirRet = mkdir(cAsciiDirectory, S_IRWXU | S_IRWXG | S_IRWXO);
	if (mkdirRet == -1)
		cout << "\"" << cAsciiDirectory << "\" already exists!" << endl;
		
	mkdirRet = mkdir(cExtractedGameDirectory, S_IRWXU | S_IRWXG | S_IRWXO);
	if (mkdirRet == -1)
		cout << "\"" << cExtractedGameDirectory << "\" already exists!" << endl;
		
		
	//change to cut_game directory
	chdirRet = chdir(cCutDownGameDirectory);
	if (chdirRet == -1)
	{
		cout << "Could not change directory! Quitting." << endl;
		return 1;
	}
	
	//list all files in cut_game directory
	listDir(filesInCutGameDirectory);
	
	//change to cut_game directory
	chdirRet = chdir(cExtractedGameDirectory);
	if (chdirRet == -1)
	{
		cout << "Could not change directory! Quitting." << endl;
		return 1;
	}
	
	//list all files in cut_game directory
	listDir(filesInExtractedGameDirectory);
	
	//change to ascii_game directory
	chdirRet = chdir(cAsciiDirectory);
	if (chdirRet == -1)
	{
		cout << "Could not change directory! Quitting." << endl;
		return 1;
	}
	//list all files in ascii_game directory
	listDir(filesInAsciiDirectory);
	
	//change to cut game directory
	chdirRet = chdir(cCutDownGameDirectory);
	if (chdirRet == -1)
	{
		cout << "Could not change directory! Quitting." << endl;
		return 1;
	}
	
	
	for(size_t i = 2; i < filesInCutGameDirectory.size(); i++) 
	{
		string gameID = filesInCutGameDirectory[i];
		gameID = gameID.substr(4);
		string AsciiGameID = "Ascii_" + gameID;

		bool skip_bool = false;

		if (find(filesInAsciiDirectory.begin(), filesInAsciiDirectory.end(), AsciiGameID) != filesInAsciiDirectory.end()) 
		{
			skip_bool = true;
		}
		
		if (find(filesInExtractedGameDirectory.begin(), filesInExtractedGameDirectory.end(), gameID) != filesInExtractedGameDirectory.end()) 
		{
			skip_bool = true;
		}

		if (skip_bool)
		{
			//Skip This File
		}
		
		else
		{
			data = "";
			
			const char* fileName = filesInCutGameDirectory[i].c_str();
			data = get_file_contents(fileName);
			
			//strip all non unicode characters
			stripUnicode(data);
			
			string tempString = asciiDirectory + "/" + AsciiGameID;
			const char* cTempString = tempString.c_str();
			
			fstream fileInput;
			fileInput.open (cTempString, std::fstream::in | std::fstream::out | std::fstream::app);
			fileInput << data;
			fileInput.close();
		}
	}
}

void listDir(vector<string> &filesInDirectory)
{
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

bool invalidChar (char c)
{
	return !(c >= 0 && c < 128);  
}

void stripUnicode(string &str)
{
	str.erase(remove_if(str.begin(),str.end(), invalidChar), str.end());
}

//non ascii striping of strings found at "http://stackoverflow.com/questions/10178700/c-strip-non-ascii-characters-from-string"
//listDir algorithm "http://stackoverflow.com/questions/4204666/how-to-list-files-in-a-directory-in-a-c-program"
//get_file_contents algorithm "http://insanecoding.blogspot.com/2011/11/how-to-read-in-file-in-c.html"
