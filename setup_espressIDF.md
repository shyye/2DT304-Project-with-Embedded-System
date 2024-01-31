# Install Espress IDF for Windows
This is the manual/setup for Espress IDF (Espressif IoT Development Framework) for Windows which was used in this project.

## 1. Installation through IDE
Follow Espressif Get Started Guide and choose installation through IDE (in this case VSCode Extension).  

Link to full guide: [Get Started ESP32 ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/)  

![Image](images\setup_espressIDF\Install_EspressIDF_1-1_choose_VSCodeExtension.png)

Direct link to Installation through IDE: [VSCode Extension](https://github.com/espressif/vscode-esp-idf-extension/blob/master/docs/tutorial/install.md)  


## 2. If you choose setup option 'EXPRESS' in the tutorial above.
Install the latest version.  
![Image]()


## NOTE! MSys/Mingw Error
If you get this error:  
![Error]()

See one possible solution below.

### 1. Delete the /esp folder created by the installation through VSCode
If you want to reset and start over, you will create this folder again during the setup.

### 2. Download Toolchain Setup
Use this guide: [Standard Setup of Toolchain for Windows](https://docs.espressif.com/projects/esp-idf/en/v3.1.5/get-started/windows-setup.html)  
to download the Windows all-in-one toolchain & MSYS zip

#### Alternative 1:
- If you want to clone the repository yourself, follow through the full guide above.

#### Alternative 2 (my choice):
- Otherwise you can start the installation through Visual Studio Code again.
- ***NOTE! If you use another location than ``` C:\ ``` you need to add a new path before you run the installation again.*** Check step 3 below.

### 3. Add path for MSys (ONLY NEEDED FOR Alternative 2 ABOVE, otherwise skip this step)

#### 3.1 Find the path to your msys32 folder
Locate the msys32 folder (in the folder you downloaded in the previous step).  
Step into the msys32 folder and copy the path.  
![Image]()

#### 3.2 Edit Environmental Variables
Go to Windows -> Search -> Edit environment variables for your account  
or Windows -> Search -> Edit the system environment variables -> Click 'Environment Variables'

Click on 'New' for the 'User variables for YourAccountName'.   
![Image]()

Fill in the values.  
Variable name: A name of your choice, for example msys.  
Variable value: Paste your copied path here.  
![Image]()

#### 3.3 Restart your computer
(to get the Environment Variables to work)



