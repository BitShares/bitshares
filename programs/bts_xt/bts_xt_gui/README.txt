== HTML5 GUI + QT WebKit Conatainer ==

This project consists of two parts:
1. QT application in qtapp directory
2. HTML5 web application, located in webapp directory


-- Configuration and installation --


1. QT application:

Download and install QT5, see http://qt-project.org/downloads
Set envronment variable CMAKE_PREFIX_PATH to point to clang_64 in your QT directory, e.g.:
> export CMAKE_PREFIX_PATH=/Users/user/Qt5.2.1/5.2.1/clang_64

User CMake to configure bitshares_toolkit, set INCLUDE_GUI to TRUE or to ON, e.g.
> cmake -DINCLUDE_GUI=ON ../bitshares_toolkit
if there were no compilation errors, the executable should be located in programs/bts_xt/bts_xt_gui/qtapp/bts_xt_gui
you need to run it similar to bts_xt_client - it accepts the same command line parameters except rpc server related ones,
there is no need to specify rpc_user, rpc_user, rpc and http endpoints - the application uses hardcoded parameters.
Here is an example command:
> bts_xt_gui --data-dir ~/tmp/bts_xt_gui --trustee-address JShWneMdiV7kQcz9WHzJc3hEorUScpk24



2. Web application:

The app uses Lineman.js to manage development environment, run dev web server and compile assets on the fly,
Please read Lineman.js to get up and running, the installation is very simple, basically you need to install node.js
and install Lineman and dependencies via the following commands:
> npm install -g lineman  
> npm install

Then go to webapp directory and start lineman:
> lineman run
Open http://localhost:8000 and make sure the application is working

Please note, that web application requires (this is not implemented yet) bts_xt_gui or bts_xt_client to be running (bts_xt_client in server mode (--server) on port 9989)

