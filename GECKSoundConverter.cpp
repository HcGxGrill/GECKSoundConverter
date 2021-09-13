//  GECK Sound Converter
//  Quick Extractor and Converter to allow assigning existing sound files in the Fallout 3/NV GECK since it doesn't allow you to assign anything other than .wav

//  Credits:
//  BSArchive: https://github.com/TES5Edit/TES5Edit/tree/dev/Tools/BSArchive
//  FFMpeg: https://www.ffmpeg.org/

#include "main.h"
#include "resource.h"

int versionMajor = 0;
int versionMinor = 1;

std::string programName = "GECK Sound Converter";
std::string programFilePath = "";
std::string tempDirectoryPath = "C:\\GECKSoundConvTemp\\";
std::string bsarchexe = "bsarch.exe";
std::string ffmpegexe = "ffmpeg.exe";

std::string fileTypeOGG = ".ogg";
std::string fileTypeMP3 = ".mp3";
std::string fileTypeWAV = ".wav";

//  Sets Program File path variable
void SetProgramFilePath(char* myProgramPath)
{
    std::string result = myProgramPath;
    std::string stringToErase = "GECKSoundConverter.exe";

    size_t pos = result.find(stringToErase);
    if (pos != std::string::npos)
    {
        result.erase(pos, stringToErase.length());
    }

    programFilePath = result;
}

//  Writes to Output window
void TraceLog(std::string logString)
{
    std::cout << "\n";
    std::cout << logString;
}

//  Shows Program Name and Version number in Output Window on start
void WelcomeMessage()
{
    std::cout << programName << std::endl;
    std::cout << "Version: " + std::to_string(versionMajor) + "." + std::to_string(versionMinor);
    std::cout << "\n";
    std::cout << "\n";
}

//  Appends file path string with quotation marks
std::string AddQuotesToString(std::string myString)
{
    std::string result = "\"";
    result.append(myString);
    result.append("\"");

    return result;
}

std::wstring GetStringValueFromHKLM(const std::wstring& regSubKey, const std::wstring& regValue)
{
    size_t bufferSize = 0xFFF;
    std::wstring valueBuf;
    valueBuf.resize(bufferSize);
    auto cbData = static_cast<DWORD>(bufferSize * sizeof(wchar_t));
    auto rc = RegGetValueW(
        HKEY_LOCAL_MACHINE,
        regSubKey.c_str(),
        regValue.c_str(),
        RRF_RT_REG_SZ,
        nullptr,
        static_cast<void*>(valueBuf.data()),
        &cbData
    );
    while (rc == ERROR_MORE_DATA)
    {
        cbData /= sizeof(wchar_t);
        if (cbData > static_cast<DWORD>(bufferSize))
        {
            bufferSize = static_cast<size_t>(cbData);
        }
        else
        {
            bufferSize *= 2;
            cbData = static_cast<DWORD>(bufferSize * sizeof(wchar_t));
        }
        valueBuf.resize(bufferSize);
        rc = RegGetValueW(
            HKEY_LOCAL_MACHINE,
            regSubKey.c_str(),
            regValue.c_str(),
            RRF_RT_REG_SZ,
            nullptr,
            static_cast<void*>(valueBuf.data()),
            &cbData
        );
    }
    if (rc == ERROR_SUCCESS)
    {
        cbData /= sizeof(wchar_t);
        valueBuf.resize(static_cast<size_t>(cbData - 1));
        return valueBuf;
    }

    return L"";
}

std::string GetGameFilePathFromRegistry(std::string gameName)
{
    std::wstring regSubKey;

#ifdef _WIN64
    regSubKey = L"SOFTWARE\\WOW6432Node\\Bethesda Softworks\\";
#else
    regSubKey = L"SOFTWARE\\Bethesda Softworks\\";
    
#endif
    regSubKey.append(std::wstring(gameName.begin(), gameName.end()));

    std::wstring regValue(L"Installed Path");
    std::wstring valueFromRegistry;

    try
    {
        valueFromRegistry = GetStringValueFromHKLM(regSubKey, regValue);
    }
    catch (std::exception& e)
    {
    }

    using convert_type = std::codecvt_utf8<wchar_t>;
    std::wstring_convert<convert_type, wchar_t> converter;

    std::string result = converter.to_bytes(valueFromRegistry);

    result += "Data\\";

    return result;
}

