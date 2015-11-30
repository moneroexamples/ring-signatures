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

    for (const cryptonote::txin_v& tx_in: tx.vin)
    {
        // get tx input key
        const cryptonote::txin_to_key& tx_in_to_key
                = boost::get<cryptonote::txin_to_key>(tx_in);


        print("Input's Key image: {}, xmr: {:0.4f}\n",
              tx_in_to_key.k_image,
              xmreg::get_xmr(tx_in_to_key.amount));

        // get absolute offsets of mixins
        std::vector<uint64_t> absolute_offsets
                = cryptonote::relative_output_offsets_to_absolute(
                        tx_in_to_key.key_offsets);

        std::vector<cryptonote::output_data_t> outputs;
        core_storage.get_db().get_output_key(tx_in_to_key.amount,
                                             absolute_offsets,
                                             outputs);

        size_t count = 0;

        for (const uint64_t& i: absolute_offsets)
        {
            cryptonote::output_data_t output_data;

            // get tx hash and output index for output
            if (count < outputs.size())
            {
                output_data = outputs.at(count);
            }
            else
            {
                output_data = core_storage.get_db().get_output_key(
                        tx_in_to_key.amount, i);
            }


            // find tx_hash with given output
            crypto::hash tx_hash;
            cryptonote::transaction tx_found;

            if (!mcore.get_tx_hash_from_output_pubkey(
                    output_data.pubkey,
                    output_data.height,
                    tx_hash, tx_found))
            {
                print("- cant find tx_hash for ouput: {}, mixin no: {}, blk: {}\n",
                      output_data.pubkey, count + 1, output_data.height);

                continue;
            }


            // find output in a given transaction
            // basted on its public key
            cryptonote::tx_out found_output;
            size_t output_index;

            if (!mcore.find_output_in_tx(tx_found,
                                         output_data.pubkey,
                                         found_output,
                                         output_index))
            {
                print("- cant find tx_out for ouput: {}, mixin no: {}, blk: {}\n",
                      output_data.pubkey, count + 1, output_data.height);

                continue;
            }

            print("\n - mixin no: {}, block height: {}",
                  count + 1, output_data.height);

            bool is_ours {false};

            if (VIEWKEY_AND_ADDRESS_GIVEN)
            {
                // check if the given mixin's output is ours based
                // on the view key and public spend key from the address
                is_ours = xmreg::is_output_ours(output_index, tx_found,
                                                private_view_key,
                                                address.m_spend_public_key);

                Color c  = is_ours ? Color::GREEN : Color::RED;

                print(", ours: "); print_colored(c, "{}", is_ours);
            }

            print("\n  - output's pubkey: {}\n", output_data.pubkey);

            print("  - in tx with hash: {}, out_i: {:03d}, xmr: {:0.4f}\n",
                  tx_hash, output_index, xmreg::get_xmr(found_output.amount));

            ++count;
        }

        cout << endl;
    }

    cout << "\nEnd of program." << endl;

    return 0;
}
