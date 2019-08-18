include ../../../config.mak

TARGET = $(LIB)libopenssl.a

CC = $(BIN)gcc
CC_OPTION = $(MAIN_FLAG) -march=r3000 -c -mno-abicalls -O2 -G 8 -I$(INC) -I../../../libSrc/lwip/include -I../../../libSrc/openssl/source -I../../../libSrc/openssl/source/crypto -I$(G_INC) -DBOOT_ROM_TYPE -Wa,-ahl=
#CC_OPTION = $(MAIN_FLAG) -march=r3000 -c -mno-abicalls -O0 -ggdb -G 8 -I$(INC) -I$(G_INC) -DBOOT_ROM_TYPE -Wa,-ahl=

AS = $(BIN)as
AS_OPTION = -march=r3000 -I$(INC) -I$(G_INC) -ahl=

LD = $(BIN)ld

AR = $(BIN)ar

SSL_OBJ = $(OBJ)SSL/
CRYPTO_OBJ = $(OBJ)CRYPTO/
CRYPTO_ERR_OBJ = $(OBJ)CRYPTO/ERR/
CRYPTO_ASN_OBJ = $(OBJ)CRYPTO/ASN/
CRYPTO_LHASH_OBJ = $(OBJ)CRYPTO/LHASH/
CRYPTO_STACK_OBJ = $(OBJ)CRYPTO/STACK/
CRYPTO_OBJECTS_OBJ = $(OBJ)CRYPTO/OBJECTS/
CRYPTO_X509_OBJ = $(OBJ)CRYPTO/X509/
CRYPTO_ASN1_OBJ = $(OBJ)CRYPTO/ASN1/
CRYPTO_BN_OBJ = $(OBJ)CRYPTO/BN/
CRYPTO_BUFFER_OBJ = $(OBJ)CRYPTO/BUFFER/
CRYPTO_DES_OBJ = $(OBJ)CRYPTO/DES/
CRYPTO_MD4_OBJ = $(OBJ)CRYPTO/MD4/
CRYPTO_MD5_OBJ = $(OBJ)CRYPTO/MD5/
CRYPTO_RAND_OBJ = $(OBJ)CRYPTO/RAND/
CRYPTO_BIO_OBJ = $(OBJ)CRYPTO/BIO/
CRYPTO_EVP_OBJ = $(OBJ)CRYPTO/EVP/
CRYPTO_RSA_OBJ = $(OBJ)CRYPTO/RSA/
CRYPTO_SHA_OBJ = $(OBJ)CRYPTO/SHA/
CRYPTO_EC_OBJ = $(OBJ)CRYPTO/EC/
CRYPTO_DH_OBJ = $(OBJ)CRYPTO/DH/
CRYPTO_DSA_OBJ = $(OBJ)CRYPTO/DSA/
CRYPTO_ECDSA_OBJ = $(OBJ)CRYPTO/ECDSA/
CRYPTO_PKCS7_OBJ = $(OBJ)CRYPTO/PKCS7/
CRYPTO_PKCS12_OBJ = $(OBJ)CRYPTO/PKCS12/
CRYPTO_DSO_OBJ = $(OBJ)CRYPTO/DSO/
CRYPTO_X509V3_OBJ = $(OBJ)CRYPTO/X509V3/
CRYPTO_PEM_OBJ = $(OBJ)CRYPTO/PEM/
CRYPTO_HMAC_OBJ = $(OBJ)CRYPTO/HMAC/
CRYPTO_COMP_OBJ = $(OBJ)CRYPTO/COMP/
CRYPTO_OCSP_OBJ = $(OBJ)CRYPTO/OCSP/
CRYPTO_CONF_OBJ = $(OBJ)CRYPTO/CONF/
CRYPTO_ECDH_OBJ = $(OBJ)CRYPTO/ECDH/
CRYPTO_RC4_OBJ = $(OBJ)CRYPTO/RC4/

S_OBJS =

