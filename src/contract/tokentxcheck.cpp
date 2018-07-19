#include "tokentxcheck.h"
#include "amount.h"
#include "chain.h"
#include "core_io.h"
#include "dstencode.h"
#include "init.h"
#include "main.h"
#include "net.h"
#include "rpc/server.h"
#include "script/sign.h"
#include "timedata.h"
#include "util.h"
#include "utilmoneystr.h"


#ifdef ENABLE_WALLET
#include "wallet/wallet.h"
#include "wallet/walletdb.h"
#endif

int CheckTokenVin(const UniValue &params)
{

    using namespace std;
    int ret = TOKENOK;
    LOCK2(cs_main, pwalletMain->cs_wallet);

    vector<std::string> token_vin_list  = params.getKeys();
    vector<COutput> vecOutputs;
    assert(pwalletMain != NULL);
    pwalletMain->AvailableCoins(vecOutputs, false, NULL, true);
    int count_utxo =0;
    for (unsigned int i =0;i<token_vin_list.size();i++)
    {
        string token_vin_txid = token_vin_list[i];
        uint256 hash;
        hash.SetHex(token_vin_txid);
       // isminefilter filter = ISMINE_SPENDABLE;
        if (!pwalletMain->mapWallet.count(hash))
        {
            return ERRORTOKENVIN;
        }
        for (unsigned  int j =0;j<vecOutputs.size();j++ )
        {
             COutput  out = vecOutputs.at(j);
             if ( out.tx->GetHash() == hash )
             {
                 count_utxo ++;
                 break;
             }
        }
    }

    if (!count_utxo)
    {
        ret = ERRORTOKENVIN;
    }

    return ret;
}
