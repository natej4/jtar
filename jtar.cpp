#include "file.h"
#include <iostream>
#include <cstdlib>
#include <fstream>
#include <list>
#include <sstream>
#include <sys/stat.h>
#include <sys/types.h>
#include <string>
#include <utime.h>

using namespace std;

//method declarations
int args (int argc, char* argv[]);
void menu(int o, int argc, char* argv[]);
void getList (string s, list<string>& mount);
string read_file_into_string (string filename);
bool directoryCheck(string& name);
string getPmode (const char name[]);
string getSize (const char name[]);
string getTime (const char name[]);
void assignFiles (list<File>& files, list<string>& mount);
void buildTar (list<File>& files);
void txtToTar (fstream& tar, string name);
void unpack(char* name, int o);


int main(int argc, char* argv[]){
    int o = args(argc, argv);
    if (o == -1) 
        return -1;
    else 
        menu (o, argc, argv);

}

int args (int argc, char* argv[]){
    if (argc < 2){
        cerr << "invalid number of arguments" << endl;
        return -1;
    }
    
    if (strcmp(argv[1], "-cf") == 0)
        if (argc > 2) return 1;
        else {
            cerr << "-cf: no file(s) specified" << endl;
            return -1;
        }

    else if (strcmp(argv[1], "-tf") == 0)
        if (argc > 2) return 2;
        else {
            cerr << "-tf: no file specified" << endl;
            return -1;
        }

    else if (strcmp(argv[1], "-xf") == 0)
        if (argc > 2) return 3;
        else{
            cerr << "-xf: no file specified" << endl; 
            return -1;
        }
        
    else if (strcmp(argv[1], "--help")==0)
        return 4;
    
    else{
        cerr << "invalid option" << endl;
        return -1;
    }


}

void menu(int o, int argc, char* argv[]){
    switch (o)
    {
    case 1:{
        list<string> mount;
        for (int i = 2; i < argc; i++)
        {
            getList(argv[i], mount);
        }
        
        list<File> files;
        assignFiles(files, mount);/*
        for(list<File>::iterator it = files.begin(); it != files.end(); it++){
            File local(*it);
            cout << local.getName() << endl;
        }*/
        buildTar(files);
        system("rm temp");
        break;
    }
    case 2:
        unpack(argv[2], 2);
        break;
    case 3:
        unpack(argv[2], 3);
        break;
    case 4:{
        system("cat help");
        break;
    }
    default:
        break;
    }
    
}

void getList (string s, list<string>& mount){
        mount.push_back(s);
        string command, command2, path, buffer;
        if (directoryCheck (s))
        {
                command = "ls " + s + " > temp";
                system (command.c_str());
                //command2 = "cd " + s;
                //system (command2.c_str());
                buffer = read_file_into_string("temp");
                //buffer = getName(buffer);
                istringstream input(buffer);

                string token, check;
                
                while (input >> token)
                {
                    if (s != token)
                    {
                        check = s + "/" + token;
                        getList(check, mount);
                    }
                }
        }
}

string read_file_into_string (string filename){
    ifstream infile (filename.c_str());
    ostringstream buf;
    char ch;

    while (buf && infile.get(ch))
        buf.put(ch);

    return buf.str();
}

bool directoryCheck(string& name){
    struct stat buf;
    lstat(name.c_str(), &buf);
    if (S_ISDIR(buf.st_mode)) 
        return true;
    else return false;
}

string getPmode (const char name[]){
    struct stat buf;
    lstat(name, &buf);
    int u = ((buf.st_mode & S_IRWXU) >> 6);
    int g = ((buf.st_mode & S_IRWXG) >> 3);
    int o = (buf.st_mode & S_IRWXO);
    string p = to_string(u) + to_string(g) + to_string(o);
    return p;
}

string getSize (const char name[]){
    struct stat buf;
    lstat(name, &buf);
    return to_string(buf.st_size);
}

string getTime (const char name[]){
    struct stat buf;
    lstat(name, &buf);
    char stamp[16];
	strftime(stamp, 16, "%Y%m%d%H%M.%S", localtime(&buf.st_ctime));
    return stamp;
}

