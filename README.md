# BSc project: Starcraft Brood War bot
In this project we will make a bot for Starcraft: Brood War. woop woop

## Initial setup
Follow the steps below to setup SC:BW and BWAPI on your computer so that you can run a simple bot.

1. To install BWAPI, download and run [BWAPI_Setup.exe](https://github.com/bwapi/bwapi/releases). Locate the BWAPI installation folder (probably `C:\Users\%user%\BWAPI\`).

2. Download [Starcraft: Brood War v1.16.1](www.cs.mun.ca/~dchurchill/starcraftaicomp/files/Starcraft_1161.zip) and unzip the contents directly to the folder `.\BWAPI\Starcraft\`. Open the directory "bwapi-data" and create an empty folder called "AI" - this is where our bot will go.

3. Install Visual Studio and the C++ related packages, as well as "C++ Windows XP Support for VS 2017 (v141) tools \[Deprecated\]" which can be found under "Individual components".

4. Open `.\BWAPI\ExampleAIModule\ExampleAIModule.vcxproj` in Visual Studio and build the "ExampleAIModule" solution. Then, go to `.\BWAPI\Release\` and copy the file `ExampleAIModule.dll` (our bot!) to `.\BWAPI\Starcraft\bwapi-data\AI\`.

5. Run ChaosLauncher as administrator. Tick off "BWAPI 4.4.0 Injector \[RELEASE\]", then go to Settings and set "Installpath" to the game's installation path (`C:\Users\%user%\BWAPI\Starcraft\`).

6. Click the Start button in ChaosLauncher. This should load SC:BW. Now navigate through the menus to start a single player custom game. When the game starts, the AI should immediately start doing its thing (this AI automatically sends its workers to mine and builds new workers, but not much else). If it says 'AI module failed to load' or nothing happens, then something has gone wrong.
