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
#include <errno.h>

using namespace std;

void listDir(vector<string> &filesInDirectory);
string get_file_contents(const char *filename);

string data = "";

int main()
{
	vector<string> filesInGameDirectory;
	vector<string> filesInCutGameDirectory;

	//get cwd;
	char* cwd;
	char buffer[PATH_MAX];
	cwd = getcwd(buffer, PATH_MAX);
	string currentDirectory(cwd);
	
	int mkdirRet;
	int chdirRet;
	int renameRet;

	//create directory strings
	string masterDirectory = currentDirectory + "/ESPN_HTML_Cache";
	string dayDirectory = masterDirectory + "/Day_Files";
	string gameDirectory = masterDirectory + "/Game_Files";
	string CompletedGameDirectory = masterDirectory + "/Completed_Game_Files";
	string cutDownGameDirectory = masterDirectory + "/Cut_Down_Game_Files";

	//convert directory strings to const char for use in mkdir
	const char* cMasterDirectory = masterDirectory.c_str();
	const char* cDayDirectory = dayDirectory.c_str();
	const char* cGameDirectory = gameDirectory.c_str();
	const char* cCompletedGameDirectory = CompletedGameDirectory.c_str();
	const char* cCutDownGameDirectory = cutDownGameDirectory.c_str();

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
	
	mkdirRet = mkdir(cCutDownGameDirectory, S_IRWXU | S_IRWXG | S_IRWXO);
	if (mkdirRet == -1)
		cout << "\"" << cCutDownGameDirectory << "\" already exists!" << endl;
	
	//change to cut_game directory
	chdirRet = chdir(cCutDownGameDirectory);
	if (chdirRet == -1)
	{
		cout << "Could not change directory! Quitting." << endl;
		return 1;
	}
	//create a list of files currently in cut_game_directory
	listDir(filesInCutGameDirectory);

	//change to game directory
	chdirRet = chdir(cGameDirectory);
	if (chdirRet == -1)
	{
		cout << "Could not change directory! Quitting." << endl;
		return 1;
	}
	//create a list of files currently in directory
	listDir(filesInGameDirectory);

	//filesIn_Game_Directory		-- File Format: #########.html
	//filesIn_Cut_Game_Directory	-- File Format: Cut_#########.html
	//See if Files have already been cut!!
	
	for (size_t i = 2; i < filesInGameDirectory.size(); i++) 
	{
		//change to game directory
		chdirRet = chdir(cGameDirectory);
		if (chdirRet == -1)
		{
			cout << "Could not change directory! Quitting." << endl;
			return 1;
		}
		
		string gameID = filesInGameDirectory[i];
		gameID = "Cut_" + gameID;

		bool skip_bool = false;

		if (find(filesInCutGameDirectory.begin(), filesInCutGameDirectory.end(), gameID) != filesInCutGameDirectory.end()) 
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
			
			const char* c_temp_string = filesInGameDirectory[i].c_str();
			
			data = get_file_contents(c_temp_string);
			//cout << data << endl;
			
			skip_bool = false;
			
			size_t found = data.find("<script type=\"text/javascript\" src=\"");
			if (found==std::string::npos){
				skip_bool = true;
			}
			
			if (skip_bool)
			{
				//Skip This File
				cout << filesInGameDirectory[i] << " has an error with finding the first key" << endl;
			}
			else{
				skip_bool = false;
				
				string first_sub_string = data.substr(found);
				//cout << data << endl;
				
				found = first_sub_string.find("<!-- end sp links -->");
				if (found==std::string::npos){
					skip_bool = true;
				}
				
				if (skip_bool)
				{
					//Skip This File
					cout << filesInGameDirectory[i] << " has an error with finding the second key" << endl;
				}
				else{
					
					string second_sub_string = first_sub_string.substr(0, found);
					
					//cout << second_sub_string << endl;
					
					//Save Current Small File
					//change to cut_game directory
					chdirRet = chdir(cCutDownGameDirectory);
					if (chdirRet == -1)
					{
						cout << "Could not change directory! Quitting." << endl;
						return 1;
					}
					
					ofstream cut_file(gameID);
					cut_file << second_sub_string;
					cut_file.close();
					
					//Rename Old file to CompletedGameDirectory+/+name
					
					string old_string_name = gameDirectory + "/" + filesInGameDirectory[i];
					string new_string_name = CompletedGameDirectory + "/" + filesInGameDirectory[i];
					
					const char* c_old_string_name = old_string_name.c_str();
					const char* c_new_string_name = new_string_name.c_str();
					
					renameRet = rename(c_old_string_name, c_new_string_name);
					
					if ( renameRet == 0 ){
						//puts ( "File successfully renamed" );
						//Do nothing
					}
					else{
						perror( "Error renaming file" );
						string temp_cin_string;
						cin >> temp_cin_string;
						return 1;
					}
				}
			}
		}
	}
	
	cout << "Done Cutting Files Down!" << endl;
	
	return 0;
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

//listDir algorithm "http://stackoverflow.com/questions/4204666/how-to-list-files-in-a-directory-in-a-c-program"
//get_file_contents algorithm "http://insanecoding.blogspot.com/2011/11/how-to-read-in-file-in-c.html"

