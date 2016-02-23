#ifndef _INCLUDED_DUMMYCODER_HPP_
#define _INCLUDED_DUMMYCODER_HPP_

namespace tudocomp {

/**
 * Encodes factors as simple strings.
 */
template<typename F>
class DummyCoder {
    
    const char init_sep   = ':';
    const char fact_start = '(';
    const char fact_sep   = ',';
    const char fact_end   = ')';

public:
    DummyCoder() = delete;
    
    /// Constructor for the given output, accepting an environment.
    DummyCoder(Env& env) {
        //
    }
    
    /// Destructor
    ~DummyCoder() {
        //
    }
    
    /// Initiates the encoding with no total length information.
    inline void encode_init(Output& out) {
        encode_init(out, 0); //TODO throw exception!
    }

    /// Initiates the encoding with total length information.
    inline void encode_init(Output& out, size_t len) {
        *(out.as_stream()) << len << init_sep;
    }
    
    /// Encodes the given symbol.
    inline void encode(Output& out, size_t pos, char32_t sym) {
        *(out.as_stream()) << char(sym);
    }
    
    /// Encodes the given factor.
    void encode(Output& out, size_t pos, const F& fact);
    
    /// Finalizes the encoding.
    inline void encode_finalize(Output& out) {
        //
    }
    
    //TODO decode
};

}

#endif
