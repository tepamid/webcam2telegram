g++ -Wno-c++11-long-long -Wno-c++11-extensions -I./lib/openssl-1.0.2o/include -I./lib/boost_1_67_0 -L./lib/boost_1_67_0/stage/lib -L./lib/openssl-1.0.2o -lboost_system -lboost_thread -lssl -lcrypto ./src/main.cpp -o sender.out
