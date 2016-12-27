//
//  test_rsa_pss_sha256.cpp
//  openssl
//
//  Created by wutianqiang on 16/3/17.
//  Copyright © 2016年 Tencent. All rights reserved.
//

#include "../../export_include/gen_rsa_key_pair.h"
#include "../../export_include/rsa_pss_sha256.h"
#include <string.h>
#include <string>
#include "../../../comm/xlogger/xlogger.h"

static const char* sg_private_key = "-----BEGIN RSA PRIVATE KEY-----\n"
		"MIIEowIBAAKCAQEAvz64Xd1ViF0hUP99JHIPnucAUIXSsWWHb5EAO+QkBpBEaO77\n"
		"wjb5DpkZa1XUsrZ2hKqlQARo3zMrWCF+twcfSmoEwJGgwMpAfvS1zvj5MEXL2qXV\n"
		"5aWvcFW7AseFxotJbQjBGFVaQruxHfIA/BHINXuqZ00Tw6JhcIg7Do3UzA78yC7d\n"
		"52sjgFL6YeCMAUF7RYdRiZGkX/FMgH6jzIo2nWE7P+80rTA3h1129QS/dx4AyRkK\n"
		"I/HfBpXcYDUarHSvuNQ57w6xbrf2cKU1ks4fi2GYKnTI1zVSblfGw7VemLnqVbsz\n"
		"MDlNqVNmnv7HJosmEMbbu7aH1fHecH+hBwFaIwIDAQABAoIBADUXonNciOxMfn1Q\n"
		"EFvX1oJUuv+fxzg8aXcNmDUIRwATeQpwni/ytv3YfvgPYfcjNI/sF+hH0R11xSi4\n"
		"U8dYENXFzP/w7eHXxEP6h6an8/X0yn0u4MzaTHQxOLOy1G6kKIQ+k+y/dUlldexM\n"
		"TP2tqhk+9bWr7jq3j7l/+HU35sWnQKXu0iaw98KMHdKDo+GxJO3FadwuzVQjQqf0\n"
		"6HlJvFqBU1IEQuujqPaoblI3iTWPtK7w5E+kdNsTA577FWXD3XaCGwhDgX34lt71\n"
		"NBxQ5s6SYlSnSZ84DH37rzckZXMbD3xXwCHHSRsgayT7xh0nk0qzuTYrp6MTYGPZ\n"
		"MMffOvECgYEA3zQr7mMbRqGrTBbHFG/AYxNAvYG/yxf0xD54DulQxi5xnbVc8zLu\n"
		"6qlJlGqi32BuB3sn64c/y8KBTCaTtTQHAMo0EhOKXCJj9bPVZqqaadL03Vw5/fQg\n"
		"txso12w9VMUZ2BwFS04dogGX8SXKTLgQ8o5IlkaoyY2c2pLzNmK7X9cCgYEA21hq\n"
		"fmyoXD5O+YiuC/dN9AzfDVpKxzOMNbPrR1PtPO6BG4nYdGoYCW/URaw1whMg9QSV\n"
		"eXG2mVZC5u2VSlSWRhKuNGAGqITVm/E81xMhkja0luk47mwTg2eOF2ctV+zji5tJ\n"
		"YwkXfkH7BqJHNG50Hc8beA53joRJX0ddd+GXvpUCgYBjBq6G8Qlk6M0q0OA6POpK\n"
		"CnyqlyoUXOiHOH/jfbgMdQaLRa+bZnmBOuIeKsFgAVRPlWA8qqCvMfhTZLobpp2E\n"
		"yDEZ4/l6rvdSRxrilcY0NRfWLdi8xtbM5gvUp3z+YwP4UVY39sxG2BYlMvLx3qgG\n"
		"NQIXSfoQkfeJMbEd6+fduQKBgB59feAxDn40ExpQy5DSQb3JBECA6qAiJD1a0zvJ\n"
		"CNNNfBjt4m7cSJcXOnSs6ZQewJgOmRCWpAFQomAjans4ouL+ucSCDKrkPCU/NWvY\n"
		"NvRyMkxAAy/xSg0cGf5a00zDmyXO4jaoDuH4HgLgD/HT4MQ8A99+c1QWQ3TBINcH\n"
		"Pua5AoGBAMfB6zAJuQfJMlSdG2WEQJG1SScDrYryuS5AueJTVBhu3479sCowKkEz\n"
		"1Oz1vmn4GYRF5M2gpIc+9Z6EqIySYG4Mc2yifBVssmWyU8wocxWG8Lf5p6D8fwZv\n"
		"Xc/sCw6tJuhENs45TppUEsZk2qL58Kv1tbgPEbSuF0ubt4Lp7kxG\n"
		"-----END RSA PRIVATE KEY-----";

static const char* sg_pub_key ="-----BEGIN PUBLIC KEY-----\n"
		"MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAvz64Xd1ViF0hUP99JHIP\n"
		"nucAUIXSsWWHb5EAO+QkBpBEaO77wjb5DpkZa1XUsrZ2hKqlQARo3zMrWCF+twcf\n"
		"SmoEwJGgwMpAfvS1zvj5MEXL2qXV5aWvcFW7AseFxotJbQjBGFVaQruxHfIA/BHI\n"
		"NXuqZ00Tw6JhcIg7Do3UzA78yC7d52sjgFL6YeCMAUF7RYdRiZGkX/FMgH6jzIo2\n"
		"nWE7P+80rTA3h1129QS/dx4AyRkKI/HfBpXcYDUarHSvuNQ57w6xbrf2cKU1ks4f\n"
		"i2GYKnTI1zVSblfGw7VemLnqVbszMDlNqVNmnv7HJosmEMbbu7aH1fHecH+hBwFa\n"
		"IwIDAQAB\n-----END PUBLIC KEY-----";

void test_rsa_pss_sha256_sign(std::string _message) {
	xinfo_function();
    char l_pub_key[4096] = {0};
    char l_pri_key[4096] = {0};
    generate_rsa_key_pair_2048(l_pub_key, sizeof(l_pub_key), l_pri_key, sizeof(l_pri_key));
	xinfo2(TSF"_message=%_, _message.size=%_", _message.c_str(), _message.size());//_message is binary data
    //sign signature
    unsigned char signature[1024] ={0};
    size_t signature_len = sizeof(signature);
    bool sign_res = rsa_pss_sha256_sign(_message.c_str(), _message.size(), l_pri_key/*sg_private_key*/, (size_t)strlen(l_pri_key/*sg_private_key*/)
                        , (unsigned char*)&signature, signature_len);
    
    xinfo2(TSF"sign_res=%_, signature_len=%_, signature=%_", sign_res, signature_len, xdump(signature, signature_len));
    //message and signature are both binary data, cannot use strlen


    //verify signature
    bool verify_res = rsa_pss_sha256_verify(_message.c_str(), _message.size(), l_pub_key/*sg_pub_key*/, (size_t)strlen(l_pub_key/*sg_pub_key*/)
    					, (unsigned char*)&signature, signature_len);
    xinfo2(TSF"verify_res=%_", verify_res);
}
