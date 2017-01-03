// Tencent is pleased to support the open source community by making Mars available.
// Copyright (C) 2016 THL A29 Limited, a Tencent company. All rights reserved.

// Licensed under the MIT License (the "License"); you may not use this file except in 
// compliance with the License. You may obtain a copy of the License at
// http://opensource.org/licenses/MIT

// Unless required by applicable law or agreed to in writing, software distributed under the License is
// distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
// either express or implied. See the License for the specific language governing permissions and
// limitations under the License.

/*
* Created on 2015-04-01
*	  Author: yanguoyue
*
*/

#ifndef __TESTCONTANTS__
#define __TESTCONTANTS__ 


static const char* const CONFIG_PATH = "C:\\host";

const char* const LONGLINK_TEST_CMD[] = {
	"..\\stub\\echo.py -e AAAAEAAQAAE7msoGAAAAAA== -k 20000 127.0.0.1:8080",
	"..\\stub\\echo.py -e AAAAEAAQAAE7msoGAAAAAA== -k 20000 127.0.0.1:8001",
	"..\\stub\\echo.py -e AAAAEAAQAAE7msoGAAAAAA== -k 20000 127.0.0.1:8002",
	"..\\stub\\echo.py -e AAAAEAAQAAE7msoGAAAAAA== -k 20000 127.0.0.1:8003",
	"..\\stub\\echo.py -e AAAAEAAQAAE7msoGAAAAAA== -k 20000 127.0.0.1:8004",
	"..\\stub\\echo.py -e AAAAEAAQAAE7msoGAAAAAA== -d 10000 -k 20000 127.0.0.1:7999",
	"..\\stub\\echo.py -e AAAAEAAQAAE7msoGAAAAAA== -d 10000 -k 20000 127.0.0.1:8000"
};

const char* const NETSOURCE_TEST_CMD = "..\\httpfile.py 127.0.0.88:8180";
const char* const NETSOURCE_CHDIR_DIR = "..\\stub\\http_file";

const char* const LONGLINKTASKMANAGER_TEST_CMD[] = {
	"..\\stub\\echo.py -e AAAAEAAQAAE7msoGAAAAAA== -k 20000 --echo1 AAAAEAAQAAE7msoGAAAAAA==  --echo2 AAAAEAAQAAEAAAPpAAAAAQ== --length2 1 --period2 2000 127.0.0.100:8090",
	"..\\stub\\echo.py -e AAAAEAAQAAE7msoGAAAAAA== -k 20000 --echo1 AAAAEAAQAAE7msoGAAAAAA==  --echo2 AAAAEAAQAAEAAAPpAAAAAQ== --length2 1 --period2 2000 127.0.0.101:8091",
	"..\\stub\\echo.py -e AAAAEAAQAAE7msoGAAAAAA== -k 30000 --echo1 AAAAEAAQAAE7msoGAAAAAA==  --echo2 AAAAsAAQAAEAAAPpAAAAAgAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA= --length2 16 --period2 4000 127.0.0.102:8092",
	"..\\stub\\echo.py -e AAAAEAAQAAE7msoGAAAAAA== -k 30000 --echo1 AAAAEAAQAAE7msoGAAAAAA==  --echo2 AAAAsAAQAAEAAAPpAAAAAgAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA= --length2 16 --period2 4000 127.0.0.103:8093",
};

const char* const WHOLE_TEST_AVAILABLE_CMD = "..\\stub\\echo.py -e AAAAEAAQAAE7msoGAAAAAA== -k 30000 --echo1 AAAAEAAQAAE7msoGAAAAAA==  --echo2 AAAAsAAQAAEAAAPpAAAAAgAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA= %s:%u";

const char* const START_NORMAL_ECHO_CMD = "python ..\\stub\\echo_normal.py %s:%u";
const char* const START_NEWDNS_SERVER_CMD = "python ..\\httpfile.py %s:%u";
#endif
