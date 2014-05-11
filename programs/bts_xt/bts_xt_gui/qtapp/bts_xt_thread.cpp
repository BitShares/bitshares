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
#include <fc/log/logger.hpp>
#include <fc/io/json.hpp>
#include <fc/reflect/variant.hpp>

#include <iostream>

#include <fc/network/http/server.hpp>
#include <fc/network/tcp_socket.hpp>
#include <fc/network/ip.hpp>

#include "bts_xt_thread.h"

struct config
{
    config():ignore_console(false){}
    bts::rpc::rpc_server::config rpc;
    bool ignore_console;
};
FC_REFLECT( config, (rpc)(ignore_console) )


void try_open_wallet(bts::rpc::rpc_server_ptr);
void configure_logging(const fc::path&);
config   load_config( const fc::path& datadir );
bts::blockchain::chain_database_ptr load_and_configure_chain_database(const fc::path& datadir, 
                                                                      const boost::program_options::variables_map& option_variables);


void BtsXtThread::run() {

    try {
        bool p2p_mode = _option_variables.count("p2p") != 0;
        
        ::configure_logging(_datadir);
                
        auto cfg   = load_config(_datadir);
        auto chain = load_and_configure_chain_database(_datadir, _option_variables);
        auto wall  = std::make_shared<bts::wallet::wallet>();
        wall->set_data_directory( _datadir );
        
        auto c = std::make_shared<bts::client::client>(p2p_mode);
        c->set_chain( chain );
        c->set_wallet( wall );
        
        if (_option_variables.count("trustee-private-key"))
        {
            auto key = fc::variant(_option_variables["trustee-private-key"].as<std::string>()).as<fc::ecc::private_key>();
            c->run_trustee(key);
        }
        else if( fc::exists( "trustee.key" ) )
        {
            auto key = fc::json::from_file( "trustee.key" ).as<fc::ecc::private_key>();
            c->run_trustee(key);
        }
        
        bts::rpc::rpc_server_ptr rpc_server = std::make_shared<bts::rpc::rpc_server>();
        rpc_server->set_client(c);
        
        std::cout << "starting rpc server..\n";
        fc_ilog( fc::logger::get("rpc"), "starting rpc server..");
        bts::rpc::rpc_server::config rpc_config(cfg.rpc);
        if (_option_variables.count("rpcuser"))
            rpc_config.rpc_user = _option_variables["rpcuser"].as<std::string>();
        if (_option_variables.count("rpcpassword"))
            rpc_config.rpc_password = _option_variables["rpcpassword"].as<std::string>();
        // for now, force binding to localhost only
        if (_option_variables.count("rpcport"))
            rpc_config.rpc_endpoint = fc::ip::endpoint(fc::ip::address("127.0.0.1"), _option_variables["rpcport"].as<uint16_t>());
        if (_option_variables.count("httpport"))
            rpc_config.httpd_endpoint = fc::ip::endpoint(fc::ip::address("127.0.0.1"), _option_variables["httpport"].as<uint16_t>());
        try_open_wallet(rpc_server); // assuming password is blank
        rpc_server->configure(rpc_config);
                
        if (p2p_mode)
        {
            c->configure( _datadir );
            if (_option_variables.count("port"))
                c->listen_on_port(_option_variables["port"].as<uint16_t>());
            c->connect_to_p2p_network();
            if (_option_variables.count("connect-to"))
                c->connect_to_peer(_option_variables["connect-to"].as<std::string>());
        }
        else
            c->add_node( "127.0.0.1:4569" );
        
        while(!_cancel) fc::usleep(fc::microseconds(10000));
        
        wall->save();
    } 
    catch ( const fc::exception& e ) 
    {
        wlog( "${e}", ("e", e.to_detail_string() ) );
    }

}

void try_open_wallet(bts::rpc::rpc_server_ptr rpc_server) {
    try
    {
        // try to open without a passphrase first
        rpc_server->direct_invoke_method("open_wallet", fc::variants());
        return;
    }
    catch (bts::rpc::rpc_wallet_passphrase_incorrect_exception&)
    {
    }
    catch (const fc::exception& e)
    {
    }
    catch (...)
    {
    }
}

void configure_logging(const fc::path& data_dir)
{
    fc::logging_config cfg;
    
    fc::file_appender::config ac;
    ac.filename = data_dir / "log.txt";
    ac.truncate = false;
    ac.flush    = true;
    
    fc::file_appender::config ac_rpc;
    ac_rpc.filename = data_dir / "rpc.log";
    ac_rpc.truncate = false;
    ac_rpc.flush    = true;
    ac_rpc.format   = "${message}";
    
    cfg.appenders.push_back(fc::appender_config( "default", "file", fc::variant(ac)));
    cfg.appenders.push_back(fc::appender_config( "rpc", "file", fc::variant(ac_rpc)));
    
    fc::logger_config dlc;
    dlc.level = fc::log_level::debug;
    dlc.name = "default";
    dlc.appenders.push_back("default");
    
    fc::logger_config dlc_rpc;
    dlc_rpc.level = fc::log_level::debug;
    dlc_rpc.name = "rpc";
    dlc_rpc.appenders.push_back("rpc");
    
    cfg.loggers.push_back(dlc);
    cfg.loggers.push_back(dlc_rpc);
    
    fc::configure_logging( cfg );
}

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
                genesis_block = bts::net::create_genesis_block(genesis_json_file);
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
