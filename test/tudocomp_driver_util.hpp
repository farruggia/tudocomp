
std::string shell_escape(const std::string& s) {
    //std::cout << "\n\nescape\n" << s << "\nto\n";
    std::stringstream ss;

    ss << "\"";

    for (auto c : s) {
        if (c == '"') {
            ss << "\\\"";
        } else if (c == '\\') {
            ss << "\\\\";
        } else {
            ss << c;
        }
    }

    ss << "\"";

    auto r = ss.str();

    //std::cout << r << "\n\n";

    return r;
}

std::string driver(std::string args) {
    using namespace std;

    FILE *in;
    char buff[512];
    std::stringstream ss;

    const std::string cmd_base = "./src/tudocomp_driver/tudocomp_driver " + args + " 2>&1";
    const std::string cmd = std::string("sh -c ") + shell_escape(cmd_base) + " 2>&1";

    if(!(in = popen(cmd.data(), "r"))) {
        throw std::runtime_error("Error executing " + cmd);
    }

    while(fgets(buff, sizeof(buff), in)!=NULL){
        ss << buff;
    }
    pclose(in);

    return ss.str();
}

std::string roundtrip_in_file_name(std::string algo,
                                   std::string name_addition) {
    return algo + name_addition + ".txt";
}
std::string roundtrip_comp_file_name(std::string algo,
                                   std::string name_addition) {
    return algo + name_addition + ".tdc";
}
std::string roundtrip_decomp_file_name(std::string algo,
                                   std::string name_addition) {
    return algo + name_addition + ".decomp.txt";
}

std::string format_std_outputs(const std::vector<std::string>& v) {
    std::stringstream ss;

    for (size_t i = 0; i < v.size(); i+=2) {
        if (v[i+1].empty()) {
            continue;
        }
        ss << v[i] << ": \n";
        ss << v[i+1] << "\n";
        ss << "\n";
    }

    std::string r = ss.str();

    if (r.size() > 0) {
        //r = r + "---\n";
    }

    return r;
}

std::string format_escape(const std::string& s) {
    std::stringstream ss;
    ss << "\"";
    for (auto c : s) {
        uint8_t b = c;
        if (b == '\\') {
            ss << "\\\\";
        } else if (b == 0) {
            ss << "\\0";
        } else if (b == '\"') {
            ss << "\\\"";
        } else if (b < 32 || b > 127) {
            ss << "\\x" << std::hex << int(b);
        } else {
            ss << c;
        }
    }
    ss << "\"";
    return ss.str();
}

std::string format_diff(const std::string& a, const std::string& b) {
    std::string diff;
    for(size_t i = 0; i < std::max(a.size(), b.size()); i++) {
        if (i < std::min(a.size(), b.size())
            && a[i] == b[i]
        ) {
            diff.push_back('-');
        } else {
            diff.push_back('#');
        }
    }
    return diff;
}

struct Error {
    bool has_error;
    std::string test;
    std::string message;
    std::string compress_cmd;
    std::string compress_stdout;
    std::string decompress_cmd;
    std::string decompress_stdout;
    std::string text;
    std::string roundtrip_text;
};

Error roundtrip(std::string algo,
               std::string name_addition,
               std::string text,
               bool use_raw,
               bool& abort)
{
    Error current { false };

    std::string in_file   = roundtrip_in_file_name(algo, name_addition);
    std::string comp_file = roundtrip_comp_file_name(algo, name_addition);
    std::string decomp_file  = roundtrip_decomp_file_name(algo, name_addition);

    //std::cout << "Roundtrip with\n";
    std::cout << in_file << " -> ";
    std::cout.flush();

    remove_test_file(in_file);
    remove_test_file(comp_file);
    remove_test_file(decomp_file);

    write_test_file(in_file, text);

    std::string comp_out;
    std::string decomp_out;

    // Compress
    {
        std::string in = test_file_path(in_file);
        std::string out = test_file_path(comp_file);
        std::string cmd;
        if (use_raw) {
            cmd = "--raw --algorithm " + shell_escape(algo)
                + " --output " + shell_escape(out) + " " + shell_escape(in);
        } else {
            cmd = "--algorithm " + shell_escape(algo)
                + " --output " + shell_escape(out) + " " + shell_escape(in);
        }
        current.compress_cmd = cmd;
        comp_out = driver(cmd);
    }

    std::cout << comp_file << " -> ";
    std::cout.flush();

    bool compressed_file_exists = test_file_exists(comp_file);

    if (!compressed_file_exists) {
        current.has_error = true;
        current.compress_stdout = comp_out;
        current.message = "compression did not produce output";
        current.test = in_file + " -> " + comp_file;
        std::cout << "ERR\n";
        return current;
    }

    // Decompress
    {
        std::string in = test_file_path(comp_file);
        std::string out = test_file_path(decomp_file);
        std::string cmd;
        if (use_raw) {
            cmd = "--raw --decompress --algorithm " + shell_escape(algo)
                + " --output " + shell_escape(out) + " " + shell_escape(in);
        } else {
            cmd = "--decompress --output " + shell_escape(out) + " " + shell_escape(in);
        }
        current.decompress_cmd = cmd;
        decomp_out = driver(cmd);
    }

    std::cout << decomp_file << " ... ";
    std::cout.flush();

    bool decompressed_file_exists = test_file_exists(decomp_file);
    if (!decompressed_file_exists) {
        current.has_error = true;
        current.compress_stdout = comp_out;
        current.decompress_stdout = decomp_out;
        current.message = "decompression did not produce output";
        current.test = comp_file + " -> " + decomp_file;
        std::cout << "ERR\n";
        return current;
    } else {
        std::string read_text = read_test_file(decomp_file);
        if (read_text != text) {
            current.has_error = true;
            current.compress_stdout = comp_out;
            current.decompress_stdout = decomp_out;
            current.test = in_file + " -> " + comp_file + " -> " + decomp_file;
            current.message = "compression - decompression roundtrip did not preserve the same string";
            current.text = text;
            current.roundtrip_text = read_text;

            //abort = true;
            std::cout << "ERR\n";
            return current;
        } else {
            std::cout << "OK\n";
        }
    }

    return current;
}