void assignFiles (list<File>& files, list<string>& mount){
    for(list<string>::iterator it = mount.begin(); it != mount.end(); it++){
        string name;
        name = *it;
        string p = getPmode(name.c_str());
        string size = getSize(name.c_str());
        string time = getTime(name.c_str());
        //cout << name.c_str() << endl;
        File f(name.c_str(), p.c_str(), size.c_str(), time.c_str());
        if (directoryCheck(name)) f.flagAsDir();
        files.push_back(f);
    }

}

void buildTar (list<File>& files){
   /* char* nn;
    cout << "Enter name for new .tar file:";
    cin >> nn;*/
    fstream tar("jtar.tar", ios::out | ios::binary);
    string s = to_string(files.size());
    const char* len = s.c_str();
    tar.seekg(0);
    tar.write(len, 1);

    for(list<File>::iterator it = files.begin(); it != files.end(); it++){
        File local(*it);

        string name = local.getName();
        const char* mode = local.getPmode().c_str();
        const char* size = local.getSize().c_str();
        const char* time = local.getStamp().c_str();
        const char* buf = " ";
        tar.write(name.c_str(), name.size());
        tar.write(buf, 1);
        tar.write(mode, strlen(mode));
        tar.write(buf, 1);
        tar.write(size, strlen(size));
        tar.write(buf, 1);
        tar.write(time, strlen(time));
        tar.write(buf, 1);
        if(!local.isADir()) 
            txtToTar(tar, name);

    }
}

void txtToTar (fstream& tar, string name){
    ifstream text(name.c_str());
    char ch;
    while(text){
        text >> ch;
        //cout << ch;
        string c = to_string(ch);
        tar.write(c.c_str(), 1);
    }
    tar.write(" ", 1);
}

void unpack(char* name, int o){
    ifstream intar(name, ios::in | ios::binary);
    int len;
    char n;
    
    intar >> len;
    if (o == 2){
        for(int i = 0; i<len; i++){
            string output = "";
            while (intar.peek() < 65 && intar.peek() != 47){
                intar.ignore();

            }
                while( intar.peek() >= 65 || intar.peek() == 47){
                    intar >> n;
                    output += n;
                
                }

                intar.get(n);
                output += n;
                if(o == 2) cout << output << endl;
            
        }
    }
    else{

        for(int i = 0; i<len; i++){
            char n;
            bool mark = false;
            string name; string perm; string size; string time;
            while ((char)intar.peek() != ' '){
                intar.get(n);
                name += n;
            }
            intar.ignore();
            while ((char)intar.peek() != ' '){
                intar.get(n);
                perm += n;
            }
            intar.ignore();
            while ((char)intar.peek() != ' '){
                intar.get(n);
                size += n;
            }
            intar.ignore();
            while ((char)intar.peek() != ' '){
                intar.get(n);
                time += n;
            }
            intar.ignore();

            if (intar.peek() < 65){
                //cout << "hey" << endl;
                intar.ignore();
                intar.ignore(10000, ' ');
                mark = true;
            }
                
            //intar.getline(n, 32, ' ');
            //intar.getline(p, 5, ' ');
            //intar.getline(size, 7, ' ');
            //intar.getline(time, 16, ' ');

           File f(name.c_str(), perm.c_str(), size.c_str(), time.c_str());
           
               struct stat buf;
               lstat(name.c_str(), &buf);
               if(S_ISDIR(buf.st_mode));
               else if (S_ISREG(buf.st_mode)){
               string command = "touch " + name;
               system(command.c_str());
               command = "chmod " + perm +" "+ name;
               system(command.c_str());
               command = " touch -1 " + time + " "+name;
           }
        
               else{
                   string command = "mkdir " + name;
                   system(command.c_str());
                   command = "chmod " + perm + " " +name;
                   system(command.c_str());
                   command = "touch -t " + time + " "+ name;
                   system(command.c_str());
               }
           }
           
    }
    
}