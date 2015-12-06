#include "src/MicroCore.h"
#include "src/CmdLineOptions.h"
#include "src/tools.h"

#include "ext/format.h"

using namespace std;
using namespace fmt;

using xmreg::operator<<;
using xmreg::print_sig;


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
    string tx_hash_str = tx_hash_opt ? *tx_hash_opt : "5961e98c41d212eee89c2e581cdae08fd6c6c5703b484d6332b1196c47f1c7de";


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

    crypto::hash tx_prefix_hash = cryptonote::get_transaction_prefix_hash(tx);


    size_t in_i = tx.vin.size();

    vector<uint64_t> results;
    results.resize(tx.vin.size(), 0);

    for (size_t i = 0; i < tx.vin.size(); ++i)
    {
        const cryptonote::txin_v& tx_in = tx.vin[i];

        // get tx input key
        const cryptonote::txin_to_key& tx_in_to_key
                = boost::get<cryptonote::txin_to_key>(tx_in);


        cout <<  "Key image: " << tx_in_to_key.k_image << endl;



        uint64_t pmax_used_block_height {0};

        vector<crypto::public_key> mixins_pub_keys;


        // get absolute offsets of mixins
        std::vector<uint64_t> absolute_offsets
                = cryptonote::relative_output_offsets_to_absolute(
                        tx_in_to_key.key_offsets);

        std::vector<cryptonote::output_data_t> outputs;
        core_storage.get_db().get_output_key(tx_in_to_key.amount,
                                             absolute_offsets,
                                             outputs);



        vector<crypto::public_key> outs_pub_keys;

        for (size_t outi = 0; outi < absolute_offsets.size(); ++outi)
        {
            cryptonote::output_data_t output_data = outputs.at(outi);
            outs_pub_keys.push_back(output_data.pubkey);

            cout << "  - mix out pubkey: " << output_data.pubkey << endl;
            //cout << "  - sig: " << tx.signatures[i][outi] << endl;

            vector<const crypto::public_key*> out_pub_key_array;

            out_pub_key_array.push_back(&output_data.pubkey);
            vector<crypto::signature> sig_array;
            sig_array.push_back( tx.signatures[i][outi]);
//
//            crypto::check_ring_signature(tx_prefix_hash,
//                                         tx_in_to_key.k_image,
//                                         out_pub_key_array,
//                                         tx.signatures[i].data());

//            crypto::crypto_ops::check_ring_signature(tx_prefix_hash,
//                                                     tx_in_to_key.k_image,
//                                                     out_pub_key_array.data(),
//                                                     out_pub_key_array.size(),
//                                                     sig_array.data());


//            bool result = crypto::check_ring_signature(tx_prefix_hash,
//                                         tx_in_to_key.k_image,
//                                         out_pub_key_array.data(),
//                                         out_pub_key_array.size(),
//                                         sig_array.data());


            for (const crypto::signature& sig: tx.signatures[i])
            {
                cout << "    - sig: " << print_sig(sig) << endl;
//                bool result = crypto::check_signature(tx_prefix_hash,
//                                                      output_data.pubkey,
//                                                      sig);

                vector<crypto::signature> sig_array;
                sig_array.push_back(sig);

            bool result = crypto::check_ring_signature(tx_prefix_hash,
                                         tx_in_to_key.k_image,
                                         out_pub_key_array.data(),
                                         out_pub_key_array.size(),
                                         sig_array.data());

                cout << "    - result: " << result << endl;

            }


            //outs_pub_keys.push_back(output_data.pubkey);
        }


        //cout << "tx.signatures[i].size(): " << tx.signatures[i].size() << endl;

//        cout << "\ntx.signatures[i].size(): " << tx.signatures[i].size() << endl;
//
//        for (const crypto::signature& sig: tx.signatures[i])
//        {
//            cout << "  - sig: " << print_sig(sig) << endl;
//        }


        uint64_t result;

        vector<crypto::signature> sigs = tx.signatures[i];


        cout << endl;

        for (auto& s: sigs)
        {
            cout << print_sig(s) << endl;
        }


        cout << "After random: " << endl;

        random_shuffle (sigs.begin(), sigs.end());

        for (auto& s: sigs)
        {
            cout << print_sig(s) << endl;
        }
//
        mcore.check_ring_signature(tx_prefix_hash,
                         tx_in_to_key.k_image,
                         outs_pub_keys,
                         sigs,
                         result);



        cout << result << endl;

//
//        check_ring_signature(tx_prefix_hash,
//                             tx_in_to_key.k_image,
//                             pubkeys[sig_index],
//                             tx.signatures[i],
//                             results[sig_index]);


    }


//
//    for (const std::vector<crypto::signature>& sigv: tx.signatures)
//    {
//        size_t i {0};
//
//        for (const crypto::signature& sig: sigv)
//        {
//            cout << " - " << print_sig(sig) << endl;
//        }
//
//        cout << endl;
//    }
//
//
//




    cout << "\nEnd of program." << endl;

    return 0;
}
