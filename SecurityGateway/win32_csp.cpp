#include <windows.h>
#include <windef.h>

#include <wincrypt.h>

#include <Cryptuiapi.h>

#include <stdio.h>
#include <string.h>

/*
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/bio.h>
*/
//#pragma comment(lib,"Crypt32.lib")		//CSP
//#pragma comment(lib,"cryptui.lib")	   //证书相关UI
#include <QDebug>

//int use_cryptoAPI_cert(char *data,char *cert_prop);

static const CERT_CONTEXT *find_certificate_in_store(const char *cert_prop, HCERTSTORE cert_store)
{
    /* Find, and use, the desired certificate from the store. The
     * 'cert_prop' certificate search string can look like this:
     * SUBJ:<certificate substring to match>
     * THUMB:<certificate thumbprint hex value>, e.g.
     *     THUMB:f6 49 24 41 01 b4 fb 44 0c ce f4 36 ae d0 c4 c9 df 7a b6 28
     */
    const CERT_CONTEXT *rv = NULL;

    if (!strncmp(cert_prop, "SUBJ:", 5)) {
	/* skip the tag */
    cert_prop += 5;

    /*DER : andy*/
    rv = CertFindCertificateInStore(cert_store, X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
        0, CERT_FIND_SUBJECT_STR_A, cert_prop, NULL);

    /*PEM : andy didn't work*/
    //rv = CertFindCertificateInStore(cert_store, X509_NDR_ENCODING | PKCS_7_NDR_ENCODING,
    //    0, CERT_FIND_SUBJECT_STR_A, cert_prop, NULL);

    } else if (!strncmp(cert_prop, "THUMB:", 6)) {
	unsigned char hash[255];
	char *p;
	int i, x = 0;
	CRYPT_HASH_BLOB blob;

	/* skip the tag */
	cert_prop += 6;
	for (p = (char *) cert_prop, i = 0; *p && i < sizeof(hash); i++) {
	    if (*p >= '0' && *p <= '9')
		x = (*p - '0') << 4;
	    else if (*p >= 'A' && *p <= 'F')
		x = (*p - 'A' + 10) << 4;
	    else if (*p >= 'a' && *p <= 'f')
		x = (*p - 'a' + 10) << 4;
	    if (!*++p)	/* unexpected end of string */
		break;
	    if (*p >= '0' && *p <= '9')
		x += *p - '0';
	    else if (*p >= 'A' && *p <= 'F')
		x += *p - 'A' + 10;
	    else if (*p >= 'a' && *p <= 'f')
		x += *p - 'a' + 10;
	    hash[i] = x;
	    /* skip any space(s) between hex numbers */
	    for (p++; *p && *p == ' '; p++);
	}
	blob.cbData = i;
	blob.pbData = (unsigned char *) &hash;

    /*DER : andy*/
    rv = CertFindCertificateInStore(cert_store, X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
        0, CERT_FIND_HASH, &blob, NULL);

    /*PEM : andy didn't work*/
    //rv = CertFindCertificateInStore(cert_store, X509_NDR_ENCODING | PKCS_7_NDR_ENCODING,
     //   0, CERT_FIND_HASH, &blob, NULL);

    }

    return rv;
}


//free it outside
int use_cryptoAPI_cert(char **data,char *cert_prop)
{
    HCERTSTORE	cs;

	cs = CertOpenStore((LPCSTR) CERT_STORE_PROV_SYSTEM, 0, 0, CERT_SYSTEM_STORE_CURRENT_USER | CERT_STORE_OPEN_EXISTING_FLAG | CERT_STORE_READONLY_FLAG, L"MY");
    qDebug() << "Open Store Ok" ;
	if(cs == NULL)
	{
//        goto err;
        return 0;
	}
	const CERT_CONTEXT *cert_context = find_certificate_in_store(cert_prop,cs);
    qDebug() << "Find Cert Ok" ;

	CertCloseStore(cs,0);

	if(!cert_context)
	{
        qDebug() << "Reopen Store " << endl;
        cs = CertOpenStore((LPCSTR) CERT_STORE_PROV_SYSTEM, 0, 0, CERT_SYSTEM_STORE_LOCAL_MACHINE | CERT_STORE_OPEN_EXISTING_FLAG | CERT_STORE_READONLY_FLAG, L"MY");
		if(cs == NULL)
		{
//            goto err;
            return 0;
		}

		cert_context = find_certificate_in_store(cert_prop,cs);
		CertCloseStore(cs,0);
        qDebug() << "Reopen Store success " << endl;
    }

	if(cert_context == NULL)
	{
//        goto err;
        return 0;
	}

    qDebug("%d",cert_context->cbCertEncoded);
    //要多分配一个字节，不然部分memcpy的时候会出错？？
    *data = calloc(cert_context->cbCertEncoded +1,sizeof(char));
    //*data = malloc(cert_context->cbCertEncoded +1);
    //memset(*data,0,cert_context->cbCertEncoded+1);

    if(*data == NULL)
        return 0;

    qDebug() << "Malloc OK";
    memcpy(*data,cert_context->pbCertEncoded,cert_context->cbCertEncoded);

    //add for x509 to pem test
    //FILE *fp = NULL;
    //fp = fopen("test.pem","w+");

    /*
     * X509 *cert = NULL;
    cert = d2i_X509(NULL,(const unsigned char **)cert_context->pbCertEncoded,cert_context->cbCertEncoded);
    //PEM_write_X509(fp,cert);
    //fclose(fp);
    BIO *out = NULL;
    out = BIO_new_file("test.pem","w");

    //BIO_write_filename(out,"test.pem");
    PEM_write_bio_X509(out,cert);
    BI_free(out);
    */
    return cert_context->cbCertEncoded;



//err:
    return 0;
}

//free it out side

int get_cryptoAPI_cert(char *data, char *cert_prop)
{
/*
    HCERTSTORE	cs;

    cs = CertOpenStore((LPCSTR) CERT_STORE_PROV_SYSTEM, 0, 0, CERT_SYSTEM_STORE_CURRENT_USER | CERT_STORE_OPEN_EXISTING_FLAG | CERT_STORE_READONLY_FLAG, L"MY");
    if(cs == NULL)
    {
        goto err;
        return 0;
    }
    //const CERT_CONTEXT *cert_context = find_certificate_in_store(cert_prop,cs);
    CERT_CONTEXT *cert_context = find_certificate_in_store(cert_prop,cs);
    CertCloseStore(cs,0);

    if(!cert_context)
    {
        cs = CertOpenStore((LPCSTR) CERT_STORE_PROV_SYSTEM, 0, 0, CERT_SYSTEM_STORE_LOCAL_MACHINE | CERT_STORE_OPEN_EXISTING_FLAG | CERT_STORE_READONLY_FLAG, L"MY");
        if(cs == NULL)
        {
            goto err;
            return 0;
        }

        cert_context = find_certificate_in_store(cert_prop,cs);
        CertCloseStore(cs,0);
    }

    if(cert_context == NULL)
    {
        return 0;
        goto err;
    }
    //malloc memory

    //data = malloc(cert_context->cbCertEncoded);
    if(data = NULL)
    {
        //goto err
        return 0;
    }

    //没有做数据清理
    return 1;
err:
    return 0;
*/
}
