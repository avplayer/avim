
#include "avconnection.hpp"

AVConnection::AVConnection()
{
    qRegisterMetaType<std::shared_ptr<RSA>>("std::shared_ptr<RSA>");
    qRegisterMetaType<std::shared_ptr<X509>>("std::shared_ptr<X509>");
}

void AVConnection::set_cert_and_key(std::shared_ptr<RSA> key, std::shared_ptr<X509> cert)
{

}
