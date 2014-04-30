#ifndef BTSXTTHREAD_H
#define BTSXTTHREAD_H

#include <QThread>

namespace boost { namespace program_options { class variables_map; } }
namespace bts { namespace rpc { class rpc_server; } }

class BtsXtThread : public QThread
{
    Q_OBJECT
    const boost::program_options::variables_map* _p_option_variables;
    bts::rpc::rpc_server* p_rpc_server;
    bool _cancel;
    
public:
    BtsXtThread(const boost::program_options::variables_map* p_option_variables)
    : QThread(0), _p_option_variables(p_option_variables), _cancel(false)
    {
    }
    
    void run() Q_DECL_OVERRIDE ;
    void cancel(){ _cancel = true; }

signals:
    void resultReady(const QString &s);
};


#endif