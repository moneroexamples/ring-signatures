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



struct for_signatures
{
    crypto::hash tx_hash ;
    crypto::key_image kimg ;
    std::vector<crypto::public_key> outs_pub_keys;
    cryptonote::keypair in_ephemeral;
    size_t real_output {0};
};




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
    auto idx_opt = opts.get_option<size_t>("idx");


    // get the program command line options, or
    // some default values for quick check
    string tx_hash_str = tx_hash_opt ?
                         *tx_hash_opt :
                         "fc783b65b728bd5a14c2ff085421a72f52b2568b1b8698eb473ca2e27372d235";


    crypto::hash tx_hash;

    if (!xmreg::parse_str_secret_key(tx_hash_str, tx_hash))
    {
        cerr << "Cant parse tx hash: " << tx_hash_str << endl;
        return 1;
    }

    crypto::secret_key private_view_key;
    crypto::secret_key private_spend_key;
    cryptonote::account_public_address address;


//    string viewkey_str {"fed77158ec692fe9eb951f6aeb22c3bda16fe8926c1aac13a5651a9c27f34309"};
//    string spendkey_str {"1eaa41781d5f880dc69c9379e281225c781a6db8dc544a26008e7a07890afa03"};
//
//    string address_str {"41vEA7Ye8Bpeda6g59v5t46koWrVn2PNgEKgzquJjmiKCFTsh9gajr8J3pad49rqu581TAtFGCH9CYTCkYrCpuWUG9GkgeB"};
//

    string viewkey_str {"fed77158ec692fe9eb951f6aeb22c3bda16fe8926c1aac13a5651a9c27f34309"};
    string spendkey_str {"1eaa41781d5f880dc69c9379e281225c781a6db8dc544a26008e7a07890afa03"};

    string address_str {"41vEA7Ye8Bpeda6g59v5t46koWrVn2PNgEKgzquJjmiKCFTsh9gajr8J3pad49rqu581TAtFGCH9CYTCkYrCpuWUG9GkgeB"};


    // parse string representing given private viewkey
    if (!xmreg::parse_str_secret_key(viewkey_str, private_view_key))
    {
        cerr << "Cant parse view key: " << viewkey_str << endl;
        return 1;
    }



    // parse string representing given private spend
    if (!xmreg::parse_str_secret_key(spendkey_str, private_spend_key))
    {
        cerr << "Cant parse view key: " << spendkey_str << endl;
        return 1;
    }

    // parse string representing given monero address
    if (!xmreg::parse_str_address(address_str,  address))
    {
        cerr << "Cant parse address: " << address_str << endl;
        return 1;
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


    // lets check our keys
    print("private view key : {}\n", private_view_key);
    print("private spend key: {}\n", private_spend_key);
    print("address          : {}\n\n\n", address);



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

    size_t mixin_no = 4;






    // find random output for each input
    // based on the amount of that input
    for (size_t i = 0; i < tx.vin.size(); ++i) {


        vector<for_signatures> for_sig_v;


        const cryptonote::txin_v &tx_in = tx.vin[i];


        // get tx input key
        const cryptonote::txin_to_key& tx_in_to_key
                = boost::get<cryptonote::txin_to_key>(tx_in);


        uint64_t num_outs = core_storage.get_db().get_num_outputs(tx_in_to_key.amount);


        cout << "tx input amount: " << tx_in_to_key.amount << endl;
        cout << "no of ouputs with such amount: " << num_outs << endl;



        cout << "uint64_t is size: " << sizeof(unsigned long int) << endl; // uint64_t

        cout << "char size: " << sizeof(char) << endl; // char

        cout << "(char)1 << 53: " << (char) 1 << 8 << endl; // char


        ofstream myfile;

        myfile.open("example.csv");

        for (size_t j = 0; j< 1; ++j)
        {
            uint64_t rndv = crypto::rand<uint64_t>();
            // triangular distribution over [a,b) with a=0, mode c=b=up_index_limit
            uint64_t r = rndv % ((uint64_t)1 << 53);

            cout << rndv  << " % " << ((uint64_t)1 << 53) << " max(): "
            <<  std::numeric_limits<uint64_t>::max() << endl;

            cout << "r = " << r << endl;

            double frac = std::sqrt((double)r / ((uint64_t)1 << 53));
            uint64_t ii = (uint64_t)(frac*num_outs);

            cout << " frac " << frac << ", i " << ii << endl;
            myfile << ii << "\n";

        }
        myfile.close();

        cout << "tx input amount: " << tx_in_to_key.amount << endl;
        cout << "no of ouputs with such amount: " << num_outs << endl;






        continue;

//        for (size_t j = 0; j < mixin_no; ++j)
//        {
//            const cryptonote::tx_out_index toi
//                    = core_storage.get_db().get_output_tx_and_index(tx_in_to_key.amount, num_outs - 1);
//
//            uint64_t height = core_storage.get_db().get_tx_block_height(toi.first);
//
//            cout << endl;
//            cout << "amount index: " << num_outs - 1 << endl;
//            cout << "mixin no: " << j << endl;
//            cout << "tx found hash: " << crypto::hash(toi.first) << " f";
//            cout << " in block of height: " << height;
//            cout << "\noutput index in that transaction: " << toi.second << endl;
//
//            --num_outs;
//        }

    }

    return 0;

    cout << "\n" << endl;


    cout << "Signatures: " << endl;

    crypto::hash tx_prefix_hash = cryptonote::get_transaction_prefix_hash(tx);


    size_t in_i = tx.vin.size();

    vector<uint64_t> results;
    results.resize(tx.vin.size(), 0);

//    vector<string> outputs_pub_keys_str {
//        "399a1777ba884c6fd8dd8d79ed9ea2baf5cdad335124610f36eed940403821ed",
//        "de7f2e6719c8daebbb0963d815902b2ee50f51a64bc33f02c6195b03a3419863",
//        "6a3d9f707354f289901a49091910f2debbddfe3284878a53ad6bdb77cd4344ab",
//        "b181d711a77e230d91f3b40825de20e93de75e75d95fe1b8ef8dc8a6651062da"
//    };


    vector<string> outputs_pub_keys_str {
        "f80c28efb6ad285d36aad15341626a991e4974dc5c14d87f8bbb2e9f8687c70f",
        "e12e253a58a50cb457b7323212da24b0375d39cd20e8e46101359a4f27cd5a90",
        "cd1ff4678b81e13d70d00605fad591c79687a031ac9146574f07c34910f89454",
        "a3c34a78a0f002ea39d7f0ce8665a0e40534f2c9cd4fcbbb2ba98490b0466b3a"
    };

    cryptonote::account_keys sender_account_keys {address,
                                                  private_spend_key,
                                                  private_view_key};



    size_t no_of_mixins  = outputs_pub_keys_str.size();

    size_t output_index = idx_opt ? *idx_opt : 0;

    cout << "output_index: " << output_index << endl;

    crypto::hash tx_hash_prefix  = cryptonote::null_hash;


    tx_hash_prefix = crypto::rand<crypto::hash>();

    cout << tx_hash_prefix << endl;


    vector<crypto::public_key> pub_tx_keys;

    for(string& pk_s: outputs_pub_keys_str)
    {

        crypto::public_key pk;

        xmreg::parse_str_secret_key(pk_s, pk);

        pub_tx_keys.push_back(pk);
        cout << "pk: " << pk << endl;
    }


//    for (size_t i = 0; i < no_of_mixins; ++i)
//    {
//        //cout << "    - sig: " << print_sig(sigs[i]) << endl;
//        pub_tx_keys.push_back(crypto::rand<crypto::public_key>());
//        cout << "pk: " << pub_tx_keys.back() << endl;
//    }



    cryptonote::transaction new_tx;

    cryptonote::keypair txkey = cryptonote::keypair::generate();

    add_tx_pub_key_to_extra(new_tx, txkey.pub);

    crypto::hash new_tx_prefix_hash
            =  cryptonote::get_transaction_prefix_hash(new_tx);


    cout << "\nnew_tx_prefix_hash: "
         << new_tx_prefix_hash
         << endl;



    cryptonote::keypair in_ephemeral;
    crypto::key_image ki;

    cout << "\npub_tx_keys[output_index]: " <<  pub_tx_keys[output_index] << endl;



    crypto::key_derivation recv_derivation ;


//    bool r = crypto::generate_key_derivation(pub_tx_keys[output_index],
//                                             sender_account_keys.m_view_secret_key,
//                                             recv_derivation);


//    cout << "\n r: " << r << endl;
//    cout << "\nrecv_derivation: " << recv_derivation << endl;

    crypto::public_key reall_output_tx_key;

    // real one
    string reall_output_tx_key_str {"a48f34d9ecfa901bbe3858e8bba2d36a9e17475c5dee30b3b09c7665c69eb2d8"};
    size_t reall_output_tx_index {2};

    // fake one
    //string reall_output_tx_key_str {"b9c65864db37b952103f051dd9afa5588fb5505128d97085f9bed107aa1d8d22"};
    //size_t reall_output_tx_index {50};

    xmreg::parse_str_secret_key(reall_output_tx_key_str, reall_output_tx_key);



    if (!generate_key_image_helper(sender_account_keys,
                                   reall_output_tx_key,
                                   reall_output_tx_index,
                                   in_ephemeral,
                                   ki))
    {
        return false;
    }




    cout << "\nki: " << ki << endl;

    cout << "in_ephemeral.sec: " << in_ephemeral.sec << endl;


    std::vector<const crypto::public_key*> keys_ptrs;

    for (const crypto::public_key& pk: pub_tx_keys)
    {
        keys_ptrs.push_back(&pk);
    }


    crypto::signature* sigs = new crypto::signature[no_of_mixins];

    crypto::generate_ring_signature(new_tx_prefix_hash,
                                    ki,
                                    keys_ptrs,
                                    in_ephemeral.sec,
                                    output_index,
                                    sigs);

//
    cout << "\n - generate_ring_signature: " << endl;
    for (size_t i = 0; i < no_of_mixins; ++i)
    {
        cout << "    - sig: " << print_sig(sigs[i]) << endl;
    }

//
//
//
    bool result;

    result = crypto::check_ring_signature(
            new_tx_prefix_hash,
            ki,
            keys_ptrs,
            sigs);

    cout <<  "\n - result: " << result << "\n\n" << endl ;


    delete[] sigs;

    cout << "\nEnd of program." << endl;
//
    return 0;



    for (size_t i = 0; i < tx.vin.size(); ++i)
    {


        vector<for_signatures> for_sig_v;


        const cryptonote::txin_v &tx_in = tx.vin[i];

        // get tx input key
        const cryptonote::txin_to_key &tx_in_to_key
                = boost::get<cryptonote::txin_to_key>(tx_in);


        cout << "Key image: " << tx_in_to_key.k_image << endl;


        uint64_t pmax_used_block_height{0};

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


        // get public key of outputs used in mixins
        for (size_t outi = 0; outi < absolute_offsets.size(); ++outi) {

            cryptonote::output_data_t output_data = outputs.at(outi);

            outs_pub_keys.push_back(output_data.pubkey);
        }


        // for each mixin
        for (size_t outi = 0; outi < absolute_offsets.size(); ++outi)
        {

            cryptonote::output_data_t output_data = outputs.at(outi);


            cout << "  - mix out pubkey: " << output_data.pubkey << endl;
            //cout << "  - sig: " << tx.signatures[i][outi] << endl;

            vector<const crypto::public_key*> out_pub_key_array;

            out_pub_key_array.push_back(&output_data.pubkey);
            vector<crypto::signature> sig_array;
            sig_array.push_back(tx.signatures[i][outi]);
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


            for (const crypto::signature &sig: tx.signatures[i])
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


            // find tx_hash with given output
            crypto::hash tx_hash_found;
            cryptonote::transaction tx_found;


            if (!mcore.get_tx_hash_from_output_pubkey(
                    output_data.pubkey,
                    output_data.height,
                    tx_hash_found, tx_found))
            {
                print("- cant find tx_hash for ouput: {}, mixin no: {}, blk: {}\n",
                      output_data.pubkey,outi, output_data.height);

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
                      output_data.pubkey, outi, output_data.height);

                continue;
            }


            // get tx public key from extras field
            crypto::public_key pub_tx_key = cryptonote::get_tx_pub_key_from_extra(tx_found);

            cout << "Real tx " << ": ";

            cryptonote::keypair in_ephemeral;
            crypto::key_image ki;

            if (!generate_key_image_helper(sender_account_keys,
                                           pub_tx_key,
                                           output_index,
                                           in_ephemeral,
                                           ki))
            {
                   return false;
            }

            //        size_t real_output = absolute_offsets.size();
//




            cout << (output_data.pubkey == in_ephemeral.pub) << ": ";

            cout << "output_index: " << output_index << ": ";

            cout << "ki: " << ki << endl;

            for_signatures fs;

            //fs.tx_hash = cryptonote::get_transaction_hash(tx);
            fs.tx_hash = cryptonote::get_transaction_prefix_hash(tx);
            //fs.tx_hash = tx_hash_found;
            fs.kimg = ki;
            fs.outs_pub_keys = outs_pub_keys;
            fs.in_ephemeral = in_ephemeral;
            fs.real_output = absolute_offsets.size();

            for_sig_v.push_back(fs);


            //outs_pub_keys.push_back(output_data.pubkey);

        } //  for (size_t outi = 0; outi < absolute_offsets.size(); ++outi)




        cout << "\n\n\nfor_sig_v.size(): " << for_sig_v.size() << endl;


        std::vector<std::vector<crypto::signature> > signatures;



        for (const for_signatures& fs: for_sig_v)
        {

           // std::vector<crypto::signature> new_sigs(absolute_offsets.size());


//            signatures.push_back(std::vector<crypto::signature>());
//            std::vector<crypto::signature>& sigs = signatures.back();
//            sigs.resize(absolute_offsets.size());



            crypto::signature* sigs = new crypto::signature[4];

            cout <<"\n"
                 << "tx_hash_prefix: " << fs.tx_hash << "\n"
                 << "key_image: " << fs.kimg << "\n"
                 << "mixins no: " << fs.outs_pub_keys.size() << "\n"
                 << "in_ephemeral.sec: " << fs.in_ephemeral.sec <<  "\n"
                 << "fs.real_output: " << fs.real_output  << "\n" << endl;

            std::vector<const crypto::public_key*> keys_ptrs;

            for (const crypto::public_key& pk: fs.outs_pub_keys)
            {
                keys_ptrs.push_back(&pk);
                cout << " - " << pk << endl;

            }


            crypto::generate_ring_signature(fs.tx_hash,
                                            fs.kimg,
                                            keys_ptrs,
                                            fs.in_ephemeral.sec,
                                            fs.real_output - 1,
                                            sigs);

            cout << "\n - generate_ring_signature: " << endl;
            for (size_t i = 0; i < 4; ++i)
            {
                cout << "    - sig: " << print_sig(sigs[i]) << endl;
            }





            bool result;

            result = crypto::check_ring_signature(
                    fs.tx_hash,
                    fs.kimg,
                    keys_ptrs,
                    sigs);

            cout <<  "\n - result: " << result << "\n\n" << endl ;

            delete[] sigs;
        }



//        size_t real_output = absolute_offsets.size();
//
//        std::vector<crypto::signature> new_sigs(absolute_offsets.size());
//
//        cryptonote::keypair in_ephemeral;
//        crypto::key_image ki;
//
//        if (!generate_key_image_helper(sender_account_keys,
//                                       src_entr.real_out_tx_key,
//                                       src_entr.real_output_in_tx_index,
//                                       in_ephemeral,
//                                       ki))
//        {
//            return false;
//        }
//
//        crypto::generate_ring_signature(tx_prefix_hash,
//                                        tx_in_to_key.k_image,
//                                        outs_pub_keys.data(),
//                                        in_contexts[i].in_ephemeral.sec,
//                                        real_output,
//                                        new_sigs.data());


        //cout << "tx.signatures[i].size(): " << tx.signatures[i].size() << endl;

//        cout << "\ntx.signatures[i].size(): " << tx.signatures[i].size() << endl;
//
//        for (const crypto::signature& sig: tx.signatures[i])
//        {
//            cout << "  - real sig: " << print_sig(sig) << endl;
//        }
//
//
//        uint64_t result;

//        vector<crypto::signature> sigs = tx.signatures[i];
//
//
//        cout << endl;
//
//        for (auto& s: sigs)
//        {
//            cout << print_sig(s) << endl;
//        }
//
//
//        cout << "After random: " << endl;
//
//        random_shuffle (sigs.begin(), sigs.end());
//
//        for (auto& s: sigs)
//        {
//            cout << print_sig(s) << endl;
//        }
////
//        mcore.check_ring_signature(tx_prefix_hash,
//                         tx_in_to_key.k_image,
//                         outs_pub_keys,
//                         sigs,
//                         result);





       // cout << result << endl;

//
//        check_ring_signature(tx_prefix_hash,
//                             tx_in_to_key.k_image,
//                             pubkeys[sig_index],
//                             tx.signatures[i],
//                             results[sig_index]);


    }


//




    cout << "\nEnd of program." << endl;

    return 0;
}
