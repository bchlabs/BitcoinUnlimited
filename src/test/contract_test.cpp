#include "contract/contractinterpeter.h"
#include "contract/contracterror.h"
#include "contract/rpctoken.h"
#include "contract/tokentxcheck.h"
#include "contract/contracttally.h"
#include "contract/contractio.h"
#include "contract/interactiveblockchain.h"
#include "test/test_bitcoin.h"
#include <boost/test/unit_test.hpp>
#include <iostream>
#include <string>
#include "key.h"
#include "dstencode.h"

BOOST_FIXTURE_TEST_SUITE(contract_test, BasicTestingSetup)

BOOST_AUTO_TEST_CASE(contractIO)
{
    std::cout << "contractIO  begin --------------" << std::endl;

    int test = 893;
    std::string teststr = contractio::intToString(test) ;
    std::cout << "test :" << teststr <<std::endl;
    uint64_t  test_64 = 7823924;
    std::cout << "test uint64: " << contractio::uint64ToString(test_64) << std::endl;

    std::cout << "contractIO end ----------------" << std::endl;
}

BOOST_AUTO_TEST_CASE(constractSignVerify)
{
    std::cout << "constractSignVerify  begin --------------" << std::endl;

    CKey privKey;
    privKey.MakeNewKey(true);
    CPubKey pubKey = privKey.GetPubKey();
    std::cout << "keyid is:" << pubKey.GetID().ToString() << std::endl;

    std::string sign_address = EncodeDestination(pubKey.GetID()) ;
    std::cout << "address:" << sign_address << std::endl;


    std::string message_commit = "justfor test";
    std::string message_sign = signCommit(privKey,message_commit);

    std::string message_test = "saffes";
    BOOST_CHECK(verifyWitness(message_sign,sign_address,message_commit));
    BOOST_CHECK(!verifyWitness(message_sign,sign_address,message_test));

    std::cout << "constractSignVerify  end ----------------" << std::endl;
}

BOOST_AUTO_TEST_SUITE_END()