std::string RemoveNewLinesFromString(std::string stringToChange)
{
    std::string result = stringToChange;

    result.erase(std::remove(result.begin(), result.end(), '\n'), result.end());

    return result;
}

std::string ChangeFileExtensionToWAV(std::string fileToChange)
{
    std::string result = fileToChange;
    std::string myFileExtension = fileToChange.substr(fileToChange.find_last_of("."));

    size_t pos = result.find(myFileExtension);
    if (pos != std::string::npos)
    {
        result.erase(pos, myFileExtension.length());
        result.append(".wav\"");
    }

    return result;
}

std::string RemoveDoubleSlashFromString(std::string stringToChange)
{
    std::string result = stringToChange;

    std::string from = "\\\\";
    std::string to = "\\";

    size_t start_pos = 0;
    while ((start_pos = result.find(from, start_pos)) != std::string::npos)
    {
        result.replace(start_pos, from.length(), to);
        start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
    }

    return result;
}

void DeleteAudioFile(std::string fileToDelete)
{
    //  Delete File
    std::string fileCommand = "del /f /q ";
    fileCommand += fileToDelete;

    fileCommand = RemoveNewLinesFromString(fileCommand);
    fileCommand = RemoveDoubleSlashFromString(fileCommand);
    //std::cout << fileCommand << std::endl;
    system(fileCommand.c_str());
}

void ExtractArchive(std::string archiveToExtract, std::string extractPath)
{
    std::stringstream command;
    
    command << "Tools\\bsarch unpack " << archiveToExtract << " " << extractPath;

    std::string commandStr = command.str();

    commandStr = RemoveNewLinesFromString(commandStr);

    //std::cout << commandStr << std::endl;
    system(commandStr.c_str());
}

void ConvertAudioFile(std::string fileToConvert)
{
    std::stringstream command;
    
    command << "Tools\\ffmpeg -n -i " << fileToConvert << " " << ChangeFileExtensionToWAV(fileToConvert);
    
    std::string commandStr = command.str();
    
    commandStr = RemoveNewLinesFromString(commandStr);
    
    //std::cout << commandStr << std::endl;
    system(commandStr.c_str());

    if (fileToConvert.find(".wav", 0) == std::string::npos)
    {
        //  Not a WAV, delete
        DeleteAudioFile(fileToConvert);
    }
}