C_OBJS =  $(SSL_OBJ)ssl_lib.o $(SSL_OBJ)ssl_algs.o $(SSL_OBJ)ssl_err2.o $(SSL_OBJ)ssl_err.o \
          $(SSL_OBJ)ssl_ciph.o \
          $(SSL_OBJ)ssl_sess.o \
          $(SSL_OBJ)ssl_cert.o \
          $(SSL_OBJ)ssl_rsa.o \
          $(SSL_OBJ)s23_clnt.o $(SSL_OBJ)s23_lib.o $(SSL_OBJ)s23_meth.o $(SSL_OBJ)s23_pkt.o $(SSL_OBJ)s23_srvr.o \
          $(SSL_OBJ)s2_clnt.o $(SSL_OBJ)s2_enc.o $(SSL_OBJ)s2_lib.o $(SSL_OBJ)s2_meth.o $(SSL_OBJ)s2_pkt.o \
          $(SSL_OBJ)s2_srvr.o $(SSL_OBJ)s3_both.o $(SSL_OBJ)s3_clnt.o $(SSL_OBJ)s3_enc.o $(SSL_OBJ)s3_lib.o \
          $(SSL_OBJ)s3_meth.o $(SSL_OBJ)s3_pkt.o $(SSL_OBJ)s3_srvr.o \
          $(SSL_OBJ)t1_clnt.o \
          $(SSL_OBJ)t1_enc.o \
          $(SSL_OBJ)t1_lib.o \
          $(SSL_OBJ)t1_meth.o \
          $(SSL_OBJ)t1_srvr.o \
          $(CRYPTO_OBJ)o_str.o $(CRYPTO_OBJ)ex_data.o $(CRYPTO_OBJ)mem.o $(CRYPTO_OBJ)mem_clr.o $(CRYPTO_OBJ)cryptlib.o $(CRYPTO_OBJ)mem_dbg.o \
          $(CRYPTO_OBJ)cversion.o $(CRYPTO_OBJ)o_time.o \
          $(CRYPTO_ERR_OBJ)err.o $(CRYPTO_ERR_OBJ)err_prn.o \
          $(CRYPTO_ERR_OBJ)err_all.o \
          $(CRYPTO_LHASH_OBJ)lhash.o \
          $(CRYPTO_STACK_OBJ)stack.o \
          $(CRYPTO_OBJECTS_OBJ)obj_dat.o \
          $(CRYPTO_OBJECTS_OBJ)obj_lib.o \
          $(CRYPTO_OBJECTS_OBJ)o_names.o \
          $(CRYPTO_X509_OBJ)x509_lu.o $(CRYPTO_X509_OBJ)x509_cmp.o $(CRYPTO_X509_OBJ)x509_obj.o $(CRYPTO_X509_OBJ)x509_vpm.o \
          $(CRYPTO_X509_OBJ)x509_d2.o $(CRYPTO_X509_OBJ)by_file.o $(CRYPTO_X509_OBJ)by_dir.o $(CRYPTO_X509_OBJ)x509_trs.o \
          $(CRYPTO_X509_OBJ)x509_def.o $(CRYPTO_X509_OBJ)x509_ext.o $(CRYPTO_X509_OBJ)x509_v3.o $(CRYPTO_X509_OBJ)x_all.o \
          $(CRYPTO_X509_OBJ)x509name.o $(CRYPTO_X509_OBJ)x509_att.o $(CRYPTO_X509_OBJ)x509_req.o $(CRYPTO_X509_OBJ)x509rset.o \
          $(CRYPTO_X509_OBJ)x509cset.o $(CRYPTO_X509_OBJ)x509_vfy.o $(CRYPTO_X509_OBJ)x509type.o \
          $(CRYPTO_X509_OBJ)x509_txt.o \
          $(CRYPTO_ASN1_OBJ)a_object.o $(CRYPTO_ASN1_OBJ)asn1_lib.o $(CRYPTO_ASN1_OBJ)evp_asn1.o $(CRYPTO_ASN1_OBJ)a_int.o \
          $(CRYPTO_ASN1_OBJ)a_type.o $(CRYPTO_ASN1_OBJ)x_sig.o $(CRYPTO_ASN1_OBJ)tasn_typ.o $(CRYPTO_ASN1_OBJ)tasn_fre.o \
          $(CRYPTO_ASN1_OBJ)tasn_enc.o $(CRYPTO_ASN1_OBJ)a_bitstr.o $(CRYPTO_ASN1_OBJ)tasn_utl.o $(CRYPTO_ASN1_OBJ)tasn_dec.o \
          $(CRYPTO_ASN1_OBJ)tasn_new.o $(CRYPTO_ASN1_OBJ)x_algor.o $(CRYPTO_ASN1_OBJ)a_bytes.o $(CRYPTO_ASN1_OBJ)x_attrib.o \
          $(CRYPTO_ASN1_OBJ)a_dup.o $(CRYPTO_ASN1_OBJ)x_bignum.o $(CRYPTO_ASN1_OBJ)x_long.o $(CRYPTO_ASN1_OBJ)x_name.o \
          $(CRYPTO_ASN1_OBJ)x_pubkey.o $(CRYPTO_ASN1_OBJ)x_crl.o $(CRYPTO_ASN1_OBJ)x_x509.o $(CRYPTO_ASN1_OBJ)x_exten.o \
          $(CRYPTO_ASN1_OBJ)x_val.o $(CRYPTO_ASN1_OBJ)x_x509a.o $(CRYPTO_ASN1_OBJ)a_time.o $(CRYPTO_ASN1_OBJ)x_info.o \
          $(CRYPTO_ASN1_OBJ)d2i_pu.o $(CRYPTO_ASN1_OBJ)i2d_pr.o $(CRYPTO_ASN1_OBJ)i2d_pu.o $(CRYPTO_ASN1_OBJ)d2i_pr.o \
          $(CRYPTO_ASN1_OBJ)a_octet.o $(CRYPTO_ASN1_OBJ)a_set.o $(CRYPTO_ASN1_OBJ)x_pkey.o $(CRYPTO_ASN1_OBJ)f_int.o \
          $(CRYPTO_ASN1_OBJ)f_string.o $(CRYPTO_ASN1_OBJ)a_enum.o $(CRYPTO_ASN1_OBJ)asn1_gen.o $(CRYPTO_ASN1_OBJ)a_print.o \
          $(CRYPTO_ASN1_OBJ)x_req.o $(CRYPTO_ASN1_OBJ)a_strex.o $(CRYPTO_ASN1_OBJ)a_strnid.o $(CRYPTO_ASN1_OBJ)a_mbstr.o \
          $(CRYPTO_ASN1_OBJ)t_x509.o $(CRYPTO_ASN1_OBJ)a_utf8.o \
          $(CRYPTO_ASN1_OBJ)p5_pbe.o \
          $(CRYPTO_ASN1_OBJ)p5_pbev2.o \
          $(CRYPTO_ASN1_OBJ)p8_pkey.o \
          $(CRYPTO_ASN1_OBJ)asn_pack.o $(CRYPTO_ASN1_OBJ)t_crl.o $(CRYPTO_ASN1_OBJ)t_pkey.o $(CRYPTO_ASN1_OBJ)asn1_par.o \
          $(CRYPTO_ASN1_OBJ)t_x509a.o $(CRYPTO_ASN1_OBJ)a_i2d_fp.o $(CRYPTO_ASN1_OBJ)a_bool.o $(CRYPTO_ASN1_OBJ)a_digest.o \
          $(CRYPTO_ASN1_OBJ)a_d2i_fp.o $(CRYPTO_ASN1_OBJ)a_gentm.o $(CRYPTO_ASN1_OBJ)a_hdr.o $(CRYPTO_ASN1_OBJ)a_meth.o \
          $(CRYPTO_ASN1_OBJ)a_sign.o $(CRYPTO_ASN1_OBJ)a_utctm.o $(CRYPTO_ASN1_OBJ)a_verify.o $(CRYPTO_ASN1_OBJ)asn1_err.o \
          $(CRYPTO_ASN1_OBJ)asn_moid.o $(CRYPTO_ASN1_OBJ)f_enum.o $(CRYPTO_ASN1_OBJ)x_spki.o \
          $(CRYPTO_ASN1_OBJ)nsseq.o \
          $(CRYPTO_BN_OBJ)bn_lib.o $(CRYPTO_BN_OBJ)bn_word.o $(CRYPTO_BN_OBJ)bn_shift.o $(CRYPTO_BN_OBJ)bn_asm.o $(CRYPTO_BN_OBJ)bn_print.o \
          $(CRYPTO_BN_OBJ)bn_ctx.o $(CRYPTO_BN_OBJ)bn_gcd.o  $(CRYPTO_BN_OBJ)bn_add.o $(CRYPTO_BN_OBJ)bn_mul.o $(CRYPTO_BN_OBJ)bn_mod.o \
          $(CRYPTO_BN_OBJ)bn_div.o $(CRYPTO_BN_OBJ)bn_blind.o $(CRYPTO_BN_OBJ)bn_mont.o $(CRYPTO_BN_OBJ)bn_exp.o $(CRYPTO_BN_OBJ)bn_rand.o \
          $(CRYPTO_BN_OBJ)bn_recp.o $(CRYPTO_BN_OBJ)bn_sqr.o $(CRYPTO_BN_OBJ)bn_exp2.o $(CRYPTO_BN_OBJ)bn_prime.o $(CRYPTO_BN_OBJ)bn_gf2m.o \
          $(CRYPTO_BN_OBJ)bn_kron.o $(CRYPTO_BN_OBJ)bn_nist.o $(CRYPTO_BN_OBJ)bn_sqrt.o \
          $(CRYPTO_BUFFER_OBJ)buffer.o \
          $(CRYPTO_MD5_OBJ)md5_dgst.o \
          $(CRYPTO_RAND_OBJ)rand_lib.o \
          $(CRYPTO_RAND_OBJ)md_rand.o \
          $(CRYPTO_BIO_OBJ)b_print.o \
          $(CRYPTO_BIO_OBJ)bio_lib.o \
          $(CRYPTO_BIO_OBJ)b_dump.o \
          $(CRYPTO_BIO_OBJ)bss_file.o \
          $(CRYPTO_BIO_OBJ)bss_sock.o \
          $(CRYPTO_BIO_OBJ)bf_buff.o \
          $(CRYPTO_EVP_OBJ)digest.o $(CRYPTO_EVP_OBJ)m_sha1.o $(CRYPTO_EVP_OBJ)evp_lib.o $(CRYPTO_EVP_OBJ)p_lib.o \
          $(CRYPTO_EVP_OBJ)names.o $(CRYPTO_EVP_OBJ)m_md5.o $(CRYPTO_EVP_OBJ)evp_enc.o $(CRYPTO_EVP_OBJ)evp_pkey.o \
          $(CRYPTO_EVP_OBJ)p_sign.o $(CRYPTO_EVP_OBJ)evp_pbe.o $(CRYPTO_EVP_OBJ)e_null.o $(CRYPTO_EVP_OBJ)m_ecdsa.o \
          $(CRYPTO_EVP_OBJ)m_dss1.o $(CRYPTO_EVP_OBJ)p_verify.o \
          $(CRYPTO_EVP_OBJ)m_mdc2.o \
          $(CRYPTO_EVP_OBJ)m_dss.o \
          $(CRYPTO_EVP_OBJ)m_md2.o \
          $(CRYPTO_EVP_OBJ)m_md4.o \
          $(CRYPTO_EVP_OBJ)e_des.o \
          $(CRYPTO_EVP_OBJ)e_des3.o \
          $(CRYPTO_EVP_OBJ)encode.o \
          $(CRYPTO_EVP_OBJ)p_seal.o \
          $(CRYPTO_EVP_OBJ)p5_crpt2.o \
          $(CRYPTO_EVP_OBJ)p5_crpt.o \
          $(CRYPTO_EVP_OBJ)p_dec.o \
          $(CRYPTO_EVP_OBJ)p_enc.o \
          $(CRYPTO_EVP_OBJ)p_open.o \
          $(CRYPTO_EVP_OBJ)evp_key.o \
          $(CRYPTO_EVP_OBJ)evp_err.o \
          $(CRYPTO_EVP_OBJ)e_rc4.o \
          $(CRYPTO_RSA_OBJ)rsa_sign.o $(CRYPTO_RSA_OBJ)rsa_lib.o  $(CRYPTO_RSA_OBJ)rsa_eay.o  $(CRYPTO_RSA_OBJ)rsa_none.o \
          $(CRYPTO_RSA_OBJ)rsa_x931.o $(CRYPTO_RSA_OBJ)rsa_pk1.o $(CRYPTO_RSA_OBJ)rsa_ssl.o $(CRYPTO_RSA_OBJ)rsa_oaep.o \
          $(CRYPTO_RSA_OBJ)rsa_asn1.o \
          $(CRYPTO_SHA_OBJ)sha512.o $(CRYPTO_SHA_OBJ)sha256.o $(CRYPTO_SHA_OBJ)sha1dgst.o \
          $(CRYPTO_COMP_OBJ)comp_lib.o \
          $(CRYPTO_COMP_OBJ)c_zlib.o \
          $(CRYPTO_X509V3_OBJ)v3_akeya.o $(CRYPTO_X509V3_OBJ)v3_genn.o $(CRYPTO_X509V3_OBJ)v3_purp.o $(CRYPTO_X509V3_OBJ)v3_lib.o \
          $(CRYPTO_X509V3_OBJ)v3_pcia.o $(CRYPTO_X509V3_OBJ)v3_bcons.o $(CRYPTO_X509V3_OBJ)v3_utl.o $(CRYPTO_X509V3_OBJ)v3_int.o \
          $(CRYPTO_X509V3_OBJ)v3_pmaps.o $(CRYPTO_X509V3_OBJ)v3_ncons.o $(CRYPTO_X509V3_OBJ)v3_ocsp.o $(CRYPTO_X509V3_OBJ)v3_alt.o \
          $(CRYPTO_X509V3_OBJ)v3_akey.o $(CRYPTO_X509V3_OBJ)v3_info.o $(CRYPTO_X509V3_OBJ)v3_pci.o $(CRYPTO_X509V3_OBJ)v3_cpols.o \
          $(CRYPTO_X509V3_OBJ)v3_crld.o $(CRYPTO_X509V3_OBJ)v3_extku.o $(CRYPTO_X509V3_OBJ)v3_pcons.o $(CRYPTO_X509V3_OBJ)v3_conf.o \
          $(CRYPTO_X509V3_OBJ)v3_ia5.o $(CRYPTO_X509V3_OBJ)v3_skey.o $(CRYPTO_X509V3_OBJ)v3_bitst.o $(CRYPTO_X509V3_OBJ)v3_enum.o \
          $(CRYPTO_X509V3_OBJ)v3_sxnet.o $(CRYPTO_X509V3_OBJ)v3_pku.o $(CRYPTO_X509V3_OBJ)v3_prn.o $(CRYPTO_X509V3_OBJ)pcy_cache.o \
          $(CRYPTO_X509V3_OBJ)pcy_data.o $(CRYPTO_X509V3_OBJ)pcy_lib.o $(CRYPTO_X509V3_OBJ)pcy_map.o $(CRYPTO_X509V3_OBJ)pcy_node.o \
          $(CRYPTO_X509V3_OBJ)pcy_tree.o \
          $(CRYPTO_OCSP_OBJ)ocsp_asn.o \
          $(CRYPTO_CONF_OBJ)conf_mod.o \
          $(CRYPTO_CONF_OBJ)conf_lib.o \
          $(CRYPTO_CONF_OBJ)conf_api.o \
          $(CRYPTO_CONF_OBJ)conf_def.o \
          $(CRYPTO_DSO_OBJ)dso_lib.o \
          $(CRYPTO_DSO_OBJ)dso_openssl.o \
          $(CRYPTO_DSO_OBJ)dso_null.o \
          $(CRYPTO_HMAC_OBJ)hmac.o \
          $(CRYPTO_PEM_OBJ)pem_all.o \
          $(CRYPTO_PEM_OBJ)pem_err.o \
          $(CRYPTO_PEM_OBJ)pem_info.o \
          $(CRYPTO_PEM_OBJ)pem_lib.o \
          $(CRYPTO_PEM_OBJ)pem_oth.o \
          $(CRYPTO_PEM_OBJ)pem_pk8.o \
          $(CRYPTO_PEM_OBJ)pem_pkey.o \
          $(CRYPTO_PEM_OBJ)pem_seal.o \
          $(CRYPTO_PEM_OBJ)pem_sign.o \
          $(CRYPTO_PEM_OBJ)pem_x509.o \
          $(CRYPTO_PEM_OBJ)pem_xaux.o \
          $(CRYPTO_PKCS12_OBJ)p12_decr.o \
          $(CRYPTO_PKCS12_OBJ)p12_p8d.o \
          $(CRYPTO_PKCS12_OBJ)p12_p8e.o \
          $(CRYPTO_PKCS7_OBJ)pk7_asn1.o \
          $(CRYPTO_RC4_OBJ)rc4_skey.o \
          $(CRYPTO_RC4_OBJ)rc4_enc.o
                   	
