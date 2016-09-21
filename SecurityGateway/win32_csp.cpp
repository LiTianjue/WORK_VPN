#include <windows.h>
#include <windef.h>

#include <wincrypt.h>

#include <Cryptuiapi.h>

#include <stdio.h>
#include <string.h>


//#pragma comment(lib,"Crypt32.lib")		//CSP
//#pragma comment(lib,"cryptui.lib")	   //证书相关UI
#include <QDebug>

//int use_cryptoAPI_cert(char *data,char *cert_prop);

static int CryptoAPI_SetPin(const char *cert_prop,char *pin);
typedef struct _CAPI_DATA {
    const CERT_CONTEXT *cert_context;
    HCRYPTPROV crypt_prov;
    DWORD key_spec;
    BOOL free_crypt_prov;
} CAPI_DATA;





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
}





/************************************************/
int use_cryptoAPI_cert_with_pin(char **data,char *cert_prop,int check)
{
    int ret = 0;

    HCERTSTORE cs;
    CAPI_DATA *cd = calloc(1, sizeof(*cd));
    // [1]
    if (cd == NULL ) {
        ret = -1;
        goto err;
    }
    // [2]
    cs = CertOpenStore((LPCSTR) CERT_STORE_PROV_SYSTEM, 0, 0, CERT_SYSTEM_STORE_CURRENT_USER |
                       CERT_STORE_OPEN_EXISTING_FLAG | CERT_STORE_READONLY_FLAG, L"MY");
    if (cs == NULL) {
        ret = -2;
        goto err;
    }
    // [3]
    cd->cert_context = find_certificate_in_store(cert_prop, cs);
    CertCloseStore(cs, 0);
    if (!cd->cert_context) {
        cs = CertOpenStore((LPCSTR) CERT_STORE_PROV_SYSTEM, 0, 0, CERT_SYSTEM_STORE_LOCAL_MACHINE |
                           CERT_STORE_OPEN_EXISTING_FLAG | CERT_STORE_READONLY_FLAG, L"MY");
        if (cs == NULL) {
            ret = -3;
            goto err;
        }
        // [4]
        cd->cert_context = find_certificate_in_store(cert_prop, cs);
        CertCloseStore(cs, 0);
        if (cd->cert_context == NULL) {
            ret = -4;
            goto err;
        }
    }

    /* -------------- read cert success ------------------- */
    // at this moment , we don't need to encode
    /*
    cert = d2i_X509(NULL, (const unsigned char **) &cd->cert_context->pbCertEncoded,
            cd->cert_context->cbCertEncoded);
    if (cert == NULL) {
    //SSLerr(SSL_F_SSL_CTX_USE_CERTIFICATE_FILE, ERR_R_ASN1_LIB);
    goto err;
    }
    */

    // [5]
    /* set up stuff to use the private key */
    if (!CryptAcquireCertificatePrivateKey(cd->cert_context, CRYPT_ACQUIRE_COMPARE_KEY_FLAG,
                                           NULL, &cd->crypt_prov, &cd->key_spec, &cd->free_crypt_prov)) {
        ret = -5;
        goto err;
    }


    // [6]
    qDebug("%d",cd->cert_context->cbCertEncoded);
    //要多分配一个字节，不然部分memcpy的时候会出错？？
    *data = calloc(cd->cert_context->cbCertEncoded +1,sizeof(char));
    //*data = malloc(cert_context->cbCertEncoded +1);
    //memset(*data,0,cert_context->cbCertEncoded+1);

    if(*data == NULL)
    {
        ret = -6;
        goto err;
    }

    qDebug() << "Malloc OK";
    memcpy(*data,cd->cert_context->pbCertEncoded,cd->cert_context->cbCertEncoded);

    ret = cd->cert_context->cbCertEncoded;





#if 1   /*ask for password */
    if(check == 1)
    {
        int ok = -1;
        unsigned char from[36] = "CertAcquireCertificatePrivateKey";

        HCRYPTHASH hash;
        unsigned char *buf;
        DWORD len , hash_size;
        len = sizeof(hash_size);
        if(!CryptCreateHash(cd->crypt_prov,CALG_SSL3_SHAMD5,0,0,&hash))
        {
            qDebug("Create hash  Error.\n");
        }else if(!CryptGetHashParam(hash,HP_HASHSIZE,(BYTE *)&hash_size,&len,0))
        {

            qDebug("Get Hash size Error.\n");
        }else if(!CryptSetHashParam(hash,HP_HASHVAL,(BYTE *)from,0))
        {
            qDebug("Set Hash Value Error.\n");
        }else{

            len = 256;
            buf = malloc(256);
            memset(buf,0,len);

            if(!CryptSignHash(hash,cd->key_spec,NULL,0,buf,&len)) {
                qDebug("Error Hash.\n");
                ret = -7;
            }else
                ok = 0;

            free(buf);
            CryptDestroyHash(hash);
        }

        if(ok != 0)
            free(*data);
    }

#endif

err:
    if (cd) {
        if (cd->free_crypt_prov && cd->crypt_prov)
            CryptReleaseContext(cd->crypt_prov, 0);
        if (cd->cert_context)
            CertFreeCertificateContext(cd->cert_context);
        free(cd);
    }
    return ret;
}

/**********************************************************/
