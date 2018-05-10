#include "sender.h"

#include <iostream>
#include <string>

using namespace std;

int main(int argc, char** argv) {
    if (argc < 1) {
        cout << "No files to process. Exiting." << endl;
        return 0;
    }

    ifstream botCredsFile;
    botCredsFile.open(".telegram_bot");
    string botCreds, telegramChannelName;
    if (!getline(botCredsFile, botCreds)) {
        cout << "Failed to read bot credentials. Exiting." << endl;
        return 0;
    }
    if (!getline(botCredsFile, telegramChannelName)) {
        cout << "Failed to read chat_id. Exiting." << endl;
        return 0;
    }
    botCredsFile.close();

    for(int i=0; i<argc; i++) {
        string filename(argv[i]);
        cout << "Uploading " << filename << " ... ";

        int res = sendPhotoToTelegram(filename, botCreds, telegramChannelName);

        cout << (res == 0 ? "SUCCESS" : "FAIL") << endl;
    }
    return 0;
}