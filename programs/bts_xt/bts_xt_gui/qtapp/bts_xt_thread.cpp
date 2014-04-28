#include <boost/program_options.hpp>
#include <boost/thread.hpp>

#include <bts/net/chain_server.hpp>
#include <bts/client/client.hpp>
#include <bts/blockchain/chain_database.hpp>
#include <bts/wallet/wallet.hpp>
#include <bts/rpc/rpc_server.hpp>
#include <bts/cli/cli.hpp>
#include <fc/filesystem.hpp>
#include <fc/thread/thread.hpp>
#include <fc/log/file_appender.hpp>
#include <fc/log/logger_config.hpp>
#include <fc/io/json.hpp>
#include <fc/reflect/variant.hpp>

#include <iostream>

#include "bts_xt_thread.h"

struct config
{
    config():ignore_console(false){}
    bts::rpc::rpc_server::config rpc;
    bool ignore_console;
};
FC_REFLECT( config, (rpc)(ignore_console) )


void configure_logging(const fc::path&);
fc::path get_data_dir(const boost::program_options::variables_map& option_variables);
config   load_config( const fc::path& datadir );
bts::blockchain::chain_database_ptr load_and_configure_chain_database(const fc::path& datadir, 
                                                                      const boost::program_options::variables_map& option_variables);

BtsXtThread::~BtsXtThread(){
    delete p_rpc_server;
}

void BtsXtThread::run() {
    const boost::program_options::variables_map& option_variables = *_p_option_variables;
    try {
        bool p2p_mode = option_variables.count("p2p") != 0;
        
        fc::path datadir = get_data_dir(option_variables);
        ::configure_logging(datadir);
        
        auto cfg   = load_config(datadir);
        auto chain = load_and_configure_chain_database(datadir, option_variables);
        auto wall  = std::make_shared<bts::wallet::wallet>();
        wall->set_data_directory( datadir );
        
        auto c = std::make_shared<bts::client::client>(p2p_mode);
        c->set_chain( chain );
        c->set_wallet( wall );
        
        if (option_variables.count("trustee-private-key"))
        {
            auto key = fc::variant(option_variables["trustee-private-key"].as<std::string>()).as<fc::ecc::private_key>();
            c->run_trustee(key);
        }
        else if( fc::exists( "trustee.key" ) )
        {
            auto key = fc::json::from_file( "trustee.key" ).as<fc::ecc::private_key>();
            c->run_trustee(key);
        }
        
        //bts::rpc::rpc_server_ptr rpc_server = std::make_shared<bts::rpc::rpc_server>();
        p_rpc_server = new bts::rpc::rpc_server();
        p_rpc_server->set_client(c);
        
        std::cout << ".... starting rpc server ...\n";
        bts::rpc::rpc_server::config rpc_config(cfg.rpc);
        rpc_config.rpc_user = "guiuser";
        rpc_config.rpc_password = "guiuserpass";
        rpc_config.rpc_endpoint = fc::ip::endpoint(fc::ip::address("127.0.0.1"), 9980);
        rpc_config.httpd_endpoint = fc::ip::endpoint(fc::ip::address("127.0.0.1"), 9989);
        p_rpc_server->configure(rpc_config);
                
        if (p2p_mode)
        {
            c->configure( datadir );
            if (option_variables.count("port"))
                c->listen_on_port(option_variables["port"].as<uint16_t>());
            c->connect_to_p2p_network();
            if (option_variables.count("connect-to"))
                c->connect_to_peer(option_variables["connect-to"].as<std::string>());
        }
        else
            c->add_node( "127.0.0.1:4569" );
        
    } 
    catch ( const fc::exception& e ) 
    {
        wlog( "${e}", ("e", e.to_detail_string() ) );
    }
    
}


void configure_logging(const fc::path& data_dir)
{
    fc::file_appender::config ac;
    ac.filename = data_dir / "log.txt";
    ac.truncate = false;
    ac.flush    = true;
    fc::logging_config cfg;
    
    cfg.appenders.push_back(fc::appender_config( "default", "file", fc::variant(ac)));
    
    fc::logger_config dlc;
    dlc.level = fc::log_level::debug;
    dlc.name = "default";
    dlc.appenders.push_back("default");
    cfg.loggers.push_back(dlc);
    fc::configure_logging( cfg );
}


fc::path get_data_dir(const boost::program_options::variables_map& option_variables)
{ try {
    fc::path datadir;
    if (option_variables.count("data-dir"))
    {
        datadir = fc::path(option_variables["data-dir"].as<std::string>().c_str());
    }
    else
    {
#ifdef WIN32
        datadir =  fc::app_path() / "BitSharesXT";
#elif defined( __APPLE__ )
        datadir =  fc::app_path() / "BitSharesXT";
#else
        datadir = fc::app_path() / ".bitsharesxt";
#endif
    }
    return datadir;
    
} FC_RETHROW_EXCEPTIONS( warn, "error loading config" ) }

bts::blockchain::chain_database_ptr load_and_configure_chain_database(const fc::path& datadir, 
                                                                      const boost::program_options::variables_map& option_variables)
{
    bts::blockchain::chain_database_ptr chain = std::make_shared<bts::blockchain::chain_database>();
    chain->open( datadir / "chain", true );
    if (option_variables.count("trustee-address"))
        chain->set_trustee(bts::blockchain::address(option_variables["trustee-address"].as<std::string>()));
    else
        chain->set_trustee(bts::blockchain::address("43cgLS17F2uWJKKFbPoJnnoMSacj"));
    if (option_variables.count("genesis-json"))
    {
        if (chain->head_block_num() == uint32_t(-1))
        {
            fc::path genesis_json_file(option_variables["genesis-json"].as<std::string>());
            bts::blockchain::trx_block genesis_block;
            try
            {
                genesis_block = bts::net::create_test_genesis_block(genesis_json_file);
            }
            catch (fc::exception& e)
            {
                wlog("Error creating genesis block from file ${filename}: ${e}", ("filename", genesis_json_file)("e", e.to_string()));
                return chain;
            }
            try 
            {
                chain->push_block(genesis_block);
            }
            catch ( const fc::exception& e )
            {
                wlog( "error pushing genesis block to blockchain database: ${e}", ("e", e.to_detail_string() ) );
            }
        }
        else
            wlog("Ignoring genesis-json command-line argument because our blockchain already has a genesis block");
    }
    return chain;
}

config load_config( const fc::path& datadir )
{ try {
    auto config_file = datadir/"config.json";
    config cfg;
    if( fc::exists( config_file ) )
    {
        cfg = fc::json::from_file( config_file ).as<config>();
    }
    else
    {
        std::cerr<<"creating default config file "<<config_file.generic_string()<<"\n";
        fc::json::save_to_file( cfg, config_file );
    }
    return cfg;
} FC_RETHROW_EXCEPTIONS( warn, "unable to load config file ${cfg}", ("cfg",datadir/"config.json")) }
