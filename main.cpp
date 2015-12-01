#include "src/MicroCore.h"
#include "src/CmdLineOptions.h"
#include "src/tools.h"

#include "ext/format.h"

using namespace std;
using namespace fmt;

using xmreg::operator<<;


using boost::filesystem::path;

unsigned int epee::g_test_dbg_lock_sleep = 0;


int main(int ac, const char* av[]) {

    // get command line options
    xmreg::CmdLineOptions opts {ac, av};

    auto help_opt = opts.get_option<bool>("help");

    // if help was chosen, display help text and finish
    if (*help_opt)
    {
        return 0;
    }


    // flag indicating if viewkey and address were
    // given by the user
    bool VIEWKEY_AND_ADDRESS_GIVEN {false};

    // get other options
    auto tx_hash_opt = opts.get_option<string>("txhash");
    auto viewkey_opt = opts.get_option<string>("viewkey");
    auto address_opt = opts.get_option<string>("address");
    auto bc_path_opt = opts.get_option<string>("bc-path");


    // get the program command line options, or
    // some default values for quick check
    string tx_hash_str = tx_hash_opt ? *tx_hash_opt : "09d9e8eccf82b3d6811ed7005102caf1b605f325cf60ed372abeb4a67d956fff";


    crypto::hash tx_hash;

    if (!xmreg::parse_str_secret_key(tx_hash_str, tx_hash))
    {
        cerr << "Cant parse tx hash: " << tx_hash_str << endl;
        return 1;
    }

    crypto::secret_key private_view_key;
    cryptonote::account_public_address address;

    if (viewkey_opt && address_opt)
    {
         // parse string representing given private viewkey
        if (!xmreg::parse_str_secret_key(*viewkey_opt, private_view_key))
        {
            cerr << "Cant parse view key: " << *viewkey_opt << endl;
            return 1;
        }

        // parse string representing given monero address
        if (!xmreg::parse_str_address(*address_opt,  address))
        {
            cerr << "Cant parse address: " << *address_opt << endl;
            return 1;
        }

        VIEWKEY_AND_ADDRESS_GIVEN = true;
    }


    path blockchain_path;

    if (!xmreg::get_blockchain_path(bc_path_opt, blockchain_path))
    {
        // if problem obtaining blockchain path, finish.
        return 1;
    }

    print("Blockchain path      : {}\n", blockchain_path);

    // enable basic monero log output
    xmreg::enable_monero_log();

    // create instance of our MicroCore
    xmreg::MicroCore mcore;

    // initialize the core using the blockchain path
    if (!mcore.init(blockchain_path.string()))
    {
        cerr << "Error accessing blockchain." << endl;
        return 1;
    }


    print("\n\ntx hash          : {}\n\n", tx_hash);

    if (VIEWKEY_AND_ADDRESS_GIVEN)
    {
        // lets check our keys
        print("private view key : {}\n", private_view_key);
        print("address          : {}\n\n\n", address);
    }


    // get the high level cryptonote::Blockchain object to interact
    // with the blockchain lmdb database
    cryptonote::Blockchain& core_storage = mcore.get_core();

    cryptonote::transaction tx;

    try
    {
        // get transaction with given hash
        tx = core_storage.get_db().get_tx(tx_hash);
    }
    catch (const std::exception& e)
    {
        cerr << e.what() << endl;
        return false;
    }


    cout << "Signatures: " << endl;

    for (const std::vector<crypto::signature>& sigv: tx.signatures)
    {
        size_t i {0};

        for (const crypto::signature& sig: sigv)
        {
            //cout << " - " << ++i << epee::string_tools::pod_to_hex(sig.c) << endl;
            //cout << " - " << ++i << sig.c << endl;
            cout << " - " << ++i << sig << endl;
        }

        cout << endl;

    }


    cout << "\nEnd of program." << endl;

    return 0;
}