ALL_OBJS =  $(aa) $(S_OBJS) $(C_OBJS)

all: $(TARGET) makefile 
$(TARGET)	: $(ALL_OBJS)
	$(AR) -rsv $(TARGET) $(ALL_OBJS)

$(S_OBJS):	$(OBJ)%.o: $(SRC)%.s makefile
	$(AS) $(AS_OPTION)$(@:.o=.lst) -o $@ $<

$(C_OBJS):	$(OBJ)%.o: $(SRC)%.c makefile 
	$(CC) $(CC_OPTION)$(@:.o=.lst) -o $@ $<

clean:
	rm -f $(ALL_OBJS) $(SSL_OBJ)*.lst $(CRYPTO_OBJ)*.lst
	rm -f $(CRYPTO_ERR_OBJ)*.lst
	rm -f $(CRYPTO_ASN_OBJ)*.lst
	rm -f $(CRYPTO_LHASH_OBJ)*.lst
	rm -f $(CRYPTO_STACK_OBJ)*.lst
	rm -f $(CRYPTO_OBJECTS_OBJ)*.lst
	rm -f $(CRYPTO_X509_OBJ)*.lst
	rm -f $(CRYPTO_ASN1_OBJ)*.lst
	rm -f $(CRYPTO_BN_OBJ)*.lst
	rm -f $(CRYPTO_BUFFER_OBJ)*.lst
	rm -f $(CRYPTO_DES_OBJ)*.lst
	rm -f $(CRYPTO_MD4_OBJ)*.lst
	rm -f $(CRYPTO_MD5_OBJ)*.lst
	rm -f $(CRYPTO_RAND_OBJ)*.lst
	rm -f $(CRYPTO_BIO_OBJ)*.lst
	rm -f $(CRYPTO_EVP_OBJ)*.lst
	rm -f $(CRYPTO_RSA_OBJ)*.lst
	rm -f $(CRYPTO_SHA_OBJ)*.lst
	rm -f $(CRYPTO_EC_OBJ)*.lst
	rm -f $(CRYPTO_DH_OBJ)*.lst
	rm -f $(CRYPTO_DSA_OBJ)*.lst
	rm -f $(CRYPTO_ECDSA_OBJ)*.lst
	rm -f $(CRYPTO_PKCS7_OBJ)*.lst
	rm -f $(CRYPTO_PKCS12_OBJ)*.lst
	rm -f $(CRYPTO_DSO_OBJ)*.lst
	rm -f $(CRYPTO_X509V3_OBJ)*.lst
	rm -f $(CRYPTO_PEM_OBJ)*.lst
	rm -f $(CRYPTO_HMAC_OBJ)*.lst
	rm -f $(CRYPTO_COMP_OBJ)*.lst
	rm -f $(CRYPTO_OCSP_OBJ)*.lst
	rm -f $(CRYPTO_CONF_OBJ)*.lst
	rm -f $(CRYPTO_ECDH_OBJ)*.lst
	rm -f $(CRYPTO_RC4_OBJ)*.lst
	rm -f $(TARGET)
