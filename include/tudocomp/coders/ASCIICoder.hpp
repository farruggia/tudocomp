#pragma once

#include <sstream>
#include <tudocomp/Coder.hpp>

namespace tdc {

/// \brief Defines data encoding to and decoding from a stream of ASCII
///        characters.
///
/// Literals are encoded by their character representation, bits are
/// encoded using either the character \e 0 or \e 1 and integers are
/// encoded as their string representations, terminated by the \e :
/// character.
class ASCIICoder : public Algorithm {
public:
    /// \brief Yields the coder's meta information.
    /// \sa Meta
    inline static Meta meta() {
        Meta m("coder", "ascii", "Simple ASCII encoding");
        return m;
    }

    /// \cond DELETED
    ASCIICoder() = delete;
    /// \endcond

    /// \brief Encodes data to an ASCII character stream.
    class Encoder : public tdc::Encoder {
    public:
        ENCODER_CTOR(env, out, literals) {}

        template<typename value_t>
        inline void encode(value_t v, const Range& r) {
            std::ostringstream s;
            s << v;
            for(uint8_t c : s.str()) m_out->write_int(c);
            m_out->write_int(':');
        }

        template<typename value_t>
        inline void encode(value_t v, const LiteralRange& r) {
            m_out->write_int(uint8_t(v));
        }

        template<typename value_t>
        inline void encode(value_t v, const BitRange& r) {
            m_out->write_int(v ? '1' : '0');
        }
    };

    /// \brief Decodes data from an ASCII character stream.
    class Decoder : public tdc::Decoder {
    public:
        DECODER_CTOR(env, in) {}

        template<typename value_t>
        inline value_t decode(const Range& r) {
            std::ostringstream os;
            for(uint8_t c = m_in->read_int<uint8_t>();
                c >= '0' && c <= '9';
                c = m_in->read_int<uint8_t>()) {

                os << c;
            }

            std::string s = os.str();

            value_t v;
            std::istringstream is(s);
            is >> v;
            return v;
        }

        template<typename value_t>
        inline value_t decode(const LiteralRange& r) {
            return value_t(m_in->read_int<uint8_t>());
        }

        template<typename value_t>
        inline value_t decode(const BitRange& r) {
            uint8_t b = m_in->read_int<uint8_t>();
            return (b != '0');
        }
    };
};

}

