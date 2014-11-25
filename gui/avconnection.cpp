
#include "avconnection.hpp"

AVConnection::AVConnection()
{
    qRegisterMetaType<std::shared_ptr<RSA>>("std::shared_ptr<RSA>");
    qRegisterMetaType<std::shared_ptr<X509>>("std::shared_ptr<X509>");
    qRegisterMetaType<ConState>("AVConnection::ConState");
}

void AVConnection::set_cert_and_key(std::shared_ptr<RSA> key, std::shared_ptr<X509> cert)
{

}

void AVConnection::start()
{

}

void AVConnection::stop()
{

}
