#ifndef WIN32_CSP_H
#define WIN32_CSP_H

int use_cryptoAPI_cert(char **data,char *cert_prop);

int use_cryptoAPI_cert_with_pin(char **data,char *cert_prop,int check);

int get_cryptoAPI_cert(char *data,char *cert_prop);
#endif // WIN32_CSP_H

