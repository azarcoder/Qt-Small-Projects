Here is a step-by-step guide to downloading, installing, and integrating Qwt with the Qt Designer (UI Editor) on Windows 11.

### 1. Download Qwt

1. Go to the [Qwt SourceForge page](https://sourceforge.net/projects/qwt/files/qwt/).
2. Download the latest version (e.g., `qwt-6.3.0.zip` or similar).
3. Extract the zip file to a simple location with no spaces in the path (e.g., `C:\Qwt-6.3.0`).

### 2. Build Qwt (Crucial Step)

To use Qwt in Qt Designer, you must compile it. The most important rule is: **The build configuration of the Qwt plugin must match the build configuration of Qt Designer.**

* Standard Qt installations use **MSVC** (Microsoft Visual C++) in **Release** mode.
* Do **not** use MinGW unless your entire Qt setup is explicitly MinGW.

**Steps:**

1. Open **Qt Creator**.
2. Go to **File > Open File or Project** and select `qwt.pro` from the extracted folder.
3. Configure the project:
* Select the Kit that matches your Qt version (e.g., `Desktop Qt 6.x.x MSVC2019 64bit`).
* **Select "Release" build.** (Plugins built in Debug mode usually will *not* show up in Qt Designer).


4. Right-click the main `qwt` folder in the project tree and select **Build**.
* *Note:* This compiles the library and the designer plugin.



### 3. Install Qwt

Once the build is successful:

1. In Qt Creator, switch to the **Projects** tab (left sidebar).
2. Under "Build Steps", verify there is a "Make install" step, or simply run the install command manually.
* *Alternative (Manual):* If you don't see an install step, the build artifacts are usually located in a folder named `build-qwt-Desktop_Qt_...-Release`.


3. By default, Qwt installs itself to `C:\Qwt-6.x.x`.

### 4. Integrate with Qt Designer (The UI Editor)

You need to copy the plugin file you just built into the directory where Qt Designer looks for plugins.

1. **Find the Qwt Plugin:**
* Go to your Qwt build directory (e.g., `C:\Qwt-6.3.0\plugins\designer` OR inside the `build-...-Release` folder).
* Look for `qwt_designer_plugin.dll`.


2. **Find the Qt Designer Plugin Folder:**
* Navigate to your Qt installation directory. It usually looks like this:
`C:\Qt\[Qt_Version]\[Compiler]\plugins\designer`
* *Example:* `C:\Qt\6.8.0\msvc2019_64\plugins\designer`


3. **Copy and Paste:**
* Paste `qwt_designer_plugin.dll` into that folder.


4. **Copy the Qwt Libraries (DLLs):**
* Qt Designer also needs to load the core Qwt DLLs to run the plugin.
* Go to your Qwt lib folder (e.g., `C:\Qwt-6.3.0\lib`).
* Copy `qwt.dll` (and `qwtmathml.dll` if present).
* Paste them into the Qt bin folder: `C:\Qt\[Qt_Version]\[Compiler]\bin`.



### 5. Verification

1. Restart **Qt Creator** or open **Qt Designer** standalone.
2. Create a new Form (e.g., Main Window).
3. Look at the "Widget Box" (the left palette).
4. Scroll to the bottom. You should see a new section called **Qwt Widgets**.
5. Drag the **QwtKnob** onto your form.

### 6. Summary Check

If the `QwtKnob` does not appear:

* **Version Mismatch:** Did you build Qwt with MSVC 2019 but your Qt Designer is from MSVC 2022? They must match.
* **Mode Mismatch:** Did you build in **Debug**? Rebuild in **Release**.

# .pro file configuration
# --- QWT CONFIGURATION ---
# Your Qwt root folder
QWT_ROOT = C:/qwt-6.1.4

# 1. Point to 'src' because that is where your .h files are
INCLUDEPATH += $$QWT_ROOT/src

# 2. Point to 'lib' for the library files
# (The lib folder is usually next to the src folder)
LIBS += -L$$QWT_ROOT/lib -lqwt

# If you are using Qwt MathML (optional, usually not needed for just a knob)
# LIBS += -L$$QWT_LOCATION/lib -lqwtmathml
