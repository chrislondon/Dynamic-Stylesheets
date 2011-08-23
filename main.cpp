/*
 * main.cpp
 *
 *  Created on: Aug 23, 2011
 *      Author: chrislondon
 */

#include <iostream>
#include <fstream>
#include <string>
using namespace std;

struct node {
    string key;
    string val;
    node *nxt;
};

int main () {
    const char *path = getenv("PATH_TRANSLATED" );

    string valid_chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890-_";
    string line;

    // linked list of key and value strings (key, val)
    node *vals = NULL;
    node *temp, *temp2;

    size_t at_found;
    size_t colon_found;
    size_t end_found;

    ifstream myfile (path);
    if (myfile.is_open()) {
        cout << "Content-type: text/css\n\n";

        while (!myfile.eof() ) {
            getline(myfile, line);

            // find @
            at_found = line.find_first_of("@");

            if (at_found != string::npos) {
                 // find :
                colon_found = line.find_first_of(":", at_found + 1);

                // if :
                if (colon_found != string::npos) {
                    // find ; and store as last
                    end_found = line.find_first_of(";", colon_found + 1);

                    // take all from : to ; and remove starting and ending non-characters
                    temp = new node;
                    temp->key = line.substr(at_found + 1, colon_found - at_found - 1);
                    temp->val = line.substr(colon_found + 1, end_found - colon_found - 1);
                    temp->nxt = NULL;

                    // store data
                    if (vals == NULL)
                        vals = temp;
                    else {
                        temp2 = vals;

                        while (temp2->nxt != NULL) {
                            temp2 = temp2->nxt;
                        }

                        temp2->nxt = temp;
                    }

                // else
                } else {
                    // find next none character or ; and store as last
                    end_found = line.find_first_not_of(valid_chars, at_found + 1);

                    temp = vals;

                    while (temp != NULL) {
                        if (temp->key == line.substr(at_found + 1, end_found - at_found - 1)) {
                            // retrieve stored data and replace
                            line.replace(at_found, end_found - at_found, temp->val);

                            break;
                        }
                        temp = temp->nxt;
                    }
                }
            }
            // loop through line again after last

            cout << line << endl;
        }

        myfile.close();
    } else cout << "Unable to open file\n";

    return 0;
}
