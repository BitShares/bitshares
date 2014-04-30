#include "html5viewer/html5viewer.h"
#include "bts_xt_thread.h"

#include <boost/program_options.hpp>
#include <boost/thread.hpp>
#include <fc/thread/thread.hpp>

#include <QApplication>


int main( int argc, char** argv )
{
    // parse command-line options
    boost::program_options::options_description option_config("Allowed options");
    option_config.add_options()("data-dir", boost::program_options::value<std::string>(), "configuration data directory")
    ("help", "display this help message")
    ("p2p", "enable p2p mode")
    ("port", boost::program_options::value<uint16_t>(), "set port to listen on")
    ("connect-to", boost::program_options::value<std::string>(), "set remote host to connect to")
    ("trustee-private-key", boost::program_options::value<std::string>(), "act as a trustee using the given private key")
    ("trustee-address", boost::program_options::value<std::string>(), "trust the given BTS address to generate blocks")
    ("genesis-json", boost::program_options::value<std::string>(), "generate a genesis block with the given json file (only for testing, only accepted when the blockchain is empty)");
    
    boost::program_options::positional_options_description positional_config;
    positional_config.add("data-dir", 1);
    
    boost::program_options::variables_map option_variables;
    try
    {
        boost::program_options::store(boost::program_options::command_line_parser(argc, argv).
                                      options(option_config).positional(positional_config).run(), option_variables);
        boost::program_options::notify(option_variables);
    }
    catch (boost::program_options::error&)
    {
        std::cerr << "Error parsing command-line options\n\n";
        std::cerr << option_config << "\n";
        return 1;
    }
    
    if (option_variables.count("help"))
    {
        std::cout << option_config << "\n";
        return 0;
    }
    
    BtsXtThread btsxt(&option_variables);
    btsxt.start();
        
    QApplication app(argc, argv);
    Html5Viewer viewer;
    viewer.setOrientation(Html5Viewer::ScreenOrientationAuto);
    viewer.resize(800,600);
    viewer.show();
    QUrl url = QUrl("http://127.0.0.1:9989");
    url.setUserName("user");
    url.setPassword("password");
    viewer.loadUrl(url);
    app.exec();    

    btsxt.cancel();
    btsxt.wait();    
    
    return 1;
}