int main(int argc, char* argv[])
{
    SetProgramFilePath(argv[0]);

    WelcomeMessage();

    std::cout << "This program will extract Sound Archives for the selected game and convert the files for use within the GECK.\nPlease note this can take a while depending on your hardware.\n\n";
    std::cout << "Which game would you like to use?\n";
    std::cout << "1. Fallout 3\n";
    std::cout << "2. Fallout New Vegas\n";
    std::cout << "3. Use Current Folder (This will find any Sound Archives in the program folder)\n";
    //std::cout << "4. Skyrim LE\n";
    //std::cout << "5 .Skyrim SE\n";    Potentially plan to add these later.
    //std::cout << "6. Fallout 4\n";
    std::cout << "4. Exit Program\n\n";
    std::cout << "Enter your selection > ";

    int game = -1;
    std::string gamePath;

    while (game == -1)
    {
        std::cin >> game;
        
        switch (game)
        {
        case 1:
            //  Fallout 3
            gamePath = GetGameFilePathFromRegistry("Fallout3");
            break;
        case 2:
            //  Fallout NV
            gamePath = GetGameFilePathFromRegistry("FalloutNV");
            break;
        case 3:
            //  Use Current Folder
            gamePath = programFilePath;
            break;
        case 4:
            //  Exit Program

            break;
        default:
            //  Invalid Selection

            std::cout << "Invalid selection. Please Try Again.\n" << std::endl;
            std::cout << "Enter your selection > ";
            game = -1;
            break;
        }
    }

    if (game != 4)
    {
        //  Create temporary directory
        tempDirectoryPath = gamePath;
        tempDirectoryPath.append("GECKSoundConvTEMP\\");
        _mkdir(tempDirectoryPath.c_str());

        //  Copy Music folder to Temp folder
        std::string copyMusicCommand = "Xcopy /q /e /y /i \"";
        copyMusicCommand += gamePath;
        copyMusicCommand += "Music\" \"";
        copyMusicCommand += tempDirectoryPath;
        copyMusicCommand += "Music\"";
        //copyMusicCommand += "Music\"";
        copyMusicCommand = RemoveNewLinesFromString(copyMusicCommand);
        copyMusicCommand = RemoveDoubleSlashFromString(copyMusicCommand);
        
        //std::cout << copyMusicCommand << std::endl;
        system(copyMusicCommand.c_str());

        //  Copy Sound folder to Temp folder
        std::string copySoundCommand = "Xcopy /q /e /y /i \"";
        copySoundCommand += gamePath;
        copySoundCommand += "Sound\" \"";
        copySoundCommand += tempDirectoryPath;
        copySoundCommand += "Sound\"";
        //copySoundCommand += "Sound\"";
        copySoundCommand = RemoveNewLinesFromString(copySoundCommand);
        copySoundCommand = RemoveDoubleSlashFromString(copySoundCommand);

        //std::cout << copySoundCommand << std::endl;
        system(copySoundCommand.c_str());

        //  Extract all Sound Archives in given path
        for (const auto& archive : std::filesystem::directory_iterator(gamePath))
        {
            std::stringstream currentFileSS;

            currentFileSS << archive.path() << std::endl;

            std::string currentFile = currentFileSS.str();

            std::transform(currentFile.begin(), currentFile.end(), currentFile.begin(),
                [](unsigned char c) { return std::tolower(c); });

            if (currentFile.find("- sound", 0) != std::string::npos)
            {
                ExtractArchive(currentFile, AddQuotesToString(tempDirectoryPath));
            }
            else
            {
                //  File is not .bsa
            }
        }

        //  Convert files
        for (const auto& file : std::filesystem::recursive_directory_iterator(tempDirectoryPath))
        {
            std::stringstream currentFileSS;

            currentFileSS << file.path() << std::endl;

            std::string currentFile = currentFileSS.str();

            if (currentFile.find(".", 0) != std::string::npos)
            {
                ConvertAudioFile(currentFile);
            }
            else
            {
                //  Is not a file
            }
        }

        std::cout << "Cleaning up temporary directorys..." << std::endl;

        //  Remove any files that aren't WAV format just in case there are any left over
        for (const auto& file : std::filesystem::recursive_directory_iterator(tempDirectoryPath))
        {
            std::stringstream currentFileSS;

            currentFileSS << file.path() << std::endl;

            std::string currentFile = currentFileSS.str();

            if (currentFile.find(".", 0) != std::string::npos)
            {
                if (currentFile.find(".wav", 0) == std::string::npos)
                {
                    //  Delete File
                    DeleteAudioFile(currentFile);
                }
            }
        }

        //  Move folders from Temp Dir to Data Dir
        std::string musicMoveCommand = "Xcopy /q /e /y /i \"";
        musicMoveCommand += tempDirectoryPath;
        musicMoveCommand += "Music\" \"";
        musicMoveCommand += gamePath;
        //musicMoveCommand += "\"";
        musicMoveCommand += "Music\"";
        musicMoveCommand = RemoveNewLinesFromString(musicMoveCommand);
        musicMoveCommand = RemoveDoubleSlashFromString(musicMoveCommand);

        std::string soundMoveCommand = "Xcopy /q /e /y /i \"";
        soundMoveCommand += tempDirectoryPath;
        soundMoveCommand += "Sound\" \"";
        soundMoveCommand += gamePath;
        //soundMoveCommand += "\"";
        soundMoveCommand += "Sound\"";
        soundMoveCommand = RemoveNewLinesFromString(soundMoveCommand);
        soundMoveCommand = RemoveDoubleSlashFromString(soundMoveCommand);

        //std::cout << musicMoveCommand << std::endl;
        //std::cout << soundMoveCommand << std::endl;

        system(musicMoveCommand.c_str());
        system(soundMoveCommand.c_str());

        //  Remove Temp Dir
        std::string rmdirCommand = "rmdir /s /q \"";
        rmdirCommand += tempDirectoryPath;
        rmdirCommand += "\"";
        system(rmdirCommand.c_str());
    }

    if (game != 4)
    {
        std::cout << "\n";
        std::cout << "Process Complete!" << std::endl;
        system("pause");
    }
}